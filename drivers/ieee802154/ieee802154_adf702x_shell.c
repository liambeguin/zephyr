#include <stdio.h>
#include <stdlib.h>
#include <zephyr/net_buf.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/ieee802154.h>
#include <zephyr/net/ieee802154_mgmt.h>
#include <zephyr/net/ieee802154_radio.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/shell/shell.h>
#include <zephyr/net/socket.h>

#include "ieee802154_adf702x.h"

#define ADF702X_ARGV_DEV      (1)
#define ADF702X_ARGV_CW_MODE  (2)
#define ADF702X_ARGV_TX_FIRST (2)
#define ADF702X_MAX_TXLEN     (100)

#define PRIORITY  k_thread_priority_get(k_current_get())
#define STACKSIZE 1024


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

static int cmd_adf702x_power(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	const struct ieee802154_radio_api *api = dev->api;
	struct ieee802154_attr_value val;

	if (argc < ADF702X_ARGV_CW_MODE) {
		return -EIO;
	}

	val.phy_supported_channel_pages = atoi(argv[ADF702X_ARGV_CW_MODE]);

	return api->attr_get(dev, IEEE802154_ATTR_ADF702X_POWER, &val);
}

static int cmd_adf702x_rssi(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	const struct ieee802154_radio_api *api = dev->api;
	struct ieee802154_attr_value val;

	api->attr_get(dev, IEEE802154_ATTR_ADF702X_RSSI, &val);

	return 0;
}

static int cmd_adf702x_dump(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	const struct ieee802154_radio_api *api = dev->api;
	struct ieee802154_attr_value val;

	return api->attr_get(dev, IEEE802154_ATTR_ADF702X_DUMP, &val);
}

#if CONFIG_IEEE802154_RAW_MODE
static int cmd_adf702x_start(const struct shell *sh, size_t argc, char **argv)
{
	shell_error(sh, "Not available in CONFIG_IEEE802154_RAW_MODE");
	return 0;
}

static int cmd_adf702x_stop(const struct shell *sh, size_t argc, char **argv)
{
	shell_error(sh, "Not available in CONFIG_IEEE802154_RAW_MODE");
	return 0;
}

static int cmd_adf702x_tx(const struct shell *sh, size_t argc, char **argv)
{
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
	return 0;
}

#else /* CONFIG_IEEE802154_RAW_MODE */
static int cmd_adf702x_start(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	struct net_if *iface = net_if_lookup_by_dev(dev);

	return net_if_up(iface);
}

static int cmd_adf702x_stop(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	struct net_if *iface = net_if_lookup_by_dev(dev);

	return net_if_down(iface);
}

#if CONFIG_IEEE802154_ADF702X_SHELL_RX_CMD
static k_tid_t rx_tid;
static K_THREAD_STACK_DEFINE(rx_stack, STACKSIZE);
static struct k_thread rx_data;

static void rx(int *rx_fd, const struct shell *sh)
{
        int fd = POINTER_TO_INT(rx_fd);
	uint8_t buffer[248] = {0};

	struct sockaddr_in src_addr;
	socklen_t addr_len = sizeof(src_addr);

        int ret;

	struct msghdr msg = {0};
	struct iovec iov;

        shell_info(sh, "[%d] Waiting for data...", fd);

        while (1) {
                memset(&msg, 0, sizeof(msg));
                memset(&iov, 0, sizeof(iov));

		iov.iov_base = buffer;
		iov.iov_len = sizeof(buffer);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;

		ret = recvfrom(fd, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&src_addr, &addr_len);
		if (ret < 0) {
			shell_error(sh, "*** Failed to recv: %s (%d)", strerror(errno), errno);
			ret = -errno;
			break;
		}

		shell_error(sh, "Got %d bytes", ret);
		shell_hexdump(sh, buffer, ret);
	}

	close(fd);
}

