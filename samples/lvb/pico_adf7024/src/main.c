/*
 * Copyright (c) 2024 Liam Beguin <liambeguin@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/net/buf.h>
#include <zephyr/net/ieee802154_radio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* ieee802.15.4 device */
static struct ieee802154_radio_api *radio_api;
static const struct device *const ieee802154_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_ieee802154));


static bool init_ieee802154(void)
{
	/* Initialize ieee802154 device */
	LOG_INF("Initialize ieee802.15.4");
	if (!device_is_ready(ieee802154_dev)) {
		LOG_ERR("IEEE 802.15.4 device not ready");
		return -EIO;
	}

	radio_api = (struct ieee802154_radio_api *)ieee802154_dev->api;

	radio_api->start(ieee802154_dev);

	return 0;
}

static int cmd_adf702x_cw(const struct shell *sh, size_t argc, char **argv)
{
	if (argc < 2)
		return -EIO;

	if (!strcmp(argv[1], "start"))
		radio_api->continuous_carrier(ieee802154_dev);
	else if (!strcmp(argv[1], "stop")) {
		radio_api->stop(ieee802154_dev);
		radio_api->start(ieee802154_dev);
	} else
		shell_error(sh, "unknown command: %s", argv[1]);

	return 0;
}

static int cmd_adf702x_status(const struct shell *sh, size_t argc, char **argv)
{
	struct ieee802154_attr_value val;
	radio_api->attr_get(ieee802154_dev, 0, &val);
	return 0;
}

static int cmd_adf702x_get(const struct shell *sh, size_t argc, char **argv)
{
	struct ieee802154_attr_value val;

	val.phy_supported_channel_pages = strtol(argv[1], NULL, 0);
	LOG_INF("ADDR: 0x%x", val.phy_supported_channel_pages);
	radio_api->attr_get(ieee802154_dev, 1, &val);

	return 0;
}

static int cmd_adf702x_dump(const struct shell *sh, size_t argc, char **argv)
{
	struct ieee802154_attr_value val;
	radio_api->attr_get(ieee802154_dev, 2, &val);
	return 0;
}

static int cmd_adf702x_tx(const struct shell *sh, size_t argc, char **argv)
{
	struct net_pkt *pkt;
	struct net_buf *buf;
	int ret = 0;

	pkt = net_pkt_alloc_with_buffer(NULL, 100, AF_UNSPEC, 0, K_NO_WAIT);
	if (!pkt)
		LOG_ERR("Failed to allocate net_pkt");

	buf = net_buf_frag_last(pkt->buffer);

	uint8_t data[] = "This is a test\0";
	net_pkt_write(pkt, data, strlen(data));
	net_pkt_write(pkt, data, strlen(data));
	net_pkt_write(pkt, data, strlen(data));
	net_pkt_write(pkt, data, strlen(data));
	net_pkt_write(pkt, data, strlen(data));
	net_pkt_write(pkt, data, strlen(data));
	net_pkt_write(pkt, data, strlen(data));
	net_pkt_write_u8(pkt, 0x18);

	/* Transmit data through radio */
	ret = radio_api->tx(ieee802154_dev, IEEE802154_TX_MODE_DIRECT, pkt, buf);
	if (ret) {
		LOG_ERR("Error transmit data: %d", ret);
	}

	net_pkt_unref(pkt);

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	adf702x_cmds,
	SHELL_CMD_ARG(cw, NULL, "control continuous carrier mode\n", cmd_adf702x_cw, 2, 0),
	SHELL_CMD_ARG(status, NULL, "read status\n", cmd_adf702x_status, 0, 0),
	SHELL_CMD_ARG(get, NULL, "read register\n", cmd_adf702x_get, 2, 0),
	SHELL_CMD_ARG(dump, NULL, "dump register\n", cmd_adf702x_dump, 0, 0),
	SHELL_CMD_ARG(tx, NULL, "tx\n", cmd_adf702x_tx, 1, 0),
	SHELL_SUBCMD_SET_END
);
SHELL_CMD_REGISTER(adf702x, &adf702x_cmds, "ADF702x commands", NULL);

int main(void)
{
	/* Initialize ieee802154 device */
	if (init_ieee802154()) {
		LOG_ERR("Unable to initialize ieee802154");
		return 0;
	}

	return 0;
}
