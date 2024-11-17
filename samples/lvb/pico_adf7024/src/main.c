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
static struct ieee802154_radio_api *rx_api;
static const struct device *const rx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_ieee802154_rx));

static struct ieee802154_radio_api *tx_api;
static const struct device *const tx_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_ieee802154_tx));


static int init_led(void)
{
	int flags = 0;

	if (!gpio_is_ready_dt(&led))
		return -ENODEV;

	flags |= GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOGICAL | GPIO_OUTPUT_INIT_HIGH;
	return gpio_pin_configure_dt(&led, flags);
}

static bool init_ieee802154(void)
{
	struct ieee802154_attr_value val;

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

static int cmd_adf702x_cw(const struct shell *sh, size_t argc, char **argv)
{
	if (argc < 2)
		return -EIO;

	if (!strcmp(argv[1], "start"))
		tx_api->continuous_carrier(tx_dev);
	else if (!strcmp(argv[1], "stop")) {
		tx_api->stop(tx_dev);
		tx_api->start(tx_dev);
	} else
		shell_error(sh, "unknown command: %s", argv[1]);

	return 0;
}

static int cmd_adf702x_status(const struct shell *sh, size_t argc, char **argv)
{
	struct ieee802154_attr_value val;

	rx_api->attr_get(rx_dev, 0, &val);
	tx_api->attr_get(tx_dev, 0, &val);

	return 0;
}

static int cmd_adf702x_get(const struct shell *sh, size_t argc, char **argv)
{
	struct ieee802154_attr_value val;

	val.phy_supported_channel_pages = strtol(argv[1], NULL, 0);
	LOG_INF("ADDR: 0x%x", val.phy_supported_channel_pages);

	tx_api->attr_get(tx_dev, 1, &val);
	rx_api->attr_get(rx_dev, 1, &val);

	return 0;
}

static int cmd_adf702x_dump(const struct shell *sh, size_t argc, char **argv)
{
	struct ieee802154_attr_value val;

	tx_api->attr_get(tx_dev, 2, &val);
	rx_api->attr_get(rx_dev, 2, &val);

	return 0;
}

static int cmd_adf702x_rx(const struct shell *sh, size_t argc, char **argv)
{
	struct ieee802154_attr_value val;

	rx_api->attr_get(rx_dev, 3, &val);

	return 0;
}

static int cmd_adf702x_tx(const struct shell *sh, size_t argc, char **argv)
{
#if CONFIG_IEEE802154_RAW_MODE
	struct net_pkt *pkt;
	struct net_buf *buf;
	int ret = 0;

	pkt = net_pkt_alloc_with_buffer(NULL, 100, AF_UNSPEC, 0, K_NO_WAIT);
	if (!pkt)
		LOG_ERR("Failed to allocate net_pkt");

	buf = net_buf_frag_last(pkt->buffer);

	int bytes_to_send = argc - 1;
	if (bytes_to_send > 100) {
		LOG_ERR("can't send that much data...");
		goto out;
	}

	for (int i = 0; i < bytes_to_send; i++) {
		net_pkt_write_u8(pkt, strtol(argv[1 + i], NULL, 16));
	}

	/* Transmit data through radio */
	ret = tx_api->tx(tx_dev, IEEE802154_TX_MODE_DIRECT, pkt, buf);
	if (ret) {
		LOG_ERR("Error transmit data: %d", ret);
	}

out:
	net_pkt_unref(pkt);
#else
	LOG_ERR("tx disabled, use iface");
#endif

	return 0;
}

#define TEST_PAYLOAD "TEST PAYLOAD"
static int cmd_adf702x_iface(const struct shell *sh, size_t argc, char **argv)
{
	int ret;
	struct net_pkt *tx_pkt;
	struct net_if *iface = net_if_get_first_by_type(
			&NET_L2_GET_NAME(DUMMY));

	/* zassert_not_null(net_if_l2(iface), "No L2 found"); */
	if (!net_if_l2(iface))
		LOG_ERR("No L2 found");

	/* zassert_not_null(net_if_l2(iface)->send, "No send() found"); */
	if (!net_if_l2(iface)->send)
		LOG_ERR("No send() found");

	tx_pkt = net_pkt_alloc_with_buffer(iface, 1,
			AF_UNSPEC, 0, K_NO_WAIT);
	/* zassert_not_null(tx_pkt, "Failed to allocate packet"); */
	if (!tx_pkt)
		LOG_ERR("failed to allocate packet");

	ret = net_pkt_write(tx_pkt, TEST_PAYLOAD, sizeof(TEST_PAYLOAD));
	/* zassert_equal(0, ret, "Failed to write payload"); */
	if (!ret)
		LOG_ERR("Failed to write payload");

	ret = net_send_data(tx_pkt);
	/* zassert_equal(0, ret, "Failed to process TX packet"); */
	if (!ret)
		LOG_ERR("Failed to process TX packet");
	/* zassert_equal(tx_pkt, test_data.tx_pkt, "TX packet did not reach L2"); */

	net_pkt_unref(tx_pkt);

#if 0
	struct net_if *iface = net_if_get_first_by_type(&NET_L2_GET_NAME(DUMMY));
	struct net_pkt *tx_pkt;
	int ret = 0;

	LOG_ERR("lvb in %p", iface);
	tx_pkt = net_pkt_alloc_with_buffer(iface, sizeof(TEST_PAYLOAD), AF_UNSPEC, 0, K_NO_WAIT);
	LOG_ERR("lvb in %p", tx_pkt);

	LOG_ERR("lvb before net_pkt_write");
	ret = net_pkt_write(tx_pkt, TEST_PAYLOAD, sizeof(TEST_PAYLOAD));
	LOG_ERR("lvb ret %d", ret);

	ret = net_send_data(tx_pkt);
	LOG_ERR("lvb ret %d", ret);
	net_pkt_unref(tx_pkt);
#endif

	return ret;
}


SHELL_STATIC_SUBCMD_SET_CREATE(
	adf702x_cmds,
	SHELL_CMD_ARG(cw, NULL, "control continuous carrier mode\n", cmd_adf702x_cw, 2, 0),
#if CONFIG_IEEE802154_RAW_MODE
	SHELL_CMD_ARG(get, NULL, "read register\n", cmd_adf702x_get, 2, 0),
	SHELL_CMD_ARG(dump, NULL, "dump register\n", cmd_adf702x_dump, 0, 0),
	SHELL_CMD_ARG(rx, NULL, "rx\n", cmd_adf702x_rx, 1, 0),
	SHELL_CMD_ARG(tx, NULL, "tx\n", cmd_adf702x_tx, 1, 20),
#else
	SHELL_CMD_ARG(status, NULL, "read status\n", cmd_adf702x_status, 0, 0),
	SHELL_CMD_ARG(iface, NULL, "iface\n", cmd_adf702x_iface, 0, 0),
#endif
	SHELL_SUBCMD_SET_END
);
SHELL_CMD_REGISTER(adf702x, &adf702x_cmds, "ADF702x commands", NULL);

int main(void)
{
	/* struct net_if *iface; */
	/* iface = net_if_get_by_index(1); */

	LOG_DBG("TEST");

	init_led();

#if CONFIG_IEEE802154_RAW_MODE

	/* Initialize ieee802154 device */
	if (init_ieee802154()) {
		LOG_ERR("Unable to initialize ieee802154");
		return 0;
	}
#endif

	return 0;
}
