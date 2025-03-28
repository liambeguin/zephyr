#include <stdio.h>
#include <stdlib.h>
#include <zephyr/logging/log.h>
#include <zephyr/net_buf.h>
#include <zephyr/drivers/can.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socketcan.h>
#include <zephyr/net/socketcan_utils.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_service.h>
#include <zephyr/sys/crc.h>

#include <zephyr/net/nsp.h>

#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(nsp_can, CONFIG_NSP_LOG_LEVEL);

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);
ZBUS_MSG_SUBSCRIBER_DEFINE(nsp_can_msg_sub);
ZBUS_CHAN_ADD_OBS(nsp_out_chan, nsp_can_msg_sub, 2);

struct nsp_can_frame_id {
	union {
		uint32_t can_id;
		struct {
			uint8_t last: 1;
			uint8_t msgn: 7;
			uint8_t src: 8;
			uint8_t dst: 8;
			uint8_t priority: 3;
			uint8_t spare: 2;
			uint8_t flags: 3;
		};
	};
} __attribute__((packed));

static const struct device *can_dev = DEVICE_DT_GET(DT_CHOSEN(nsp_can));

static void nsp_can_rx_callback(const struct device *dev, struct can_frame *frame, void *user_data)
{
	ARG_UNUSED(user_data);

	static struct nsp_pkt rxpkt = {0};
	struct nsp_can_frame_id cid;
	static int inframe = 0;
	uint16_t exp_crc, crc;
	int ret;

	cid.can_id = frame->id;

	if (!inframe) {
		rxpkt.buf = net_buf_alloc(&nsp_pkt_pool, K_FOREVER);
		inframe = 1;
	}

	net_buf_add_mem(rxpkt.buf, frame->data, frame->dlc);

	if (!cid.last) {
		return;
	}

	// add header for crc, check, and pop
	net_buf_push_u8(rxpkt.buf, cid.src);
	net_buf_push_u8(rxpkt.buf, cid.dst);

	exp_crc = net_buf_remove_le16(rxpkt.buf);
	crc = crc16_ccitt(0xffff, rxpkt.buf->data, rxpkt.buf->len);
	if (crc != exp_crc) {
		LOG_ERR("invalid CRC: got 0x%04x expected 0x%04x", crc, exp_crc);
		// TODO: send NACK
		net_buf_unref(rxpkt.buf);
		rxpkt.buf = NULL;
		inframe = 0;
		return;
	}

	rxpkt.dst = net_buf_pull_u8(rxpkt.buf);
	rxpkt.src = net_buf_pull_u8(rxpkt.buf);
	rxpkt.cmd = net_buf_pull_u8(rxpkt.buf);

#if CONFIG_NSP_PRINT_ROUTING
	LOG_INF("Received packet");
#endif
	ret = zbus_chan_pub(&nsp_in_chan, &rxpkt, K_MSEC(200));
	if (ret) {
		LOG_ERR("*** Failed to publish: %s (%d)", strerror(-ret), ret);
		/* TODO: do better here */
	}

	inframe = 0;
}

static void nsp_can_task(void *ptr1, void *ptr2, void *ptr3)
{
	ARG_UNUSED(ptr1);
	ARG_UNUSED(ptr2);
	ARG_UNUSED(ptr3);

	const struct zbus_channel *chan;
	struct nsp_can_frame_id cid = {
		.flags = 0,
		.spare = 0,
		.priority = 7,
	};
	struct can_frame frame = {0};
	struct nsp_pkt txpkt = {0};
	struct can_filter filter;
	struct net_buf *can_buf;
	uint32_t id_mask;
	int len;
	int crc;
	int ret;

	id_mask = CAN_EXT_ID_MASK;
	filter.flags = CAN_FILTER_IDE;
	filter.id = 0x07000000;
	filter.mask = 0x1F000000;

	ret = can_set_bitrate(can_dev, 1000000);
	if (ret) {
		LOG_ERR("*** Failed to set CAN bitrate %d", ret);
		return;
	}

	ret = can_start(can_dev);
	if (ret) {
		LOG_ERR("*** Failed to start CAN interface %d", ret);
		return;
	}

	ret = can_add_rx_filter(can_dev, nsp_can_rx_callback, NULL, &filter);
	if (ret) {
		LOG_ERR("*** Failed to add CAN RX filter %d", ret);
		return;
	}

	LOG_INF("registered RX callback");

	while (!zbus_sub_wait_msg(&nsp_can_msg_sub, &chan, &txpkt, K_FOREVER)) {
		if (chan != &nsp_out_chan)
			continue;

		if ((txpkt.dst & CONFIG_NSP_BACKEND_CAN_TX_MASK) != txpkt.dst)
			continue;

#if CONFIG_NSP_PRINT_ROUTING
		LOG_INF("Sending packet");
#endif

		can_buf = net_buf_alloc(&nsp_pkt_pool, K_FOREVER);
		memset(&frame, 0, sizeof(frame));

		// add header for CRC then remove
		net_buf_push_u8(can_buf, txpkt.cmd);
		net_buf_push_u8(can_buf, txpkt.src);
		net_buf_push_u8(can_buf, txpkt.dst);
		net_buf_add_mem(can_buf, txpkt.buf->data, txpkt.buf->len);
		crc = crc16_ccitt(0xffff, can_buf->data, can_buf->len);
		net_buf_add_le16(can_buf, crc);
		net_buf_pull_u8(can_buf);
		net_buf_pull_u8(can_buf);

		// init frame_id
		cid.dst = txpkt.dst;
		cid.src = txpkt.src;
		cid.msgn = 0;

		net_buf_unref(txpkt.buf);

		while (can_buf->len > 0) {
			len = can_buf->len > 8 ? 8 : can_buf->len;
			cid.last = can_buf->len - len == 0 ? 1 : 0;

			frame.id = cid.can_id;
			frame.dlc = can_bytes_to_dlc(len);
			frame.flags |= CAN_FRAME_IDE;
			memcpy(frame.data, net_buf_pull_mem(can_buf, len), len);

			ret = can_send(can_dev, &frame, K_NO_WAIT, NULL, NULL);
			if (ret) {
				LOG_ERR("*** Failed to queue CAN frame #%u (err %d)", 0, ret);
				// TODO
			}

			cid.msgn++;
		}

		net_buf_unref(can_buf);
	}
}
K_THREAD_DEFINE(nsp_can_task_id, 800, nsp_can_task, NULL, NULL, NULL, 2, 0, 0);
