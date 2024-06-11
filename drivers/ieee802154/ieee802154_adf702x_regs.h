/*
 * Copyright (c) 2024 HawkEye 360
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_REGS_H_
#define ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_REGS_H_

/* 0x000 - 0x00F Auxiliary RAM */
// 0x000 - 0x00C Reserved
#define ADF702X_REG_AUX_VAR_TX_MODE			0x00D
// 0x00E - 0x00F Reserved

/* 0x010 - 0x0FF Packet RAM */
#define ADF702X_TX_BASE_ADR 0x10
#define ADF702X_RX_BASE_ADR 0x10

/* 0x100 - 0x13F Configuration Registers */
#define ADF702X_REG_INTERRUPT_MASK_0			0x100
#define ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_NUM_WAKEUPS	BIT(7)
#define ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_SWM_RSSI_DET	BIT(6)
#define ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_AES_DONE		BIT(5) // ADF7023
#define ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_TX_EOF		BIT(4)
#define ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_ADDRESS_MATCH	BIT(3) // ADF7023
#define ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_CRC_CORRECT	BIT(2)
#define ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_SYNC_DETECT	BIT(1)
#define ADF702X_BIT_INTERRUPT_MASK_0_INTERRUPT_PREMABLE_DETECT	BIT(0)

#define ADF702X_REG_INTERRUPT_MASK_1			0x101
#define ADF702X_BIT_INTERRUPT_MASK_1_BATTERY_ALARM		BIT(7)
#define ADF702X_BIT_INTERRUPT_MASK_1_CMD_READY			BIT(6)
#define ADF702X_BIT_INTERRUPT_MASK_1_WUC_TIMEOUT		BIT(4)
#define ADF702X_BIT_INTERRUPT_MASK_1_SPI_READY			BIT(1)
#define ADF702X_BIT_INTERRUPT_MASK_1_CMD_FINISHED		BIT(0)

#define ADF702X_REG_NUMBER_OF_WAKEUPS_0			0x102
#define ADF702X_REG_NUMBER_OF_WAKEUPS_1			0x103
#define ADF702X_REG_NUMBER_OF_WAKEUPS_IRQ_THRESHOLD_0	0x104
#define ADF702X_REG_NUMBER_OF_WAKEUPS_IRQ_THRESHOLD_1	0x105
#define ADF702X_REG_RX_DWELL_TIME			0x106
#define ADF702X_REG_PARMTIME_DIVIDER			0x107
#define ADF702X_REG_SWM_RSSI_THRESH			0x108
#define ADF702X_REG_CHANNEL_FREQ_0			0x109
#define ADF702X_REG_CHANNEL_FREQ_1			0x10A
#define ADF702X_REG_CHANNEL_FREQ_2			0x10B

// ADF7023
#define ADF702X_REG_RADIO_CFG_0				0x10C
#define ADF702X_BIT_RADIO_CFG_0_DATA_RATE_7_0(x)		((x & 0xFF) << 0)

#define ADF702X_REG_RADIO_CFG_1				0x10D
#define ADF702X_BIT_RADIO_CFG_1_FREQ_DEVIATION_11_8(x)		((x & 0xF) << 4)
#define ADF702X_BIT_RADIO_CFG_1_DATA_RATE_11_8(x)		((x & 0xF) << 0)

#define ADF702X_REG_RADIO_CFG_2				0x10E
#define ADF702X_BIT_RADIO_CFG_2_FREQ_DEVIATION_7_0(x)		((x & 0xFF) << 0)

#define ADF702X_REG_RADIO_CFG_3				0x10F
#define ADF702X_REG_RADIO_CFG_4				0x110
#define ADF702X_REG_RADIO_CFG_5				0x111
#define ADF702X_REG_RADIO_CFG_6				0x112
#define ADF702X_BIT_RADIO_CFG_6_SYNTH_LUT_CONFIG_0(x)		((x & 0x3F) << 2)
#define ADF702X_BIT_RADIO_CFG_6_DISCRIM_PHASE(x)		((x & 0x3) << 0)

