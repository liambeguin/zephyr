/*
 * Copyright (c) 2024 HawkEye 360
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT adi_adf702x
#define LOG_MODULE_NAME adf702x
/* #define LOG_LEVEL CONFIG_IEEE802154_DRIVER_LOG_LEVEL */
#define LOG_LEVEL LOG_LEVEL_DBG

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_instance.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/debug/stack.h>

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_pkt.h>

#include <zephyr/sys/byteorder.h>
#include <string.h>
#include <zephyr/random/random.h>
#include <zephyr/linker/sections.h>
#include <zephyr/sys/atomic.h>

#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/net/ieee802154_radio.h>

#include "ieee802154_adf702x_regs.h"
#include "ieee802154_adf702x.h"
#include "ieee802154_adf702x_profiles.h"

#define MAX_POLL_LOOPS 200


static void adf702x_print_status(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;

	LOG_INST_DBG(conf->log, "status word: 0x%02X -> SPI_READY:%d IRQ_STATUS:%d CMD_READY:%d FW_STATE:0x%02x",
		ctx->status,
		(uint8_t)FIELD_GET(STATUS_SPI_READY, ctx->status),
		(uint8_t)FIELD_GET(STATUS_IRQ_STATUS, ctx->status),
		(uint8_t)FIELD_GET(STATUS_CMD_READY, ctx->status),
		(uint8_t)FIELD_GET(STATUS_FW_STATE, ctx->status));

	switch((uint8_t)FIELD_GET(STATUS_FW_STATE, ctx->status)) {
	case FW_STATE_INIT: LOG_INST_WRN(conf->log, "FW_STATE_INIT"); break;
	case FW_STATE_BUSY: LOG_INST_WRN(conf->log, "FW_STATE_BUSY"); break;
	case FW_STATE_PHY_OFF: LOG_INST_WRN(conf->log, "FW_STATE_PHY_OFF"); break;
	case FW_STATE_PHY_ON: LOG_INST_WRN(conf->log, "FW_STATE_PHY_ON"); break;
	case FW_STATE_PHY_RX: LOG_INST_WRN(conf->log, "FW_STATE_PHY_RX"); break;
	case FW_STATE_PHY_TX: LOG_INST_WRN(conf->log, "FW_STATE_PHY_TX"); break;
	case FW_STATE_PHY_SLEEP: LOG_INST_WRN(conf->log, "FW_STATE_PHY_SLEEP"); break;
	case FW_STATE_GET_RSSI: LOG_INST_WRN(conf->log, "FW_STATE_GET_RSSI"); break;
	case FW_STATE_IR_CAL: LOG_INST_WRN(conf->log, "FW_STATE_IR_CAL"); break;
	case FW_STATE_AES_DECRYPT_INIT: LOG_INST_WRN(conf->log, "FW_STATE_AES_DECRYPT_INIT"); break;
	case FW_STATE_AES_DECRYPT: LOG_INST_WRN(conf->log, "FW_STATE_AES_DECRYPT"); break;
	case FW_STATE_AES_ENCRYPT: LOG_INST_WRN(conf->log, "FW_STATE_AES_ENCRYPT"); break;
	default: LOG_INST_WRN(conf->log, "UNKNOWN FW_STATE");  break;
	}
}

static int adf702x_get_status(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;

	uint8_t tx_buffer[2] = {SPI_NOP, SPI_NOP};
	uint8_t rx_buffer[2] = {0};
	const struct spi_buf tx_buffers = {.buf = tx_buffer, .len = ARRAY_SIZE(tx_buffer)};
	const struct spi_buf rx_buffers = {.buf = rx_buffer, .len = ARRAY_SIZE(rx_buffer)};
	const struct spi_buf_set tx_set = {.buffers = &tx_buffers, .count = 1};
	const struct spi_buf_set rx_set = {.buffers = &rx_buffers, .count = 1};
	int ret;

	ret = spi_transceive_dt(&conf->spi, &tx_set, &rx_set);
	if (ret) {
		LOG_INST_ERR(conf->log, "%s: spi_transceive FAIL %d\n", __func__, ret);
		return ret;
	}

	ctx->status = rx_buffer[1];

	return 0;
}

