#include <zephyr/net/nsp.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>


static void nsp_rx_print_callback(const struct zbus_channel *chan)
{
	const struct nsp_pkt *pkt = zbus_chan_const_msg(chan);

	nsp_pkt_hexdump("RX: ", pkt);
}

ZBUS_LISTENER_DEFINE(nsp_rx_listener, nsp_rx_print_callback);

ZBUS_CHAN_DEFINE(nsp_in_chan,
	struct nsp_pkt,
	NULL,
	NULL,
	ZBUS_OBSERVERS(nsp_rx_listener),
	ZBUS_MSG_INIT(
		.src = 0,
		.dst = 0,
		.a = 0,
		.b = 0,
		.pf = 0,
		.cmd = 0,
		.payload = NULL,
		.len = 0,
	)
);

static void nsp_tx_print_callback(const struct zbus_channel *chan)
{
	const struct nsp_pkt *pkt = zbus_chan_const_msg(chan);

	nsp_pkt_hexdump("TX: ", pkt);
}

ZBUS_LISTENER_DEFINE(nsp_tx_listener, nsp_tx_print_callback);
ZBUS_CHAN_DEFINE(nsp_out_chan,
	struct nsp_pkt,
	NULL,
	NULL,
	ZBUS_OBSERVERS(nsp_tx_listener),
	ZBUS_MSG_INIT(
		.src = 0,
		.dst = 0,
		.a = 0,
		.b = 0,
		.pf = 0,
		.cmd = 0,
		.payload = NULL,
		.len = 0,
	)
);
