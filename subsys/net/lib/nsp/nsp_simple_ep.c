#include <zephyr/net/nsp.h>
#include <zephyr/zbus/zbus.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/nsp.h>

#define NSP_SIMPLE_EP_ADDR 0x20

LOG_MODULE_REGISTER(nsp_ep, LOG_LEVEL_DBG);

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);


void simple_ep_cb(const struct zbus_channel *chan)
{
	const struct nsp_pkt *rxpkt = zbus_chan_const_msg(chan);
	struct nsp_pkt txpkt = {0};

        if (chan != &nsp_in_chan)
		return;

	rxpkt = zbus_chan_const_msg(chan);
        if (rxpkt->dst != NSP_SIMPLE_EP_ADDR)
		return;

	/* setup pkt */
	txpkt.src = rxpkt->dst;
	txpkt.dst = rxpkt->src;
	txpkt.a = 0; // NACK
	txpkt.pf = 1; // preset to finish
	txpkt.cmd = rxpkt->cmd;

	switch (rxpkt->cmd) {
	case PING:
		char *name = "Hello From Zephyr";
		txpkt.a = 1; // ACK
		txpkt.payload = name;
		txpkt.len = strlen(name);
		break;
	case INIT:
	case PEEK:
	case POKE:
	case TELEMETRY:
	default:
		break;
	};

	// Only reply when PF is set on receive
	if (rxpkt->pf)
		nsp_send(&txpkt);
}


ZBUS_LISTENER_DEFINE(simple_ep, simple_ep_cb);
ZBUS_CHAN_ADD_OBS(nsp_in_chan, simple_ep, 3);