static int adf702x_set_command(const struct device *dev, uint8_t command)
{
	const struct adf702x_config *conf = dev->config;
	const struct spi_buf tx_buffers = {.buf = &command, .len = 1};
	const struct spi_buf_set tx_set = {.buffers = &tx_buffers, .count = 1};
	int ret;

	ret = spi_transceive_dt(&conf->spi, &tx_set, NULL);
	if (ret)
		LOG_INST_ERR(conf->log, "%s: spi_transceive FAIL %d\n", __func__, ret);

	return ret;
}


static int adf702x_set_fw_state(const struct device *dev, uint8_t fw_state)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	int ret = 0;
	int cnt = 0;

	switch(fw_state) {
	case FW_STATE_PHY_OFF:
		adf702x_set_command(dev, CMD_PHY_OFF);
		break;
	case FW_STATE_PHY_ON:
		adf702x_set_command(dev, CMD_PHY_ON);
		break;
	case FW_STATE_PHY_SLEEP:
		adf702x_set_command(dev, CMD_PHY_SLEEP);
		break;
	case FW_STATE_PHY_RX:
		adf702x_set_command(dev, CMD_PHY_RX);
		break;
	case FW_STATE_PHY_TX:
		adf702x_set_command(dev, CMD_PHY_TX);
		break;
	default:
		LOG_INST_ERR(conf->log, "Unknown state: 0x%x", fw_state);
		return -EINVAL;
	}

	do {
		ret = adf702x_get_status(dev);
		cnt++;
	} while((FIELD_GET(STATUS_FW_STATE, ctx->status) != fw_state) && (cnt < MAX_POLL_LOOPS));

	if (cnt == MAX_POLL_LOOPS)
		LOG_INST_WRN(conf->log, "set_fw_status: TIMEOUT");

	return ret;
}

static int adf702x_ram_read(const struct device *dev, int addr, int len, uint8_t *data)
{
	const struct adf702x_config *conf = dev->config;
	uint8_t tx_buffer[] = {
		SPI_MEM_RD | FIELD_GET(0x700, addr),
		FIELD_GET(0x0ff, addr),
		SPI_NOP,
	};

	const struct spi_buf tx_buffers[] = {
		{.buf = tx_buffer, .len = ARRAY_SIZE(tx_buffer)},
		{.buf = NULL,       .len = len}, // NOTE: buf should be filled
						 // with SPI_NOP here but 0x00
						 // works too and makes for
						 // less code
	};
	const struct spi_buf rx_buffers[] = {
		{.buf = NULL, .len = 3},
		{.buf = data, .len = len},
	};
	const struct spi_buf_set tx_set = {.buffers = tx_buffers, .count = 2};
	const struct spi_buf_set rx_set = {.buffers = rx_buffers, .count = 2};
	int ret;

	ret = spi_transceive_dt(&conf->spi, &tx_set, &rx_set);
	if (ret)
		LOG_INST_ERR(conf->log, "%s: spi_transceive FAIL %d\n", __func__, ret);

	return ret;
}

static int adf702x_ram_dump_conf(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	uint8_t conf_regs[64] = {0};

	adf702x_ram_read(dev, 0x100, 64, conf_regs);
	LOG_INST_HEXDUMP_ERR(conf->log, conf_regs, 64, "LVB:");

	return 0;
}

static int adf702x_packet_read(const struct device *dev, const char *prefix)
{
	const struct adf702x_config *conf = dev->config;
	uint8_t header[1] = {0};
	uint8_t pram[256] = {0};
	uint8_t len = 0;

	adf702x_ram_read(dev, ADF702X_RX_BASE_ADR, 1, header);
	len = header[0] - 1;

	adf702x_ram_read(dev, ADF702X_RX_BASE_ADR + 1, len, pram);
	LOG_INST_DBG(conf->log, "%s Frame: length: %02X", prefix, len);
	LOG_INST_HEXDUMP_DBG(conf->log, pram, len, "payload:");

	return 0;
}
static int adf702x_ram_write(const struct device *dev, int addr, int len, uint8_t *data)
{
	const struct adf702x_config *conf = dev->config;
	uint8_t tx_buffer[] = {
		SPI_MEM_WR | FIELD_GET(0x700, addr),
		FIELD_GET(0x0ff, addr),
	};
	struct spi_buf tx_buffers[2] = {
		{ .buf = tx_buffer, .len = ARRAY_SIZE(tx_buffer) },
		{ .buf = data,      .len = len                   },
	};
	const struct spi_buf_set tx_set = {.buffers = tx_buffers, .count = 2};
	int ret;

	ret = spi_transceive_dt(&conf->spi, &tx_set, NULL);
	if (ret)
		LOG_INST_ERR(conf->log, "%s: spi_transceive FAIL %d\n", __func__, ret);

	return ret;
}

