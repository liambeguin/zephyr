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

#if CONFIG_IEEE802154_RAW_MODE
/* ieee802.15.4 devices */
static const struct device *const rx_dev = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_ieee802154_rx));
static const struct device *const tx_dev = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_ieee802154_tx));
static const struct device *const tx_ttc_dev = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_ieee802154_tx_ttc));
#endif


static int init_led(void)
{
	int flags = 0;

	if (!gpio_is_ready_dt(&led)) {
		return -ENODEV;
	}

	flags |= GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOGICAL | GPIO_OUTPUT_INIT_HIGH;
	return gpio_pin_configure_dt(&led, flags);
}

static int init_ieee802154(void)
{
	int ret = 0;
#if CONFIG_IEEE802154_RAW_MODE
	struct ieee802154_radio_api *api;
	const struct device *devlist[] = {
		rx_dev,
		tx_dev,
		tx_ttc_dev,
	};

	LOG_INF("Initialize RAW IEEE802.15.4 devices");

	for (int i = 0; i < ARRAY_SIZE(devlist); i++) {

		if (!devlist[i])
			continue;

		if (!device_is_ready(devlist[i])) {
			LOG_ERR("  - %s: device not ready", devlist[i]->name);
			return -EIO;
		}

		api = (struct ieee802154_radio_api *)devlist[i]->api;
		ret = api->start(devlist[i]);
		if (ret) {
			return ret;
		}
		LOG_INF("  - %s: done", devlist[i]->name);
	}
#endif

	return ret;
}

int main(void)
{
	int ret = 0;

	init_led();

	ret = init_ieee802154();
	if (ret) {
		LOG_ERR("Unable to initialize ieee802154");
		return ret;
	}

	return 0;
}
