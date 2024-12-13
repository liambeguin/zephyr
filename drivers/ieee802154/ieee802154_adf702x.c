/*
 * Copyright (c) 2024 HawkEye 360
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT adi_adf702x
#define LOG_MODULE_NAME adf702x

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME, CONFIG_IEEE802154_DRIVER_LOG_LEVEL);

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/debug/stack.h>

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/net/dummy.h>
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

#include "ieee802154_adf702x.h"

#define MAX_POLL_LOOPS 200


static int adf7024_regs_set_profile(const struct device *dev, enum adf7024_profile profile)
{
	struct adf702x_context *ctx = dev->data;
	// default with AFC ON and FSK mod
	const uint8_t adf7024_radio_profile_regs[][23] = {
		[PROFILE_A] = {
		       //0   1     2     3     4     5     6     7
		       0x60, 0x00, 0x60, 0x20, 0x04, 0x00, 0x00, 0x00,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_B] = {
		       0x80, 0x01, 0xC8, 0x20, 0x0E, 0x00, 0x00, 0x00,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_C] = {
		       0xF4, 0x01, 0xFA, 0x20, 0x13, 0x00, 0x00, 0x00,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_D] = {
		       0xE8, 0x03, 0xFA, 0x20, 0x26, 0x00, 0x00, 0x00,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_E] = {
		       0xD0, 0x17, 0xF4, 0x20, 0x4B, 0x00, 0x00, 0x80,
		       0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
		[PROFILE_F] = {
			0xB8, 0x2B, 0xEE, 0x16, 0x70, 0x00, 0x00, 0xC0,
			0x37, 0x2B, 0x2F, 0x12, 0x07, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA7 },
	};

	ctx->conf_regs.radio_profile_0  = adf7024_radio_profile_regs[profile][0];
	ctx->conf_regs.radio_profile_1  = adf7024_radio_profile_regs[profile][1];
	ctx->conf_regs.radio_profile_2  = adf7024_radio_profile_regs[profile][2];
	ctx->conf_regs.radio_profile_3  = adf7024_radio_profile_regs[profile][3];
	ctx->conf_regs.radio_profile_4  = adf7024_radio_profile_regs[profile][4];
	ctx->conf_regs.radio_profile_5  = adf7024_radio_profile_regs[profile][5];
	ctx->conf_regs.radio_profile_6  = adf7024_radio_profile_regs[profile][6];
	ctx->conf_regs.radio_profile_7  = adf7024_radio_profile_regs[profile][7];
	ctx->conf_regs.radio_profile_8  = adf7024_radio_profile_regs[profile][8];
	ctx->conf_regs.radio_profile_9  = adf7024_radio_profile_regs[profile][9];
	ctx->conf_regs.radio_profile_10 = adf7024_radio_profile_regs[profile][10];
	ctx->conf_regs.radio_profile_11 = adf7024_radio_profile_regs[profile][11];
	ctx->conf_regs.radio_profile_12 = adf7024_radio_profile_regs[profile][12];
	ctx->conf_regs.radio_profile_13 = adf7024_radio_profile_regs[profile][13];
	ctx->conf_regs.radio_profile_14 = adf7024_radio_profile_regs[profile][14];
	ctx->conf_regs.radio_profile_15 = adf7024_radio_profile_regs[profile][15];
	ctx->conf_regs.radio_profile_16 = adf7024_radio_profile_regs[profile][16];
	ctx->conf_regs.radio_profile_17 = adf7024_radio_profile_regs[profile][17];
	ctx->conf_regs.radio_profile_18 = adf7024_radio_profile_regs[profile][18];
	ctx->conf_regs.radio_profile_19 = adf7024_radio_profile_regs[profile][19];
	ctx->conf_regs.radio_profile_20 = adf7024_radio_profile_regs[profile][20];
	ctx->conf_regs.radio_profile_21 = adf7024_radio_profile_regs[profile][21];
	ctx->conf_regs.radio_profile_22 = adf7024_radio_profile_regs[profile][22];

	// if (ctx->conf_regs.radio_afc_mode == ADF702X_VAL_RADIO_AFC_MODE_LOCK) {
		// radio_profile_3;
		// radio_profile_6;
	// }

	// MOD scheme
	// if (ctx->conf_regs.radio_afc_mode == ADF702X_VAL_RADIO_AFC_MODE_LOCK) {
	// just GFSK for now, set bit[3]
	ctx->conf_regs.radio_profile_7 |= 0x08;

	return 0;
};

#if 0
struct adf702x_conf_regs adf702x_default_conf_regs = {
	.interrupt_mask0 = ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_TX_EOF |
			   ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT,
	.interrupt_mask1 = 0x00,
	.number_of_wakeups0 = 0x00,
	.number_of_wakeups1 = 0x00,
	.number_of_wakeups_irq_threshold0 = 0xFF,
	.number_of_wakeups_irq_threshold1 = 0xFF,
	.rx_dwell_time		= 0x00,
	.parmtime_divider	= 0x33,
	.swm_rssi_thresh	= 0x31,
	.channel_freq0		= 0x51, // Channel Frequency: 433 MHz
	.channel_freq1		= 0xA7, // Channel Frequency: 433 MHz
	.channel_freq2		= 0x10, // Channel Frequency: 433 MHz
	.radio_cfg0  = ADF702X_BIT_RADIO_CFG_0_DATA_RATE_7_0(0xE8),		// Data rate: 100 kbps
	.radio_cfg1  = ADF702X_BIT_RADIO_CFG_1_FREQ_DEVIATION_11_8(0x00) |	// Frequency deviation: 25 Hz
		       ADF702X_BIT_RADIO_CFG_1_DATA_RATE_11_8(0x03),		// Data rate: 100 kbps
	.radio_cfg2  = ADF702X_BIT_RADIO_CFG_2_FREQ_DEVIATION_7_0(0xFA),	// Frequency deviation: 25 Hz
	.radio_cfg3  = 0x31,
	.radio_cfg4  = 0x16,
	.radio_cfg5  = 0x00,
	.radio_cfg6  = ADF702X_BIT_RADIO_CFG_6_DISCRIM_PHASE(0x2),
	.radio_cfg7  = ADF702X_BIT_RADIO_CFG_7_AGC_LOCK_MODE(3),
	.radio_cfg8  = ADF702X_BIT_RADIO_CFG_8_PA_SINGLE_DIFF_SEL |
		       ADF702X_BIT_RADIO_CFG_8_PA_LEVEL(0xF) |
		       ADF702X_BIT_RADIO_CFG_8_PA_RAMP(1),
	.radio_cfg9  = ADF702X_BIT_RADIO_CFG_9_IFBW(2),
	.radio_cfg10 = ADF702X_BIT_RADIO_CFG_10_AFC_SCHEME(2) |
		       ADF702X_BIT_RADIO_CFG_10_AFC_LOCK_MODE(3),
	.radio_cfg11 = ADF702X_BIT_RADIO_CFG_11_AFC_KP(3) |
		       ADF702X_BIT_RADIO_CFG_11_AFC_KI(7),
	.image_reject_cal_phase = 0x00,
	.image_reject_cal_amplitude = 0x00,
	.mode_control = ADF702X_BIT_MODE_CONTROL_BB_CAL,
	.preamble_match = 0x0C,
	.symbol_mode = ADF702X_BIT_SYMBOL_MODE_MANCHESTER_ENC,
	.preamble_len = 0x20,
	.crc_poly0 = 0x00,
	.crc_poly1 = 0x00,
	.sync_control = ADF702X_BIT_SYNC_CONTROL_SYNC_WORD_LENGTH(8),
	.sync_byte0 = 0x00,
	.sync_byte1 = 0x00,
	.sync_byte2 = 0x12,
	.tx_base_adr = ADF702X_TX_BASE_ADR,
	.rx_base_adr = ADF702X_RX_BASE_ADR,
	.packet_length_control = 0x24,
	.packet_length_max = 0xF0,
	.static_reg_fix = 0x00,
	.address_match_offset = 0x01,
	.address_length = 0x02,
	.address_filtering0  = 0x01,
	.address_filtering1  = 0xFF,
	.address_filtering2  = 0xFF,
	.address_filtering3  = 0xFF,
	.address_filtering4  = 0x02,
	.address_filtering5  = 0x0F,
	.address_filtering6  = 0xFF,
	.address_filtering7  = 0x0F,
	.address_filtering8  = 0xFF,
	.address_filtering9  = 0x00,
	.address_filtering10 = 0x00,
	.address_filtering11 = 0x00,
	.address_filtering12 = 0x00,
	.rssi_wait_time = 0x00,
	.testmodes = 0x00,
	.transition_clock_div = 0x00,
	.reserved3 = 0x00,
	.reserved4 = 0x00,
	.reserved5 = 0x00,
	.rx_synth_lock_time = 0x00,
	.tx_synth_lock_time = 0x00,
};
#else
struct adf702x_conf_regs adf702x_default_conf_regs = {
	.interrupt_mask0 = ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_TX_EOF |
			   ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT,
	.interrupt_mask1 = 0x00,
	.number_of_wakeups0 = 0x00,
	.number_of_wakeups1 = 0x00,
	.number_of_wakeups_irq_threshold0 = 0x00,
	.number_of_wakeups_irq_threshold1 = 0x00,
	.rx_dwell_time		= 0x00,
	.parmtime_divider	= 0x00,
	.swm_rssi_thresh	= 0x00,
	.channel_freq0		= 0x40, // Channel Frequency: 869 MHz
	.channel_freq1		= 0xE3, // Channel Frequency: 869 MHz
	.channel_freq2		= 0xCB, // Channel Frequency: 869 MHz
	// .radio_profile_x set in function
	.radio_agc_mode  = ADF702X_BIT_RADIO_CFG_7_AGC_LOCK_MODE(3),
	.radio_pa_ramp  = ADF702X_BIT_RADIO_CFG_8_PA_RAMP(4),
	.radio_afc_mode = ADF702X_BIT_RADIO_AFC_MODE_LOCK_MODE(3),
	.image_reject_cal_phase = 0x00,
	.image_reject_cal_amplitude = 0x00,
	.mode_control = ADF702X_BIT_MODE_CONTROL_BB_CAL,
	.preamble_match = ADF702X_VAL_PREAMBLE_MATCH_4_12,
	.symbol_mode = ADF702X_BIT_SYMBOL_MODE_PROG_CRC_EN, // Reserved on adf7024
	.preamble_len = 0x0A,
	.crc_poly0 = 0x10,
	.crc_poly1 = 0x21,
	.sync_control = ADF702X_BIT_SYNC_CONTROL_SYNC_WORD_LENGTH(24) |
			ADF702X_BIT_SYNC_CONTROL_SYNC_ERROR_TOL(3),
	.sync_byte0 = 0x12,
	.sync_byte1 = 0x34,
	.sync_byte2 = 0x56,
	.tx_base_adr = ADF702X_TX_BASE_ADR,
	.rx_base_adr = ADF702X_RX_BASE_ADR,
	.packet_length_control = ADF702X_BIT_PKT_LENGTH_CONTROL_DATA_BYTE |
				 ADF702X_BIT_PKT_LENGTH_CONTROL_CRC_EN |
				 ADF702X_BIT_PKT_LENGTH_CONTROL_LENGTH_OFFSET(4),
	.packet_length_max = 240,
	.radio_pa_level  = 0x3F, // +13.5 dBm
	.testmodes = 0x00,
};
#endif //0


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

	/* adf702x_packet_read(dev, "TX"); */

	return 0;
}