static int adf702x_packet_write(const struct device *dev, uint8_t *data, uint8_t len)
{
	const struct adf702x_config *conf = dev->config;
	uint8_t header[1] = {0};
	int ret;

	if (len > 256) {
		LOG_INST_ERR(conf->log, "Payload too large: %d", len);
		return -EMSGSIZE;
	}

	// on adf7023, header can also include address_match_offset
	header[0] = len + 1;

	ret = adf702x_ram_write(dev, ADF702X_TX_BASE_ADR, 1, header);
	if (ret)
		return ret;

	ret = adf702x_ram_write(dev, ADF702X_TX_BASE_ADR + 1, len, data);
	if (ret)
		return ret;

	adf702x_packet_read(dev, "TX");

	return 0;
}

static int adf702x_regs_set_channel_freq(const struct device *dev, uint32_t freq)
{
	struct adf702x_context *ctx = dev->data;
	uint32_t ch_freq = (uint32_t)(((float)freq / 26000000) * 65535);

	ctx->conf_regs.channel_freq0 = FIELD_GET(0x0000FF, ch_freq);
	ctx->conf_regs.channel_freq1 = FIELD_GET(0x00FF00, ch_freq);
	ctx->conf_regs.channel_freq2 = FIELD_GET(0xFF0000, ch_freq);

	return 0;
}

static int adf702x_regs_set_pa_level(const struct device *dev, float dBm)
{
	struct adf702x_context *ctx = dev->data;

	// linear equation from 2 points 3 -> -20dBm and 63 -> 13dBm.
	ctx->conf_regs.radio_pa_level = (uint8_t)((float)1.71 * dBm + (float)38.82);

	return 0;
}

static void adf702x_iface_init(struct net_if *iface)
{
	const struct device *dev = net_if_get_device(iface);
	struct adf702x_context *ctx = dev->data;
	const struct adf702x_config *conf = dev->config;

	LOG_INST_DBG(conf->log, "iface init");

	ctx->iface = iface;

	ieee802154_init(iface);
}

static enum ieee802154_hw_caps adf702x_get_capabilities(const struct device *dev)
{
	ARG_UNUSED(dev);
	return IEEE802154_HW_FCS;
}

#if 0
static int adf702x_cca(const struct device *dev)
{
	struct adf702x_context *ctx = dev->data;
	uint8_t rssi = 0;
	ARG_UNUSED(dev);
	int ret = 0;

	// TODO
	LOG_INST_DBG(conf->log, "CCA TODO");
	return 0;

	// must be in PHY_ON
	ret = adf702x_get_status(dev);
	if (ret)
		return ret;

	if (FIELD_GET(STATUS_FW_STATE, ctx->status) != FW_STATE_PHY_ON)
		return -EIO;

	ret = adf702x_set_fw_state(dev, FW_STATE_GET_RSSI);
	if (ret)
		return ret;

	ret = adf702x_ram_read(dev, ADF702X_REG_RSSI_READBACK, 1, &rssi);
	if (ret)
		return ret;

	// TODO now do something with it??

	return ret;
}
#endif

static int adf702x_set_channel(const struct device *dev, uint16_t channel)
{
	const struct adf702x_config *conf = dev->config;

	LOG_INST_ERR(conf->log, "%s: channel=%d", __func__, channel);

	return -EALREADY;
}

static int adf702x_set_txpower(const struct device *dev, int16_t dBm)
{
	const struct adf702x_config *conf = dev->config;

	LOG_INST_ERR(conf->log, "%s: dBm=%d", __func__, dBm);

	return adf702x_regs_set_pa_level(dev, dBm);
}

