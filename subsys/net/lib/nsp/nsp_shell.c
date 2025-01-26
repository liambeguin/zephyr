#include <stdio.h>
#include <stdlib.h>
#include <zephyr/net/nsp.h>
#include <zephyr/net_buf.h>
#include <zephyr/shell/shell.h>
#include <zephyr/zbus/zbus.h>

#define NSP_ARGV_EP     (1)
#define NSP_ARGV_ADDR   (2)
#define NSP_ARGV_VAL    (3)

struct shell *shared_sh;

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);

static void nsp_shell_rx_cb(const struct zbus_channel *chan)
{
	const struct nsp_pkt *rxpkt = zbus_chan_const_msg(chan);
	char *pkthdr = NULL;

        if (chan != &nsp_in_chan)
		return;

	rxpkt = (struct nsp_pkt *)zbus_chan_const_msg(chan);
        if (rxpkt->dst != CONFIG_NSP_SHELL_SRC_ADDR)
		return;

	nsp_pkt_format_header(&pkthdr, rxpkt);

        if (!rxpkt->a) {
		shell_error(shared_sh, "%s: NACK", pkthdr);
		shell_hexdump(shared_sh, rxpkt->payload, rxpkt->len);
		free(pkthdr);
		return;
	}

	switch (rxpkt->cmd) {
	case PING:
		shell_print(shared_sh, "%s: %*s", pkthdr, rxpkt->len, rxpkt->payload);
		break;
	case INIT:
	case PEEK:
	case POKE:
	case TELEMETRY:
	default:
		shell_print(shared_sh, "%s: %*s", pkthdr, rxpkt->len, rxpkt->payload);
		shell_hexdump(shared_sh, rxpkt->payload, rxpkt->len);
		break;
	};

	free(pkthdr);
}

ZBUS_LISTENER_DEFINE(nsp_shell_rx, nsp_shell_rx_cb);
ZBUS_CHAN_ADD_OBS(nsp_in_chan, nsp_shell_rx, 1);


static int cmd_nsp_ping(const struct shell *sh, size_t argc, char **argv)
{
	struct nsp_pkt txpkt = {0};

	shared_sh = (struct shell *)sh;

	txpkt.src = CONFIG_NSP_SHELL_SRC_ADDR;
	txpkt.dst = (int)strtol(argv[NSP_ARGV_EP], NULL, 16);
	txpkt.cmd = PING;
	txpkt.pf = 1;

	return nsp_send(&txpkt);
}

static int cmd_nsp_init(const struct shell *sh, size_t argc, char **argv)
{
	shell_error(sh, "NSP init");

	return 0;
}

static int cmd_nsp_peek(const struct shell *sh, size_t argc, char **argv)
{
	shell_error(sh, "nsp peek");

	return 0;
}

static int cmd_nsp_poke(const struct shell *sh, size_t argc, char **argv)
{
	shell_error(sh, "nsp poke");

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	nsp_cmds,
	SHELL_CMD_ARG(ping, NULL, "nsp ping <ep>", cmd_nsp_ping, 2, 0),
	SHELL_CMD_ARG(init, NULL, "nsp init <ep> [addr]", cmd_nsp_init, 2, 1),
	SHELL_CMD_ARG(peek, NULL, "nsp peek <ep> <addr>", cmd_nsp_peek, 3, 0),
	SHELL_CMD_ARG(poke, NULL, "nsp poke <ep> <addr> <value>", cmd_nsp_poke, 4, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(nsp, &nsp_cmds, "Nano Satellite Protocol (NSP) commands", NULL);
