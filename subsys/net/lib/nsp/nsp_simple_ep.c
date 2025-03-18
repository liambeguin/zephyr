#include <app_version.h>
#include <zephyr/net/nsp.h>
#include <zephyr/zbus/zbus.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/nsp.h>

#define xstr(s) str(s)
#define str(s) #s

LOG_MODULE_REGISTER(nsp_ep, LOG_LEVEL_DBG);

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);
ZBUS_MSG_SUBSCRIBER_DEFINE(nsp_simple_ep);
ZBUS_CHAN_ADD_OBS(nsp_in_chan, nsp_simple_ep, 2);

static void nsp_simple_ep_task(void *ptr1, void *ptr2, void *ptr3)
{
        ARG_UNUSED(ptr1);
        ARG_UNUSED(ptr2);
        ARG_UNUSED(ptr3);

	const struct zbus_channel *chan;
	struct nsp_pkt rxpkt;
	NSP_PKT_DEFINE(txpkt);

        while (!zbus_sub_wait_msg(&nsp_simple_ep, &chan, &rxpkt, K_FOREVER)) {
                if (chan != &nsp_in_chan)
			continue;

		if (rxpkt.dst != CONFIG_NSP_SIMPLE_EP_ADDR)
			continue;

		/* setup pkt */
		net_buf_reset(txpkt.buf);
		txpkt.buf = net_buf_ref(txpkt.buf); // this is ours
		txpkt.src = rxpkt.dst;
		txpkt.dst = rxpkt.src;
		txpkt.a = 0; // NACK
		txpkt.pf = 1; // preset to finish
		txpkt.cmdid = rxpkt.cmdid;

		switch (rxpkt.cmdid) {
		case PING:
			char *name = "Zephyr " xstr(APP_BUILD_VERSION);
			txpkt.a = 1; // ACK
			net_buf_add_mem(txpkt.buf, name, strlen(name) + 1);
			break;
		case INIT:
		case PEEK:
		case POKE:
		case TELEMETRY:
		default:
			break;
		};

		// Only reply when PF is set on receive
		if (rxpkt.pf)
			nsp_send(&txpkt);

		net_buf_unref(rxpkt.buf); // unref borrowed reference
        }
}
K_THREAD_DEFINE(nsp_simple_ep_task_id, 800, nsp_simple_ep_task, NULL, NULL, NULL, 2, 0, 0);