static int adf702x_tx(const struct device *dev,
		      enum ieee802154_tx_mode mode,
		      struct net_pkt *pkt,
		      struct net_buf *frag)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	int ret;

	if (!ctx->is_up)
		return -ENETDOWN;

	// For now we only support direct mode
	if (mode != IEEE802154_TX_MODE_DIRECT)
		return -EINVAL;

	if (FIELD_GET(ADF702X_BIT_PKT_LENGTH_CONTROL_PKT_LEN_MODE,
		      ctx->conf_regs.packet_length_control)) {
		LOG_INST_ERR(conf->log, "Fixed packet lenght not supported");
		return -EINVAL;
	}

	k_sem_reset(&ctx->tx_wait);
	ret = adf702x_packet_write(dev, frag->data, frag->len);
	if (ret) {
		LOG_INST_ERR(conf->log, "Failed to write to packet RAM");
		return ret;
	}

	ret = adf702x_set_fw_state(dev, FW_STATE_PHY_TX);
	if (ret)
		return ret;

	/* Now we wait for the callback from isr */
	k_sem_take(&ctx->tx_wait, K_FOREVER);

	return 0;
}

static int adf702x_start(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	uint8_t auxram[] = {0x00};
	int ret;

	LOG_INST_DBG(conf->log, "start");
	if (ctx->is_up)
		return -EALREADY;

	// disable test-pattern mode
	adf702x_ram_write(dev, ADF702X_REG_AUX_VAR_TX_MODE, 1, auxram);

	ret = adf702x_set_fw_state(dev, FW_STATE_PHY_ON);
	if (ret)
		return ret;

	ctx->is_up = true;
	return 0;
}

static int adf702x_stop(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	int ret;

	LOG_INST_DBG(conf->log, "stop");
	if (!ctx->is_up)
		return -EALREADY;

	ret = adf702x_set_fw_state(dev, FW_STATE_PHY_OFF);
	if (ret)
		return ret;

	ctx->is_up = false;
	return 0;
}

int adf702x_configure(const struct device *dev,
		      enum ieee802154_config_type type,
		      const struct ieee802154_config *config)
{
	const struct adf702x_config *conf = dev->config;
	int ret = -EINVAL;

	LOG_INST_DBG(conf->log, "Configure %d", type);

	switch (type) {
	case IEEE802154_CONFIG_AUTO_ACK_FPB:
	case IEEE802154_CONFIG_ACK_FPB:
	case IEEE802154_CONFIG_PAN_COORDINATOR:
	case IEEE802154_CONFIG_PROMISCUOUS:
	case IEEE802154_CONFIG_EVENT_HANDLER:
	default:
		break;
	}

	return ret;
}
// dummy always return suppored_ch pages
static int adf702x_attr_get(const struct device *dev, enum ieee802154_attr attr,
			    struct ieee802154_attr_value *value)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	uint8_t bram[64] = {0};

#if CONFIG_IEEE802154_RAW_MODE
	// in RAW_MODE we kinda hijack this feature
	switch (attr) {
	case 0:
		adf702x_get_status(dev);
		adf702x_print_status(dev);
		break;
	case 1:
		adf702x_ram_read(dev, value->phy_supported_channel_pages, 1, bram);
		value->phy_supported_channel_pages = bram[0];
		LOG_INST_WRN(conf->log, "value: %02X", bram[0]);
		break;
	case 2:
		adf702x_ram_dump_conf(dev);
		break;
	case 3:
		adf702x_set_fw_state(dev, FW_STATE_PHY_RX);
		break;
	default:
		return -ENOENT;
	}

	return 0;
#else
	LOG_INST_DBG(conf->log, "attr_get %d", attr);

	// Dummy stuff
	switch (attr) {
	case IEEE802154_ATTR_PHY_SUPPORTED_CHANNEL_PAGES:
		value->phy_supported_channel_pages = ctx->cc_page;
		return 0;

	case IEEE802154_ATTR_PHY_SUPPORTED_CHANNEL_RANGES:
		value->phy_supported_channels = &ctx->cc_channels;
		return 0;
	default:
		return -ENOENT;
	}
#endif
}

static int adf702x_cw(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	uint8_t auxram[] = {0x03};

	LOG_INST_DBG(conf->log, "CW");

	adf702x_get_status(dev);
	adf702x_set_fw_state(dev, FW_STATE_PHY_ON);

	adf702x_ram_write(dev, ADF702X_REG_AUX_VAR_TX_MODE, 1, auxram);
	adf702x_set_fw_state(dev, FW_STATE_PHY_TX);

	return 0;
}