static int adf702x_regs_set_channel_freq(const struct device *dev, uint32_t freq)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	uint32_t ch_freq = (uint32_t)(((float)freq / 26000000) * 65535);

	LOG_INST_INF(conf->log, "Setting channel frequency to %d Hz", freq);

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
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;

	sys_rand_get(ctx->mac, 8U);
	net_if_set_link_addr(iface, ctx->mac, 8, NET_LINK_IEEE802154);

	LOG_INST_DBG(conf->log, "iface init");
	LOG_INST_HEXDUMP_DBG(conf->log, ctx->mac, 8, "MAC: ");

	ctx->iface = iface;

	ieee802154_init(iface);
}

static enum ieee802154_hw_caps adf702x_get_capabilities(const struct device *dev)
{
	const struct adf702x_config *conf = dev->config;

	LOG_INST_DBG(conf->log, "iface hw_caps");

	return IEEE802154_HW_FCS;
}

static int adf702x_cca(const struct device *dev)
{
	struct adf702x_context *ctx = dev->data;
	ARG_UNUSED(dev);

	if (!ctx->is_up)
		return -EIO;

	return 0;
}

static int adf702x_set_channel(const struct device *dev, uint16_t channel)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(channel);

	return 0;
}

static int adf702x_filter(const struct device *dev,
		bool set,
		enum ieee802154_filter_type type,
		const struct ieee802154_filter *filter)
{
	const struct adf702x_config *conf = dev->config;

	LOG_INST_DBG(conf->log, "Applying filter %u", type);

	return 0;
}