#define ADF702X_REG_RADIO_CFG_7				0x113
#define ADF702X_BIT_RADIO_CFG_7_AGC_LOCK_MODE(x)		((x & 0x3) << 6)
#define ADF702X_BIT_RADIO_CFG_7_SYNTH_LUT_CONTROL(x)		((x & 0x3) << 4)
#define ADF702X_BIT_RADIO_CFG_7_SYNTH_LUT_CONFIG_1(x)		((x & 0xF) << 0)

#define ADF702X_REG_RADIO_CFG_8				0x114
#define ADF702X_BIT_RADIO_CFG_8_PA_SINGLE_DIFF_SEL		(0x1 << 7)
#define ADF702X_BIT_RADIO_CFG_8_PA_LEVEL(x)			((x & 0xF) << 3)
#define ADF702X_BIT_RADIO_CFG_8_PA_RAMP(x)			((x & 0x7) << 0)

#define ADF702X_REG_RADIO_CFG_9				0x115
#define ADF702X_BIT_RADIO_CFG_9_IFBW(x)				((x & 0x3) << 6)
#define ADF702X_BIT_RADIO_CFG_9_MOD_SCHEME(x)			((x & 0x7) << 3)
#define ADF702X_BIT_RADIO_CFG_9_DEMOD_SCHEME(x)			((x & 0x7) << 0)

#define ADF702X_REG_RADIO_CFG_10			0x116
#define ADF702X_BIT_RADIO_CFG_10_AFC_POLARITY			(0x0 << 4)
#define ADF702X_BIT_RADIO_CFG_10_AFC_SCHEME(x)			((x & 0x3) << 2)
#define ADF702X_BIT_RADIO_CFG_10_AFC_LOCK_MODE(x)		((x & 0x3) << 0)

#define ADF702X_REG_RADIO_CFG_11			0x117
#define ADF702X_BIT_RADIO_CFG_11_AFC_KP(x)			((x & 0xF) << 4)
#define ADF702X_BIT_RADIO_CFG_11_AFC_KI(x)			((x & 0xF) << 0)

// ADF7024
#define ADF702X_REG_RADIO_PROFILE_0			0x10C
#define ADF702X_REG_RADIO_PROFILE_1			0x10D
#define ADF702X_REG_RADIO_PROFILE_2			0x10E
#define ADF702X_REG_RADIO_PROFILE_3			0x10F
#define ADF702X_REG_RADIO_PROFILE_4			0x110
#define ADF702X_REG_RADIO_PROFILE_5			0x111
#define ADF702X_REG_RADIO_PROFILE_6			0x112
#define ADF702X_REG_RADIO_AGC_MODE			0x113
#define ADF702X_REG_RADIO_PA_RAMP			0x114
#define ADF702X_REG_RADIO_PROFILE_7			0x115
#define ADF702X_REG_RADIO_AFC_MODE			0x116
#define ADF702X_BIT_RADIO_AFC_MODE_LOCK_MODE(x)			((x & 0x3) << 0)
#define ADF702X_VAL_RADIO_AFC_MODE_FREERUN				0x00
#define ADF702X_VAL_RADIO_AFC_MODE_DISABLED				0x01
#define ADF702X_VAL_RADIO_AFC_MODE_HOLD					0x02
#define ADF702X_VAL_RADIO_AFC_MODE_LOCK					0x03
#define ADF702X_REG_RADIO_PROFILE_8			0x117

#define ADF702X_REG_IMAGE_REJECT_CAL_PHASE		0x118
#define ADF702X_REG_IMAGE_REJECT_CAL_AMPLITUDE		0x119
#define ADF702X_REG_MODE_CONTROL			0x11A
#define ADF702X_BIT_MODE_CONTROL_SWM_EN				BIT(7)
#define ADF702X_BIT_MODE_CONTROL_BB_CAL				BIT(6)
#define ADF702X_BIT_MODE_CONTROL_SWM_RSSI_QUAL			BIT(5)
#define ADF702X_BIT_MODE_CONTROL_TX_TO_RX_AUTO_TURNAROUND	BIT(4)
#define ADF702X_BIT_MODE_CONTROL_RX_TO_TX_AUTO_TURNAROUND	BIT(3)
#define ADF702X_BIT_MODE_CONTROL_CUSTOM_TRX_SYNTH_LOCK_TIME_EN	BIT(2)
#define ADF702X_BIT_MODE_CONTROL_EXT_LNA_EN			BIT(1)
#define ADF702X_BIT_MODE_CONTROL_EXT_PA_EN			BIT(0)