static const struct ieee802154_radio_api adf702x_radio_api = {
	.iface_api.init		= adf702x_iface_init,
	.get_capabilities	= adf702x_get_capabilities,
	/* .cca			= adf702x_cca, */
	.set_channel		= adf702x_set_channel,
	.set_txpower		= adf702x_set_txpower,
	.tx			= adf702x_tx,
	.start			= adf702x_start,
	.stop			= adf702x_stop,
	.configure		= adf702x_configure,
	.attr_get		= adf702x_attr_get,
	.continuous_carrier	= adf702x_cw,
};

static inline void adf702x_irq_handler(const struct device *port,
				       struct gpio_callback *cb,
				       uint32_t pins)
{
	struct adf702x_context *ctx = CONTAINER_OF(cb, struct adf702x_context,
						   irq_cb);
	ARG_UNUSED(port);
	ARG_UNUSED(pins);

	k_sem_give(&ctx->isr_lock);
}

static void adf702x_process_rx_frame(const struct device *dev)
{
	adf702x_packet_read(dev, "RX");
}

static void adf702x_process_tx_frame(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;

	LOG_INST_INF(conf->log, "Packet sent, clearing irq");

	k_sem_give(&ctx->tx_wait);
}

static void adf702x_thread_main(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	struct adf702x_context *ctx = p1;
	const struct device *dev = ctx->dev;
	const struct adf702x_config *conf = dev->config;
	uint8_t isr_status[2] = {0};
	uint8_t isr_update[2] = {0};
	int ret;

	while (true) {
		k_sem_take(&ctx->isr_lock, K_FOREVER);
		ret = adf702x_ram_read(dev, ADF702X_REG_INTERRUPT_SOURCE_0, 2, isr_status);
		LOG_INST_INF(conf->log, "got IRQ 0x%x 0x%x", isr_status[0], isr_status[1]);

		if (isr_status[0] & ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT) {
			isr_update[0] |= ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT;
			adf702x_process_rx_frame(dev);
		}

		if (isr_status[0] & ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_TX_EOF) {
			isr_update[0] |= ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_TX_EOF;
			adf702x_process_tx_frame(dev);
		}

		if (ctx->conf_regs.interrupt_mask0 & !isr_update[0])
			LOG_INST_WRN(conf->log, "Unhandled IRQ0: 0x%02x", isr_status[0]);
		if (ctx->conf_regs.interrupt_mask1 & !isr_update[1])
			LOG_INST_WRN(conf->log, "Unhandled IRQ1: 0x%02x", isr_status[1]);

		// clear processed irq
		ret = adf702x_ram_write(dev, ADF702X_REG_INTERRUPT_SOURCE_0, 2, isr_update);
		// go back to waiting for packets
		adf702x_set_fw_state(dev, FW_STATE_PHY_RX);
	}
}