static int adf702x_set_txpower(const struct device *dev, int16_t dBm)
{
	const struct adf702x_config *conf = dev->config;

	LOG_INST_DBG(conf->log, "%s: dBm=%d", __func__, dBm);

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

	LOG_INST_DBG(conf->log, "%s: entering", __func__);

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

	ret = adf702x_set_fw_state(dev, FW_STATE_PHY_RX);
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

/* driver-allocated attribute memory - constant across all driver instances */
IEEE802154_DEFINE_PHY_SUPPORTED_CHANNELS(drv_attr, 11, 26);

// dummy always return suppored_ch pages
static int adf702x_attr_get(const struct device *dev, enum ieee802154_attr attr,
			    struct ieee802154_attr_value *value)
{
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	uint8_t bram[64] = {0};

	if (ieee802154_attr_get_channel_page_and_range(
				attr, IEEE802154_ATTR_PHY_CHANNEL_PAGE_ZERO_OQPSK_2450_BPSK_868_915,
				&drv_attr.phy_supported_channels, value) == 0) {
		return 0;
	}

	switch ((int)attr) {
	case IEEE802154_ATTR_PHY_SUPPORTED_CHANNEL_PAGES:
		value->phy_supported_channel_pages = ctx->cc_page;
		return 0;

	case IEEE802154_ATTR_PHY_SUPPORTED_CHANNEL_RANGES:
		value->phy_supported_channels = &ctx->cc_channels;
		return 0;
	case IEEE802154_ATTR_ADF702X_STATUS:
		adf702x_get_status(dev);
		adf702x_print_status(dev);
		break;
	case IEEE802154_ATTR_ADF702X_RAW_REG:
		adf702x_ram_read(dev, value->phy_supported_channel_pages, 1, bram);
		value->phy_supported_channel_pages = bram[0];
		LOG_INST_WRN(conf->log, "value: %02X", bram[0]);
		break;
	case IEEE802154_ATTR_ADF702X_DUMP:
		adf702x_ram_dump_conf(dev);
		break;
	default:
		return -ENOENT;
	}

	return 0;
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
	.cca			= adf702x_cca,
	.set_channel		= adf702x_set_channel,
	.filter			= adf702x_filter,
	.set_txpower		= adf702x_set_txpower,
	.tx			= adf702x_tx,
	.start			= adf702x_start,
	.stop			= adf702x_stop,
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
	const struct adf702x_config *conf = dev->config;
	struct adf702x_context *ctx = dev->data;
	uint8_t pkt_ram[256] = {0};
	uint8_t header[1] = {0};
	struct net_pkt *pkt;
	uint8_t len = 0;

	adf702x_ram_read(dev, ADF702X_RX_BASE_ADR, 1, header);
	len = header[0] - 1;

	adf702x_ram_read(dev, ADF702X_RX_BASE_ADR + 1, len, pkt_ram);
	pkt = net_pkt_rx_alloc_with_buffer(ctx->iface, len, AF_UNSPEC, 0, K_NO_WAIT);
	if (!pkt) {
		LOG_INST_ERR(conf->log, "No free pkt available");
		goto flush;
	}

	if (net_pkt_write(pkt, pkt_ram, len)) {
		LOG_INST_DBG(conf->log, "No content read?");
		goto out;
	}

	LOG_INST_INF(conf->log, "caught packet %p (%u bytes)", pkt, len);
	if (net_recv_data(ctx->iface, pkt) < 0) {
		LOG_INST_DBG(conf->log, "Packet dropped by NET stack");
		goto out;
	}

	return;

flush:
	LOG_INST_DBG(conf->log, "flushing RX");
out:
	if (pkt) {
		net_pkt_unref(pkt);
	}
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
	adf7024_regs_set_profile(dev, conf->radio_profile);

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

#define IEEE802154_ADF702X_DEVICE_CONFIG(n)                                                        \
	LOG_INSTANCE_REGISTER(LOG_MODULE_NAME, DT_INST_REG_ADDR_RAW(n), LOG_LEVEL_DBG);            \
	static const struct adf702x_config adf702x_ctx_config_##n = {                              \
		.inst = DT_INST_REG_ADDR_RAW(n),                                                   \
		.name = DT_NODE_FULL_NAME(DT_DRV_INST(n)),                                         \
		.irq_gpio = GPIO_DT_SPEC_INST_GET(n, irq_gpios),                                   \
		.spi = SPI_DT_SPEC_INST_GET(n, SPI_WORD_SET(8) | SPI_TRANSFER_MSB, 0),             \
		.channel_frequency = DT_PROP_OR(DT_DRV_INST(n), channel_frequency, 869000000),     \
		.radio_profile = DT_INST_PROP_OR(n, adf7024_radio_profile, PROFILE_A),             \
		LOG_INSTANCE_PTR_INIT(log, LOG_MODULE_NAME, DT_INST_REG_ADDR_RAW(n))}

#define IEEE802154_ADF702X_DEVICE_DATA(n) static struct adf702x_context adf702x_ctx_data_##n = {}

#define IEEE802154_ADF702X_RAW_DEVICE_INIT(n)                                                      \
	DEVICE_DT_INST_DEFINE(n, &adf702x_init, NULL, &adf702x_ctx_data_##n,                       \
			      &adf702x_ctx_config_##n, POST_KERNEL,                                \
			      CONFIG_IEEE802154_ADF702X_INIT_PRIO, &adf702x_radio_api)

#define IEEE802154_ADF702X_NET_DEVICE_INIT(n)                                                      \
	NET_DEVICE_DT_INST_DEFINE(n, &adf702x_init, NULL, &adf702x_ctx_data_##n,                   \
				  &adf702x_ctx_config_##n, CONFIG_IEEE802154_ADF702X_INIT_PRIO,    \
				  &adf702x_radio_api, IEEE802154_L2,                               \
				  NET_L2_GET_CTX_TYPE(IEEE802154_L2), IEEE802154_MTU);

#define IEEE802154_ADF702X_INIT(inst)                                                              \
	IEEE802154_ADF702X_DEVICE_CONFIG(inst);                                                    \
	IEEE802154_ADF702X_DEVICE_DATA(inst);                                                      \
                                                                                                   \
	COND_CODE_1(CONFIG_IEEE802154_RAW_MODE,				\
		    (IEEE802154_ADF702X_RAW_DEVICE_INIT(inst);),	\
		    (IEEE802154_ADF702X_NET_DEVICE_INIT(inst);))

DT_INST_FOREACH_STATUS_OKAY(IEEE802154_ADF702X_INIT)