#define ADF702X_REG_PREAMBLE_MATCH			0x11B
#define ADF702X_BIT_PREAMBLE_MATCH				0x0F
#define ADF702X_VAL_PREAMBLE_MATCH_OFF				0x00
#define ADF702X_VAL_PREAMBLE_MATCH_4_12				0x08
#define ADF702X_VAL_PREAMBLE_MATCH_3_12				0x09
#define ADF702X_VAL_PREAMBLE_MATCH_2_12				0x0a
#define ADF702X_VAL_PREAMBLE_MATCH_1_12				0x0b
#define ADF702X_VAL_PREAMBLE_MATCH_NO_ERROR			0x0c

#define ADF702X_REG_SYMBOL_MODE				0x11C
#define ADF702X_BIT_SYMBOL_MODE_MANCHESTER_ENC			BIT(6)
#define ADF702X_BIT_SYMBOL_MODE_PROG_CRC_EN			BIT(5)
#define ADF702X_BIT_SYMBOL_MODE_EIGHT_TEN_ENC			BIT(4)
#define ADF702X_BIT_SYMBOL_MODE_DATA_WHITENING			BIT(3)
#define ADF702X_BIT_SYMBOL_MODE_SYMBOL_LENGTH(x)		((x & 0x7) << 0)

#define ADF702X_REG_PREAMBLE_LEN			0x11D
#define ADF702X_REG_CRC_POLY_0				0x11E
#define ADF702X_REG_CRC_POLY_1				0x11F
#define ADF702X_REG_SYNC_CONTROL			0x120
#define ADF702X_BIT_SYNC_CONTROL_SYNC_ERROR_TOL(x)		((x & 0x3) << 6)
#define ADF702X_BIT_SYNC_CONTROL_SYNC_WORD_LENGTH(x)		((x & 0x1F) << 0)

#define ADF702X_REG_SYNC_BYTE_0				0x121
#define ADF702X_REG_SYNC_BYTE_1				0x122
#define ADF702X_REG_SYNC_BYTE_2				0x123
#define ADF702X_REG_TX_BASE_ADR				0x124
#define ADF702X_REG_RX_BASE_ADR				0x125
#define ADF702X_REG_PKT_LENGTH_CONTROL			0x126
#define ADF702X_BIT_PKT_LENGTH_CONTROL_DATA_BYTE		BIT(7)
#define ADF702X_BIT_PKT_LENGTH_CONTROL_PKT_LEN_MODE		BIT(6)
#define ADF702X_BIT_PKT_LENGTH_CONTROL_CRC_EN			BIT(5)
#define ADF702X_BIT_PKT_LENGTH_CONTROL_DATA_MODE(x)		((x & 0x3) << 3)
#define ADF702X_BIT_PKT_LENGTH_CONTROL_LENGTH_OFFSET(x)		((x & 0x7) << 0)

#define ADF702X_REG_PACKET_LENGTH_MAX			0x127

