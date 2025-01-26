#include <stdio.h>
#include <stdlib.h>
#include <zephyr/net/nsp.h>
#include <zephyr/net_buf.h>
#include <zephyr/shell/shell.h>
#include <zephyr/zbus/zbus.h>

#define NSP_ARGV_EP     (1)
#define NSP_ARGV_ADDR   (2)
#define NSP_ARGV_VAL    (3)

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);


static int cmd_nsp_ping(const struct shell *sh, size_t argc, char **argv)
{
	struct nsp_pkt pkt = {0};

	pkt.src = CONFIG_NSP_SHELL_SRC_ADDR;
	pkt.dst = (int)strtol(argv[NSP_ARGV_EP], NULL, 16);
	pkt.pf = 1;

	zbus_chan_pub(&nsp_out_chan, &pkt, K_NO_WAIT);

	return 0;
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
