#include <zephyr/logging/log.h>
#include <zephyr/net/nsp.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(nsp_fwd, LOG_LEVEL_INF);

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);
ZBUS_MSG_SUBSCRIBER_DEFINE(nsp_fwd_msg_sub);
ZBUS_CHAN_ADD_OBS(nsp_in_chan, nsp_fwd_msg_sub, 10);


static void nsp_forwarder_task(void *ptr1, void *ptr2, void *ptr3)
{
        ARG_UNUSED(ptr1);
        ARG_UNUSED(ptr2);
        ARG_UNUSED(ptr3);

	const struct zbus_channel *chan;
	struct nsp_pkt rxpkt;

        while (!zbus_sub_wait_msg(&nsp_fwd_msg_sub, &chan, &rxpkt, K_FOREVER)) {
                if (chan != &nsp_in_chan)
			continue;

		if ((rxpkt.dst & CONFIG_NSP_BACKEND_LOCAL_TX_MASK) == rxpkt.dst)
			continue;

		LOG_INF("forwarding...");

		nsp_send(&rxpkt);
		// don't unref since the sending thread will do it!
        }
}

K_THREAD_DEFINE(nsp_forwarder_task_id, 800, nsp_forwarder_task, NULL, NULL, NULL, 10, 0, 0);
