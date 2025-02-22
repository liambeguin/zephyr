#include <stdio.h>
#include <stdlib.h>
#include <zephyr/net/nsp.h>
#include <zephyr/net_buf.h>
#include <zephyr/shell/shell.h>
#include <zephyr/zbus/zbus.h>

#define NSP_ARGV_EP     (1)
#define NSP_ARGV_ADDR   (2)
#define NSP_ARGV_LEN    (3)
#define NSP_ARGV_VAL    (3)

struct shell *shared_sh;

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);
ZBUS_MSG_SUBSCRIBER_DEFINE(nsp_shell_rx);
ZBUS_CHAN_ADD_OBS(nsp_in_chan, nsp_shell_rx, 3);

static void nsp_shell_rx_task(void *ptr1, void *ptr2, void *ptr3)
{
        ARG_UNUSED(ptr1);
        ARG_UNUSED(ptr2);
        ARG_UNUSED(ptr3);

	const struct zbus_channel *chan;
	struct nsp_pkt rxpkt;
	char *pkthdr = NULL;

        while (!zbus_sub_wait_msg(&nsp_shell_rx, &chan, &rxpkt, K_FOREVER)) {
                if (chan != &nsp_in_chan)
			continue;

		if (rxpkt.dst != CONFIG_NSP_SHELL_SRC_ADDR)
			continue;

		nsp_pkt_format_header(&pkthdr, &rxpkt);

		if (!rxpkt.a) {
			shell_error(shared_sh, "%s: NACK", pkthdr);
			shell_hexdump(shared_sh, rxpkt.buf->data, rxpkt.buf->len);
			goto clean;
		}

		switch (rxpkt.cmdid) {
		case PING:
			shell_print(shared_sh, "%s: %*s", pkthdr, rxpkt.buf->len, rxpkt.buf->data);
			break;
		case INIT:
		case PEEK:
		case POKE:
		case TELEMETRY:
		default:
			shell_print(shared_sh, "%s: %*s", pkthdr, rxpkt.buf->len, rxpkt.buf->data);
			shell_hexdump(shared_sh, rxpkt.buf->data, rxpkt.buf->len);
			break;
		};

clean:
		free(pkthdr);
		net_buf_unref(rxpkt.buf);
        }

}
K_THREAD_DEFINE(nsp_shell_rx_task_id, 600, nsp_shell_rx_task, NULL, NULL, NULL, 3, 0, 0);


static int cmd_nsp_ping(const struct shell *sh, size_t argc, char **argv)
{
	NSP_PKT_DEFINE(txpkt);

	shared_sh = (struct shell *)sh;

	txpkt.src = CONFIG_NSP_SHELL_SRC_ADDR;
	txpkt.dst = (int)strtol(argv[NSP_ARGV_EP], NULL, 16);
	txpkt.cmdid = PING;
	txpkt.pf = 1;

	return nsp_send(&txpkt);
}

static int cmd_nsp_init(const struct shell *sh, size_t argc, char **argv)
{
	NSP_PKT_DEFINE(txpkt);
	uint32_t addr = 0;

	shared_sh = (struct shell *)sh;

	txpkt.src = CONFIG_NSP_SHELL_SRC_ADDR;
	txpkt.dst = (int)strtol(argv[NSP_ARGV_EP], NULL, 16);
	txpkt.cmdid = INIT;
	txpkt.pf = 1;

	addr = strtol(argv[NSP_ARGV_ADDR], NULL, 16);
	net_buf_add_le32(txpkt.buf, addr);

	return nsp_send(&txpkt);
}

static int cmd_nsp_peek(const struct shell *sh, size_t argc, char **argv)
{
	NSP_PKT_DEFINE(txpkt);
	uint32_t addr = 0;
	uint16_t len = 0;

	shared_sh = (struct shell *)sh;

	txpkt.src = CONFIG_NSP_SHELL_SRC_ADDR;
	txpkt.dst = (int)strtol(argv[NSP_ARGV_EP], NULL, 16);
	txpkt.cmdid = PEEK;
	txpkt.pf = 1;

	addr = strtol(argv[NSP_ARGV_ADDR], NULL, 16);
	net_buf_add_le32(txpkt.buf, addr);

	len = strtol(argv[NSP_ARGV_LEN], NULL, 16);
	net_buf_add_le16(txpkt.buf, len);

	return nsp_send(&txpkt);
}

static int cmd_nsp_poke(const struct shell *sh, size_t argc, char **argv)
{
	NSP_PKT_DEFINE(txpkt);
	uint32_t addr = 0;
	uint32_t value = 0;

	shared_sh = (struct shell *)sh;

	txpkt.src = CONFIG_NSP_SHELL_SRC_ADDR;
	txpkt.dst = (int)strtol(argv[NSP_ARGV_EP], NULL, 16);
	txpkt.cmdid = POKE;
	txpkt.pf = 1;

	addr = strtol(argv[NSP_ARGV_ADDR], NULL, 16);
	net_buf_add_le32(txpkt.buf, addr);

	value = strtol(argv[NSP_ARGV_VAL], NULL, 16);
	net_buf_add_le32(txpkt.buf, value);

	return nsp_send(&txpkt);
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	nsp_cmds,
	SHELL_CMD_ARG(ping, NULL, "nsp ping <ep>", cmd_nsp_ping, 2, 0),
	SHELL_CMD_ARG(init, NULL, "nsp init <ep> [addr]", cmd_nsp_init, 2, 1),
	SHELL_CMD_ARG(peek, NULL, "nsp peek <ep> <addr> <len>", cmd_nsp_peek, 4, 0),
	SHELL_CMD_ARG(poke, NULL, "nsp poke <ep> <addr> <value>", cmd_nsp_poke, 4, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(nsp, &nsp_cmds, "Nano Satellite Protocol (NSP) commands", NULL);
