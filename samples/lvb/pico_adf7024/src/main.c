/*
 * Copyright (c) 2024 Liam Beguin <liambeguin@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/net_buf.h>
#include <zephyr/net/ieee802154_radio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Misc devices */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* ieee802.15.4 devices */
static const struct device *const rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_ieee802154_rx));
static const struct device *const tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_ieee802154_tx));

static int init_led(void)
{
	int flags = 0;

	if (!gpio_is_ready_dt(&led)) {
		return -ENODEV;
	}

	flags |= GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOGICAL | GPIO_OUTPUT_INIT_HIGH;
	return gpio_pin_configure_dt(&led, flags);
}

static bool init_ieee802154(void)
{
	struct ieee802154_radio_api *tx_api, *rx_api;

	/* Initialize ieee802154 device */
	LOG_INF("Initialize ieee802.15.4 devices");

	if (!device_is_ready(rx_dev)) {
		LOG_ERR("IEEE 802.15.4 rx device not ready");
		return -EIO;
	}

	rx_api = (struct ieee802154_radio_api *)rx_dev->api;
	rx_api->start(rx_dev);

	// automatically set to RX mode
	rx_api->attr_get(rx_dev, 3, &val);

	if (!device_is_ready(tx_dev)) {
		LOG_ERR("IEEE 802.15.4 tx device not ready");
		return -EIO;
	}

	tx_api = (struct ieee802154_radio_api *)tx_dev->api;
	tx_api->start(tx_dev);

	return 0;
}

int main(void)
{
	int ret = 0;

	init_led();

#if CONFIG_IEEE802154_RAW_MODE
	ret = init_ieee802154();
	if (ret) {
		LOG_ERR("Unable to initialize ieee802154");
		return ret;
	}
#endif

	return 0;
}
