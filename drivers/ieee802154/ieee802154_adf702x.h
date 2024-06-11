/* ieee802154_adf702x.h - Header definition for ADI adf702x */

/*
 * Copyright (c) 2024 HawkEye 360
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_H_
#define ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_H_

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

/* Build-time structure
 **********************
 */

struct adf702x_config {
	struct gpio_dt_spec irq_gpio;
	struct spi_dt_spec spi;
	const char *name;

	LOG_INSTANCE_PTR_DECLARE(log);
	uint8_t inst;
};

/* Runtime context structure
 ***************************
 */

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

	struct k_sem trx_isr_lock;
	struct k_sem trx_tx_sync;
};

#endif /* ZEPHYR_DRIVERS_IEEE802154_IEEE802154_ADF702X_H_ */
