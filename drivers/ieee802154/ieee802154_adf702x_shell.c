#include <stdio.h>
#include <stdlib.h>
#include <zephyr/net_buf.h>
#include <zephyr/net/ieee802154_radio.h>
#include <zephyr/shell/shell.h>

#include "ieee802154_adf702x.h"

#define ADF702X_ARGV_DEV      (1)
#define ADF702X_ARGV_CW_MODE  (2)
#define ADF702X_ARGV_TX_FIRST (2)
#define ADF702X_MAX_TXLEN     (100)

#define ADF702X_LIST_ENTRY(node_id)                                                                \
	{                                                                                          \
		.dev = DEVICE_DT_GET(node_id),                                                     \
	},

#define IS_ADF702X_NODE(node_id)                                                                   \
	COND_CODE_1(DT_NODE_HAS_COMPAT(node_id, adi_adf702x), (ADF702X_LIST_ENTRY(node_id)), ())

static struct adf702x_inst {
	const struct device *dev;
} adf702x_list[] = {DT_FOREACH_STATUS_OKAY_NODE(IS_ADF702X_NODE)};

static void get_adf702x_comp(size_t idx, struct shell_static_entry *entry)
{
	if (idx < ARRAY_SIZE(adf702x_list)) {
		entry->syntax = adf702x_list[idx].dev->name;
		entry->handler = NULL;
		entry->subcmd = NULL;
		entry->help = "ADF702x Device";
	} else {
		entry->syntax = NULL;
	}
}
SHELL_DYNAMIC_CMD_CREATE(dsub_adf702x, get_adf702x_comp);

static struct adf702x_inst *get_adf702x(const char *device_label)
{
	for (int i = 0; i < ARRAY_SIZE(adf702x_list); i++) {
		if (!strcmp(device_label, adf702x_list[i].dev->name)) {
			return &adf702x_list[i];
		}
	}

	/* This will never happen because was prompted by shell */
	__ASSERT_NO_MSG(false);

	return NULL;
}

static int cmd_adf702x_status(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	const struct ieee802154_radio_api *api = dev->api;
	struct ieee802154_attr_value val;

	return api->attr_get(dev, IEEE802154_ATTR_ADF702X_STATUS, &val);
}

static int cmd_adf702x_cw(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	const struct ieee802154_radio_api *api = dev->api;
	const char *mode = argv[ADF702X_ARGV_CW_MODE];

	if (argc < ADF702X_ARGV_CW_MODE) {
		return -EIO;
	}

	if (!strcmp(mode, "start")) {
		api->continuous_carrier(dev);
	} else if (!strcmp(mode, "stop")) {
		api->stop(dev);
		api->start(dev);
	} else {
		shell_error(sh, "unknown command: %s, use <start|stop>", mode);
	}

	return 0;
}

static int cmd_adf702x_dump(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	const struct ieee802154_radio_api *api = dev->api;
	struct ieee802154_attr_value val;

	return api->attr_get(dev, IEEE802154_ATTR_ADF702X_DUMP, &val);
}

static int cmd_adf702x_tx(const struct shell *sh, size_t argc, char **argv)
{
#if CONFIG_IEEE802154_RAW_MODE
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	const struct ieee802154_radio_api *api = dev->api;
	struct net_pkt *pkt;
	struct net_buf *buf;
	int bytes_to_send;
	int ret = 0;

	bytes_to_send = argc - ADF702X_ARGV_TX_FIRST;

	if (bytes_to_send < 1) {
		shell_error(sh, "No data to send: %d bytes...", bytes_to_send);
		return -EINVAL;
	}

	if (bytes_to_send > ADF702X_MAX_TXLEN) {
		shell_error(sh, "can't send more than %d bytes...", ADF702X_MAX_TXLEN);
		return -EINVAL;
	}

	pkt = net_pkt_alloc_with_buffer(NULL, ADF702X_MAX_TXLEN, AF_UNSPEC, 0, K_NO_WAIT);
	if (!pkt) {
		shell_error(sh, "Failed to allocate net_pkt");
	}

	buf = net_buf_frag_last(pkt->buffer);
	for (int i = 0; i < bytes_to_send; i++) {
		net_pkt_write_u8(pkt, (int)strtol(argv[ADF702X_ARGV_TX_FIRST + i], NULL, 16));
	}

	/* Transmit data through radio */
	ret = api->tx(dev, IEEE802154_TX_MODE_DIRECT, pkt, buf);
	if (ret) {
		shell_error(sh, "Error transmit data: %d", ret);
	}

	net_pkt_unref(pkt);

#else
	shell_warn(sh, "Raw TX disabled");
#endif

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	adf702x_cmds, SHELL_CMD_ARG(status, &dsub_adf702x, "read status", cmd_adf702x_status, 2, 0),
	SHELL_CMD_ARG(dump, &dsub_adf702x, "dump ADF702x registers", cmd_adf702x_dump, 2, 0),
	SHELL_CMD_ARG(cw, &dsub_adf702x, "control continuous carrier mode", cmd_adf702x_cw, 3, 0),
#if CONFIG_IEEE802154_RAW_MODE
	SHELL_CMD_ARG(tx, &dsub_adf702x, "Send raw data", cmd_adf702x_tx, 3, 250),
#endif
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(adf702x, &adf702x_cmds, "ADF702x debug commands", NULL);