// ADF7023
#define ADF702X_REG_STATIC_REG_FIX			0x128
#define ADF702X_REG_ADDRESS_MATCH_OFFSET		0x129
#define ADF702X_REG_ADDRESS_LENGTH			0x12A
#define ADF702X_REG_ADDRESS_FILTERING_0			0x12B
#define ADF702X_REG_ADDRESS_FILTERING_1			0x12C
#define ADF702X_REG_ADDRESS_FILTERING_2			0x12D
#define ADF702X_REG_ADDRESS_FILTERING_3			0x12E
#define ADF702X_REG_ADDRESS_FILTERING_4			0x12F
#define ADF702X_REG_ADDRESS_FILTERING_5			0x130
#define ADF702X_REG_ADDRESS_FILTERING_6			0x131
#define ADF702X_REG_ADDRESS_FILTERING_7			0x132
#define ADF702X_REG_ADDRESS_FILTERING_8			0x133
#define ADF702X_REG_ADDRESS_FILTERING_9			0x134
#define ADF702X_REG_ADDRESS_FILTERING_10		0x135
#define ADF702X_REG_ADDRESS_FILTERING_11		0x136
#define ADF702X_REG_ADDRESS_FILTERING_12		0x137
#define ADF702X_REG_RSSI_WAIT_TIME			0x138

// ADF7024
#define ADF702X_REG_RADIO_PROFILE_9			0x128
// Reserved						0x129
// Reserved 						0x12A
#define ADF702X_REG_RADIO_PROFILE_10			0x12B
#define ADF702X_REG_RADIO_PROFILE_11			0x12C
#define ADF702X_REG_RADIO_PROFILE_12			0x12D
#define ADF702X_REG_RADIO_PA_LEVEL			0x12E
#define ADF702X_REG_RADIO_PROFILE_13			0x12F
#define ADF702X_REG_RADIO_PROFILE_14			0x130
#define ADF702X_REG_RADIO_PROFILE_15			0x131
#define ADF702X_REG_RADIO_PROFILE_16			0x132
#define ADF702X_REG_RADIO_PROFILE_17			0x133
#define ADF702X_REG_RADIO_PROFILE_18			0x134
#define ADF702X_REG_RADIO_PROFILE_19			0x135
#define ADF702X_REG_RADIO_PROFILE_20			0x136
#define ADF702X_REG_RADIO_PROFILE_21			0x137
#define ADF702X_REG_RADIO_PROFILE_22			0x138

#define ADF702X_REG_TESTMODES				0x139
#define ADF702X_BIT_TESTMODES_EXT_PA_LNA_ATB_CONFIG		BIT(7)
#define ADF702X_BIT_TESTMODES_PER_IRQ_SELF_CLEAR		BIT(3)
#define ADF702X_BIT_TESTMODES_PER_ENABLE			BIT(2)
#define ADF702X_BIT_TESTMODES_CONTINUOUS_TX			BIT(1)
#define ADF702X_BIT_TESTMODES_CONTINUOUS_RX			BIT(0)

// ADF7023
#define ADF702X_REG_TRANSITION_CLOCK_DIV		0x13A
// Reserved						0x13B
// Reserved						0x13C
// Reserved						0x13D
#define ADF702X_REG_RX_SYNTH_LOCK_TIME			0x13E
#define ADF702X_REG_TX_SYNTH_LOCK_TIME			0x13F
// ADF7024
// Reserved						0x13A
// Reserved						0x13B
// Reserved						0x13C
// Reserved						0x13D
// Reserved						0x13E
// Reserved						0x13F