static inline int adf702x_configure_irq(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;

	if (!gpio_is_ready_dt(&conf->irq_gpio)) {
		LOG_INST_ERR(conf->log, "IRQ GPIO not ready");
		return -ENODEV;
	}
	gpio_pin_configure_dt(&conf->irq_gpio, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(&conf->irq_gpio, GPIO_INT_EDGE_TO_ACTIVE);

	gpio_init_callback(&ctx->irq_cb, adf702x_irq_handler, BIT(conf->irq_gpio.pin));

	if (gpio_add_callback(conf->irq_gpio.port, &ctx->irq_cb) < 0) {
		LOG_INST_ERR(conf->log, "Could not set IRQ callback.");
		return -ENXIO;
	}

	LOG_INST_INF(conf->log, "irq setup at %s pin %d", conf->irq_gpio.port->name, conf->irq_gpio.pin);

	return 0;
}

static inline int adf702x_configure_gpios(const struct device *dev)
{
	// TODO configure gpios for firecodes

	return 0;
}

static inline int adf702x_configure_spi(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;

	if (!spi_is_ready_dt(&conf->spi)) {
		LOG_INST_ERR(conf->log, "SPI bus %s is not ready", conf->spi.bus->name);
		return -ENODEV;
	}

	return 0;
}

static int adf702x_init(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	char thread_name[20];

	LOG_INST_INF(conf->log, "Initializing ADF702X Transceiver %s", conf->name);

	ctx->dev = dev;
	k_sem_init(&ctx->isr_lock, 0, 1);
	k_sem_init(&ctx->tx_wait, 0, 1);

	if (conf->irq_gpio.port && adf702x_configure_irq(dev)) {
		LOG_INST_ERR(conf->log, "Unable to configure IRQ");
		return -EIO;
	}

	if (adf702x_configure_spi(dev)) {
		LOG_INST_ERR(conf->log, "Unable to configure SPI");
		return -EIO;
	}

	adf702x_set_command(dev, CMD_HW_RESET);
	k_sleep(K_MSEC(1));

	while(!(ctx->status & STATUS_CMD_READY))
		adf702x_get_status(dev);

	ctx->conf_regs = adf702x_default_conf_regs;
	adf702x_regs_set_channel_freq(dev, conf->channel_frequency);
	adf702x_regs_set_pa_level(dev, 13.5);
	adf7024_regs_set_profile(dev, PROFILE_A);

	adf702x_ram_write(dev, 0x100, 64, (uint8_t *)&ctx->conf_regs);
	adf702x_set_command(dev, CMD_CONFIG_DEV);

	while(!(ctx->status & STATUS_CMD_READY))
		adf702x_get_status(dev);

	LOG_INST_INF(conf->log, "Configured, status: %X",
		(uint8_t)FIELD_GET(STATUS_FW_STATE, ctx->status));

	k_thread_create(&ctx->trx_thread,
			ctx->trx_stack,
			CONFIG_IEEE802154_ADF702X_RX_STACK_SIZE,
			adf702x_thread_main,
			ctx, NULL, NULL,
			K_PRIO_COOP(2), 0, K_NO_WAIT);

	snprintk(thread_name, sizeof(thread_name),
		 "adf702x_trx [%d]", conf->inst);
	k_thread_name_set(&ctx->trx_thread, thread_name);

	return 0;
}

#define IEEE802154_ADF702X_DEVICE_CONFIG(n)				\
	LOG_INSTANCE_REGISTER(LOG_MODULE_NAME, n, LOG_LEVEL_DBG);	\
	static const struct adf702x_config adf702x_ctx_config_##n = {	\
		.inst = n,						\
		.name = DT_NODE_FULL_NAME(DT_DRV_INST(n)),		\
		.irq_gpio = GPIO_DT_SPEC_INST_GET(n, irq_gpios),	\
		.spi = SPI_DT_SPEC_INST_GET(n, SPI_WORD_SET(8) |	\
				            SPI_TRANSFER_MSB, 0),	\
		.channel_frequency = DT_PROP_OR(DT_DRV_INST(n),		\
						channel_frequency,	\
						869000000),		\
		LOG_INSTANCE_PTR_INIT(log, LOG_MODULE_NAME, n)		\
	}

#define IEEE802154_ADF702X_DEVICE_DATA(n)				\
	static struct adf702x_context adf702x_ctx_data_##n = {		\
	}

#define IEEE802154_ADF702X_RAW_DEVICE_INIT(n)				\
	DEVICE_DT_INST_DEFINE(						\
		n,							\
		&adf702x_init,						\
		NULL,							\
		&adf702x_ctx_data_##n,					\
		&adf702x_ctx_config_##n,				\
		POST_KERNEL,						\
		CONFIG_IEEE802154_ADF702X_INIT_PRIO,			\
		&adf702x_radio_api)

#define IEEE802154_ADF702X_NET_DEVICE_INIT(n)				\
	NET_DEVICE_DT_INST_DEFINE(					\
		n,							\
		&adf702x_init,						\
		NULL,							\
		&adf702x_ctx_data_##n,					\
		&adf702x_ctx_config_##n,				\
		CONFIG_IEEE802154_ADF702X_INIT_PRIO,			\
		&adf702x_radio_api,					\
		IEEE802154_L2,						\
		NET_L2_GET_CTX_TYPE(IEEE802154_L2),			\
		125)

#define IEEE802154_ADF702X_INIT(inst)					\
	IEEE802154_ADF702X_DEVICE_CONFIG(inst);				\
	IEEE802154_ADF702X_DEVICE_DATA(inst);				\
									\
	COND_CODE_1(CONFIG_IEEE802154_RAW_MODE,				\
		    (IEEE802154_ADF702X_RAW_DEVICE_INIT(inst);),	\
		    (IEEE802154_ADF702X_NET_DEVICE_INIT(inst);))

DT_INST_FOREACH_STATUS_OKAY(IEEE802154_ADF702X_INIT)
