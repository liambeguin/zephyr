/* ieee802154_adf702x.h - Header definition for ADI adf702x */

/*
 * Copyright (c) 2024 HawkEye 360
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_H_
#define ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_H_

#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#include "ieee802154_adf702x_regs.h"

/* Status Word */
#define STATUS_SPI_READY		BIT(7)
#define STATUS_IRQ_STATUS		BIT(6)
#define STATUS_CMD_READY		BIT(5)
#define STATUS_FW_STATE			GENMASK(4, 0)

/* FW_STATE Description */
#define FW_STATE_INIT			0x0F
#define FW_STATE_BUSY			0x00
#define FW_STATE_PHY_OFF		0x11
#define FW_STATE_PHY_ON			0x12
#define FW_STATE_PHY_RX			0x13
#define FW_STATE_PHY_TX			0x14
#define FW_STATE_PHY_SLEEP		0x06
#define FW_STATE_GET_RSSI		0x05
#define FW_STATE_IR_CAL			0x07
#define FW_STATE_AES_DECRYPT_INIT	0x08
#define FW_STATE_AES_DECRYPT		0x09
#define FW_STATE_AES_ENCRYPT		0x0A

/* SPI Memory Access Commands */
#define SPI_MEM_WR			0x18 // Write data to packet RAM sequentially.
#define SPI_MEM_RD			0x38 // Read data from packet RAM sequentially.
#define SPI_MEMR_WR			0x08 // Write data to packet RAM nonsequentially.
#define SPI_MEMR_RD			0x28 // Read data from packet RAM nonsequentially.
#define SPI_NOP				0xFF // No operation.

/* Radio Controller Commands */
#define CMD_SYNC			0xA2 // This is an optional command. It is not necessary to use it during device initialization
#define CMD_PHY_OFF			0xB0 // Performs a transition of the device into the PHY_OFF state.
#define CMD_PHY_ON			0xB1 // Performs a transition of the device into the PHY_ON state.
#define CMD_PHY_RX			0xB2 // Performs a transition of the device into the PHY_RX state.
#define CMD_PHY_TX			0xB5 // Performs a transition of the device into the PHY_TX state.
#define CMD_PHY_SLEEP			0xBA // Performs a transition of the device into the PHY_SLEEP state.
#define CMD_CONFIG_DEV			0xBB // Configures the radio parameters based on the BBRAM values.
#define CMD_GET_RSSI			0xBC // Performs an RSSI measurement.
#define CMD_BB_CAL			0xBE // Performs a calibration of the IF filter.
#define CMD_HW_RESET			0xC8 // Performs a full hardware reset. The device enters the PHY_SLEEP state.
#define CMD_RAM_LOAD_INIT		0xBF // Prepares the program RAM for a firmware module download.
#define CMD_RAM_LOAD_DONE		0xC7 // Performs a reset of the communications processor after download of a firmware module to program RAM.
#define CMD_IR_CAL			0xBD // Initiates an image rejection calibration routine.
#define CMD_AES_ENCRYPT			0xD0 // Performs an AES encryption on the transmit payload data stored in packet RAM.
#define CMD_AES_DECRYPT			0xD2 // Performs an AES decryption on the received payload data stored in packet RAM.
#define CMD_AES_DECRYPT_INIT		0xD1 // Initializes the internal variables required for AES decryption.
#define CMD_RS_ENCODE_INIT		0xD1 // Initializes the internal variables required for the Reed Solomon encoding.
#define CMD_RS_ENCODE			0xD0 // Calculates and appends the Reed Solomon check bytes to the transmit payload data stored in packet RAM.
#define CMD_RS_DECODE			0xD2 // Performs a Reed Solomon error correction on the received payload data stored in packet RAM.


/* Custom IEEE802154 attributes */
#define IEEE802154_ATTR_ADF702X_STATUS  (IEEE802154_TX_MODE_PRIV_START + 1)
#define IEEE802154_ATTR_ADF702X_RAW_REG (IEEE802154_TX_MODE_PRIV_START + 2)
#define IEEE802154_ATTR_ADF702X_DUMP    (IEEE802154_TX_MODE_PRIV_START + 3)

/* Build-time structure
 **********************
 */

struct adf702x_config {
	struct gpio_dt_spec irq_gpio;
	struct spi_dt_spec spi;
	const char *name;

	uint32_t channel_frequency;
	uint8_t radio_profile;

	LOG_INSTANCE_PTR_DECLARE(log);
	uint8_t inst;
};

/* Runtime context structure
 ***************************
 */

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

enum adf7024_profile {
	PROFILE_A = 0,
	PROFILE_B = 1,
	PROFILE_C = 2,
	PROFILE_D = 3,
	PROFILE_E = 4,
	PROFILE_F = 5,
};

struct adf702x_context {
	const struct device *dev;
	struct net_if *iface;
	bool is_up;

	struct adf702x_conf_regs conf_regs;
	uint8_t status;

	struct gpio_callback irq_cb;

	struct k_thread trx_thread;
	K_KERNEL_STACK_MEMBER(trx_stack,
			      CONFIG_IEEE802154_ADF702X_RX_STACK_SIZE);

	/* PHY specific driver attributes */
	enum ieee802154_phy_channel_page cc_page;
	struct ieee802154_phy_channel_range cc_range;
	struct ieee802154_phy_supported_channels cc_channels;

	struct k_sem isr_lock;
	struct k_sem tx_wait;
};

#endif /* ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_H_ */