/* Auxiliary Registeres 0x307 - 0x3FD */
#define ADF702X_REG_PA_LEVEL_MCR			0x307 // ADF7023
#define ADF702X_REG_WUC_CONFIG_HIGH			0x30C
#define ADF702X_REG_WUC_CONFIG_LOW			0x30D
#define ADF702X_REG_WUC_VALUE_HIGH			0x30E
#define ADF702X_REG_WUC_VALUE_LOW			0x30F
#define ADF702X_REG_WUC_FLAG_RESET			0x310
#define ADF702X_REG_WUC_STATUS				0x311
#define ADF702X_REG_RSSI_READBACK			0x312
#define ADF702X_REG_MAX_AFC_RANGE			0x315 // ADF7023
#define ADF702X_REG_IMAGE_REJECT_CAL_CONFIG		0x319
#define ADF702X_REG_CHIP_SHUTDOWN			0x322 // ADF7023
#define ADF702X_REG_POWERDOWN_RX			0x324
#define ADF702X_REG_POWERDOWN_AUX			0x325
#define ADF702X_REG_ADC_READBACK_HIGH			0x327
#define ADF702X_REG_ADC_READBACK_LOW			0x328
#define ADF702X_REG_BATTERY_MONITOR_THRESHOLD_VOLTAGE	0x32D
#define ADF702X_REG_EXT_UC_CLK_DIVIDE			0x32E
#define ADF702X_REG_AGC_CLK_DIVIDE			0x32F // ADF7023
#define ADF702X_REG_INTERRUPT_SOURCE_0			0x336
#define ADF702X_REG_INTERRUPT_SOURCE_1			0x337
#define ADF702X_REG_CALIBRATION_CONTROL			0x338 // ADF7023
#define ADF702X_REG_CALIBRATION_STATUS			0x339
#define ADF702X_REG_RXBB_CAL_CALWRD_READBACK		0x345
#define ADF702X_REG_RXBB_CAL_CALWRD_OVERWRITE		0x346 // ADF7023
#define ADF702X_REG_RCOSC_CAL_READBACK_HIGH		0x34F
#define ADF702X_REG_RCOSC_CAL_READBACK_LOW		0x350
#define ADF702X_REG_ADC_CONFIG_LOW			0x359
#define ADF702X_REG_ADC_CONFIG_HIGH			0x35A // ADF7023
#define ADF702X_REG_AGC_OOK_CONTROL			0x35B // ADF7023
#define ADF702X_REG_AGC_CONFIG				0x35C // ADF7023
#define ADF702X_REG_AGC_MODE				0x35D
#define ADF702X_REG_AGC_LOW_THRESHOLD			0x35E // ADF7023
#define ADF702X_REG_AGC_HIGH_THRESHOLD			0x35F // ADF7023
#define ADF702X_REG_AGC_GAIN_STATUS			0x360
#define ADF702X_REG_AGC_ADC_WORD			0x361 // ADF7023
#define ADF702X_REG_FREQUENCY_ERROR_READBACK		0x372
#define ADF702X_REG_VCO_BAND_OVRW_VAL			0x3CB // ADF7023
#define ADF702X_REG_VCO_AMPL_OVRW_VAL			0x3CC // ADF7023
#define ADF702X_REG_VCO_OVRW_EN				0x3CD // ADF7023
#define ADF702X_REG_VCO_CAL_CFG				0x3D0 // ADF7023
#define ADF702X_REG_OSC_CONFIG				0x3D2
#define ADF702X_REG_VCO_BAND_READBACK			0x3DA // ADF7023
#define ADF702X_REG_VCO_AMPL_READBACK			0x3DB // ADF7023
#define ADF702X_REG_ANALOG_TEST_BUS			0x3F8 // ADF7023
#define ADF702X_REG_RSSI_TSTMUX_SEL			0x3F9
#define ADF702X_REG_GPIO_CONFIGURE			0x3FA
#define ADF702X_REG_TEST_DAC_GAIN			0x3FD // ADF7023


