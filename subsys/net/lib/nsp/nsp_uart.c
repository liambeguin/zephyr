#include <stdio.h>
#include <stdlib.h>
#include <zephyr/logging/log.h>
#include <zephyr/net_buf.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/nsp.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/crc.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(nsp_uart, CONFIG_NSP_LOG_LEVEL);

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);
ZBUS_MSG_SUBSCRIBER_DEFINE(nsp_uart_msg_sub);
ZBUS_CHAN_ADD_OBS(nsp_out_chan, nsp_uart_msg_sub, 2);


#define SLIP_END     0300
#define SLIP_ESC     0333
#define SLIP_ESC_END 0334
#define SLIP_ESC_ESC 0335

enum slip_state {
	STATE_GARBAGE,
	STATE_OK,
	STATE_ESC,
};

/* UART device */
static const struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(nsp_uart));

/* SLIP state machine */
static uint8_t slip_state = STATE_OK;

static struct net_buf *uart_buf;


static int slip_process_byte(unsigned char c)
{
#ifdef VERBOSE_DEBUG
	LOG_DBG("recv: state %u byte %x", slip_state, c);
#endif
	switch (slip_state) {
	case STATE_GARBAGE:
		if (c == SLIP_END) {
			slip_state = STATE_OK;
		}
		LOG_DBG("garbage: discard byte %x", c);
		return 0;

	case STATE_ESC:
		if (c == SLIP_ESC_END) {
			c = SLIP_END;
		} else if (c == SLIP_ESC_ESC) {
			c = SLIP_ESC;
		} else {
			slip_state = STATE_GARBAGE;
			return 0;
		}
		slip_state = STATE_OK;
		break;

	case STATE_OK:
		if (c == SLIP_ESC) {
			slip_state = STATE_ESC;
			return 0;
		} else if (c == SLIP_END) {
			return 1;
		}
		break;
	}

#ifdef VERBOSE_DEBUG
	LOG_DBG("processed: state %u byte %x", slip_state, c);
#endif

	if (!uart_buf) {
		uart_buf = net_buf_alloc(&nsp_pkt_pool, K_FOREVER);
		if (!uart_buf) {
			LOG_ERR("No more buffers");
			return 0;
		}
	}

	net_buf_add_u8(uart_buf, c);

	return 0;
}

static void interrupt_handler(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

	struct nsp_pkt rxpkt = {0};
	uint16_t exp_crc;
	uint16_t crc;
	int ret;

	while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {
		unsigned char byte;

		if (!uart_irq_rx_ready(dev)) {
			continue;
		}

		while (uart_fifo_read(dev, &byte, sizeof(byte))) {
			if (slip_process_byte(byte)) {
				/**
				 * slip_process_byte() returns 1 on
				 * SLIP_END, even after receiving full
				 * packet
				 */
				if (!uart_buf) {
					LOG_DBG("Skip SLIP_END");
					continue;
				}

				LOG_HEXDUMP_DBG(uart_buf->data, uart_buf->len, "SLIP <");

				exp_crc = net_buf_remove_le16(uart_buf);
				crc = crc16_ccitt(0xffff, uart_buf->data, uart_buf->len);
				if (crc != exp_crc) {
					LOG_ERR("invalid CRC: got 0x%04x expected 0x%04x", crc, exp_crc);
					net_buf_unref(uart_buf);
					uart_buf = NULL;
					continue;
				}

				rxpkt.buf = net_buf_alloc(&nsp_pkt_pool, K_FOREVER);

				rxpkt.dst = net_buf_pull_u8(uart_buf);
				rxpkt.src = net_buf_pull_u8(uart_buf);
				rxpkt.cmd = net_buf_pull_u8(uart_buf);
				net_buf_add_mem(rxpkt.buf, uart_buf->data, uart_buf->len);

#if CONFIG_NSP_PRINT_ROUTING
				LOG_INF("Received packet");
#endif
				ret = zbus_chan_pub(&nsp_in_chan, &rxpkt, K_MSEC(200));
				if (ret) {
					LOG_ERR("*** Failed to publish: %s (%d)", strerror(-ret), ret);
					/* TODO: do better here */
				}

				net_buf_unref(uart_buf);
				uart_buf = NULL;
			}
		}
	}
}

int nsp_uart_rx_start(void)
{
	uart_irq_callback_set(uart_dev, interrupt_handler);
	uart_irq_rx_enable(uart_dev);

	return 0;
}

static size_t slip_buffer(uint8_t *sbuf, struct net_buf *buf)
{
	size_t len = buf->len;
	uint8_t *sbuf_orig = sbuf;
	int i;

	*sbuf++ = SLIP_END;

	for (i = 0; i < len; i++) {
		uint8_t byte = net_buf_pull_u8(buf);

		switch (byte) {
		case SLIP_END:
			*sbuf++ = SLIP_ESC;
			*sbuf++ = SLIP_ESC_END;
			break;
		case SLIP_ESC:
			*sbuf++ = SLIP_ESC;
			*sbuf++ = SLIP_ESC_ESC;
			break;
		default:
			*sbuf++ = byte;
		}
	}

	*sbuf++ = SLIP_END;

	return sbuf - sbuf_orig;
}

static int try_write(uint8_t *data, uint16_t len)
{
	int wrote;

	while (len) {
		wrote = uart_fifo_fill(uart_dev, data, len);
		if (wrote <= 0) {
			return wrote;
		}

		len -= wrote;
		data += wrote;
	}

	return 0;
}

static void nsp_uart_send_task(void *ptr1, void *ptr2, void *ptr3)
{
        ARG_UNUSED(ptr1);
        ARG_UNUSED(ptr2);
        ARG_UNUSED(ptr3);

	static uint8_t slip_buf[1 + 2 * NSP_BUFSIZE] = {0};
	const struct zbus_channel *chan;
	struct nsp_pkt txpkt = {0};
	uint16_t crc;
	int len = 0;
	int ret;

	nsp_uart_rx_start();

        while (!zbus_sub_wait_msg(&nsp_uart_msg_sub, &chan, &txpkt, K_FOREVER)) {
                if (chan != &nsp_out_chan)
			continue;

		if ((txpkt.dst & CONFIG_NSP_BACKEND_UART_TX_MASK) != txpkt.dst)
			continue;

#if CONFIG_NSP_PRINT_ROUTING
		LOG_INF("Sending packet");
#endif

		// header
		net_buf_push_u8(txpkt.buf, txpkt.cmd);
		net_buf_push_u8(txpkt.buf, txpkt.src);
		net_buf_push_u8(txpkt.buf, txpkt.dst);
		// tail
		crc = crc16_ccitt(0xffff, txpkt.buf->data, txpkt.buf->len);
		net_buf_add_le16(txpkt.buf, crc);

		LOG_HEXDUMP_DBG(txpkt.buf->data, txpkt.buf->len, "SLIP >");

		/* SLIP encode and send */
		len = slip_buffer(slip_buf, txpkt.buf);
		ret = try_write(slip_buf, len);

		net_buf_unref(txpkt.buf);
        }
}

K_THREAD_DEFINE(nsp_uart_send_task_id, 400, nsp_uart_send_task, NULL, NULL, NULL, 2, 0, 0);