static int cmd_adf702x_rx(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	struct net_if *iface = net_if_lookup_by_dev(dev);
	int ret = 0;
	int fd;

	struct sockaddr_ll socket_sll = {
		.sll_ifindex = net_if_get_by_iface(iface),
		.sll_family = AF_PACKET,
	};

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0) {
		shell_error(sh, "*** Failed to create RAW socket: %s", strerror(errno));
		ret = -errno;
		goto out;
	}

	if (bind(fd, (const struct sockaddr *)&socket_sll, sizeof(struct sockaddr_ll))) {
		shell_error(sh, "*** Failed to bind packet socket: %s", strerror(errno));
		ret = -errno;
		goto cleanup;
	}

	rx_tid = k_thread_create(&rx_data, rx_stack,
			K_THREAD_STACK_SIZEOF(rx_stack),
			(k_thread_entry_t)rx,
			INT_TO_POINTER(fd),
			(void *)sh,
			NULL, PRIORITY, 0, K_NO_WAIT);
	if (!rx_tid) {
		ret = -ENOENT;
		errno = -ret;
		shell_error(sh, "*** Failed to create rx thread: %s", strerror(errno));
		goto cleanup;
	}

	return ret;

cleanup:
	close(fd);
out:
	return ret;
}
#endif /* CONFIG_IEEE802154_ADF702X_SHELL_RX_CMD */

static int cmd_adf702x_tx(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *dev = get_adf702x(argv[ADF702X_ARGV_DEV])->dev;
	struct net_if *iface = net_if_lookup_by_dev(dev);
	uint8_t raw_payload[64] = {0};
	struct msghdr msg = {0};
	struct iovec io_vector;
	int bytes_to_send;
	int ret = 0;
	int fd;

	struct sockaddr_ll socket_sll = {
		.sll_ifindex = net_if_get_by_iface(iface),
		.sll_family = AF_PACKET,
	};

	bytes_to_send = argc - ADF702X_ARGV_TX_FIRST;
	for (int i = 0; i < bytes_to_send; i++) {
		raw_payload[i] = (uint8_t)strtol(argv[ADF702X_ARGV_TX_FIRST + i], NULL, 16);
	}

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0) {
		shell_error(sh, "*** Failed to create RAW socket: %s", strerror(errno));
		ret = -errno;
		goto out;
	}

	if (bind(fd, (const struct sockaddr *)&socket_sll, sizeof(struct sockaddr_ll))) {
		shell_error(sh, "*** Failed to bind packet socket: %s", strerror(errno));
		ret = -errno;
		goto release_fd;
	}

	io_vector.iov_base = raw_payload;
	io_vector.iov_len = bytes_to_send;
	msg.msg_iov = &io_vector;
	msg.msg_iovlen = 1;

	if (sendmsg(fd, &msg, 0) != bytes_to_send) {
		shell_error(sh, "*** Failed to send: %s", strerror(errno));
		ret = -errno;
		goto release_fd;
	}

release_fd:
	close(fd);
out:
	return ret;
}
#endif /* CONFIG_IEEE802154_RAW_MODE */

SHELL_STATIC_SUBCMD_SET_CREATE(
	adf702x_cmds, SHELL_CMD_ARG(status, &dsub_adf702x, "read status", cmd_adf702x_status, 2, 0),
	SHELL_CMD_ARG(start, &dsub_adf702x, "start ADF702x iface", cmd_adf702x_start, 2, 0),
	SHELL_CMD_ARG(stop, &dsub_adf702x, "stop ADF702x iface", cmd_adf702x_stop, 2, 0),
	SHELL_CMD_ARG(power, &dsub_adf702x, "configure pa level", cmd_adf702x_power, 3, 0),
	SHELL_CMD_ARG(rssi, &dsub_adf702x, "read RSSI", cmd_adf702x_rssi, 2, 0),
	SHELL_CMD_ARG(dump, &dsub_adf702x, "dump ADF702x registers", cmd_adf702x_dump, 2, 0),
	SHELL_CMD_ARG(cw, &dsub_adf702x, "control continuous carrier mode", cmd_adf702x_cw, 3, 0),
#if CONFIG_IEEE802154_ADF702X_SHELL_RX_CMD
	SHELL_CMD_ARG(rx, &dsub_adf702x, "start rx thread", cmd_adf702x_rx, 2, 0),
#endif /* CONFIG_IEEE802154_ADF702X_SHELL_RX_CMD */
	SHELL_CMD_ARG(tx, &dsub_adf702x, "Send raw data", cmd_adf702x_tx, 3, 250),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(adf702x, &adf702x_cmds, "ADF702x debug commands", NULL);