struct adf702x_conf_regs {
	uint8_t interrupt_mask0;			// 0x100
	uint8_t interrupt_mask1;			// 0x101
	uint8_t number_of_wakeups0;			// 0x102
	uint8_t number_of_wakeups1;			// 0x103
	uint8_t number_of_wakeups_irq_threshold0;	// 0x104
	uint8_t number_of_wakeups_irq_threshold1;	// 0x105
	uint8_t rx_dwell_time;				// 0x106
	uint8_t parmtime_divider;			// 0x107
	uint8_t swm_rssi_thresh;			// 0x108
	uint8_t channel_freq0;				// 0x109
	uint8_t channel_freq1;				// 0x10A
	uint8_t channel_freq2;				// 0x10B
	union {						// 0x10C
		uint8_t radio_cfg0;
		uint8_t radio_profile_0;
	};
	union {						// 0x10D
		uint8_t radio_cfg1;
		uint8_t radio_profile_1;
	};
	union {						// 0x10E
		uint8_t radio_cfg2;
		uint8_t radio_profile_2;
	};
	union {						// 0x10F
		uint8_t radio_cfg3;
		uint8_t radio_profile_3;
	};
	union {						// 0x110
		uint8_t radio_cfg4;
		uint8_t radio_profile_4;
	};
	union {						// 0x111
		uint8_t radio_cfg5;
		uint8_t radio_profile_5;
	};
	union {						// 0x112
		uint8_t radio_cfg6;
		uint8_t radio_profile_6;
	};
	union {						// 0x113
		uint8_t radio_cfg7;
		uint8_t radio_agc_mode;
	};
	union {						// 0x114
		uint8_t radio_cfg8;
		uint8_t radio_pa_ramp;
	};
	union {						// 0x115
		uint8_t radio_cfg9;
		uint8_t radio_profile_7;
	};
	union {						// 0x116
		uint8_t radio_cfg10;
		uint8_t radio_afc_mode;
	};
	union {						// 0x117
		uint8_t radio_cfg11;
		uint8_t radio_profile_8;
	};
	uint8_t image_reject_cal_phase;			// 0x118
	uint8_t image_reject_cal_amplitude;		// 0x119
	uint8_t mode_control;				// 0x11A
	uint8_t preamble_match;				// 0x11B
	uint8_t symbol_mode;				// 0x11C
	uint8_t preamble_len;				// 0x11D
	uint8_t crc_poly0;				// 0x11E
	uint8_t crc_poly1;				// 0x11F
	uint8_t sync_control;				// 0x120
	uint8_t sync_byte0;				// 0x121
	uint8_t sync_byte1;				// 0x122
	uint8_t sync_byte2;				// 0x123
	uint8_t tx_base_adr;				// 0x124
	uint8_t rx_base_adr;				// 0x125
	uint8_t packet_length_control;			// 0x126
	uint8_t packet_length_max;			// 0x127
	union {						// 0x128
		uint8_t static_reg_fix;
		uint8_t radio_profile_9;
	};
	union {						// 0x129
		uint8_t address_match_offset;
		uint8_t reserved0;
	};
	union {						// 0x12A
		uint8_t address_length;
		uint8_t reserved1;
	};
	union {						// 0x12B
		uint8_t address_filtering0;
		uint8_t radio_profile_10;
	};
	union {						// 0x12C
		uint8_t address_filtering1;
		uint8_t radio_profile_11;
	};
	union {						// 0x12D
		uint8_t address_filtering2;
		uint8_t radio_profile_12;
	};
	union {						// 0x12E
		uint8_t address_filtering3;
		uint8_t radio_pa_level;
	};
	union {						// 0x12F
		uint8_t address_filtering4;
		uint8_t radio_profile_13;
	};
	union {						// 0x130
		uint8_t address_filtering5;
		uint8_t radio_profile_14;
	};
	union {						// 0x131
		uint8_t address_filtering6;
		uint8_t radio_profile_15;
	};
	union {						// 0x132
		uint8_t address_filtering7;
		uint8_t radio_profile_16;
	};
	union {						// 0x133
		uint8_t address_filtering8;
		uint8_t radio_profile_17;
	};
	union {						// 0x134
		uint8_t address_filtering9;
		uint8_t radio_profile_18;
	};
	union {						// 0x135
		uint8_t address_filtering10;
		uint8_t radio_profile_19;
	};
	union {						// 0x136
		uint8_t address_filtering11;
		uint8_t radio_profile_20;
	};
	union {						// 0x137
		uint8_t address_filtering12;
		uint8_t radio_profile_21;
	};
	union {						// 0x138
		uint8_t rssi_wait_time;
		uint8_t radio_profile_22;
	};
	uint8_t testmodes;				// 0x139
	union {						// 0x13A
		uint8_t transition_clock_div;
		uint8_t reserved2;
	};
	uint8_t reserved3;				// 0x13B
	uint8_t reserved4;				// 0x13C
	uint8_t reserved5;				// 0x13D
	union {						// 0x13E
		uint8_t rx_synth_lock_time;
		uint8_t reserved6;
	};
	union {						// 0x13F
		uint8_t tx_synth_lock_time;
		uint8_t reserved7;
	};
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

#endif /* ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_REGS_H_ */
