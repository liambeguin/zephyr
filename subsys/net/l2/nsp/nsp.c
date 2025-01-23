#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(nsp_l2, LOG_LEVEL_DBG);
#include <errno.h>

#include <zephyr/net/capture.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/net/net_linkaddr.h>
#include <zephyr/random/random.h>
#include <zephyr/net/ieee802154.h>

static inline int ieee802154_radio_start(struct net_if *iface)
{
	const struct ieee802154_radio_api *radio = net_if_get_device(iface)->api;

	if (!radio) {
		return -ENOENT;
	}

	return radio->start(net_if_get_device(iface));
}

static inline int ieee802154_radio_stop(struct net_if *iface)
{
	const struct ieee802154_radio_api *radio = net_if_get_device(iface)->api;

	if (!radio) {
		return -ENOENT;
	}

	return radio->stop(net_if_get_device(iface));
}

static inline int ieee802154_radio_tx(struct net_if *iface, enum ieee802154_tx_mode mode,
				      struct net_pkt *pkt, struct net_buf *buf)
{
	const struct ieee802154_radio_api *radio = net_if_get_device(iface)->api;

	if (!radio) {
		return -ENOENT;
	}

	return radio->tx(net_if_get_device(iface), mode, pkt, buf);
}

static enum net_verdict nsp_l2_recv(struct net_if *iface, struct net_pkt *pkt)
{
	/* The IEEE 802.15.4 stack assumes that drivers provide a single-fragment package. */
	__ASSERT_NO_MSG(pkt->buffer && pkt->buffer->frags == NULL);

	net_pkt_unref(pkt);
	return NET_OK;
}

static int nsp_l2_send(struct net_if *iface, struct net_pkt *pkt)
{
	int len;
	int ret;

	ret = ieee802154_radio_tx(iface, IEEE802154_TX_MODE_DIRECT, pkt, pkt->buffer);
	if (ret) {
		return ret;
	}

	len = pkt->buffer->len;
	net_pkt_unref(pkt);

	return len;
}

static int nsp_l2_enable(struct net_if *iface, bool state)
{
	struct ieee802154_context *ctx = net_if_l2_data(iface);

	NET_DBG("iface %p %s", iface, state ? "up" : "down");

	k_sem_take(&ctx->ctx_lock, K_FOREVER);

	if (ctx->channel == IEEE802154_NO_CHANNEL) {
		k_sem_give(&ctx->ctx_lock);
		return -ENETDOWN;
	}

	k_sem_give(&ctx->ctx_lock);

	if (state) {
		return ieee802154_radio_start(iface);
	}

	return ieee802154_radio_stop(iface);
}

static enum net_l2_flags nsp_l2_flags(struct net_if *iface)
{
	struct ieee802154_context *ctx = net_if_l2_data(iface);

	/* No need for locking as these flags are set once
	 * during L2 initialization and then never changed.
	 */
	return ctx->flags;
}

void nsp_l2_init(struct net_if *iface)
{
	struct ieee802154_context *ctx = net_if_l2_data(iface);

	NET_DBG("Initializing custom NSP stack on iface %p", iface);

	k_sem_init(&ctx->ctx_lock, 1, 1);
	k_sem_init(&ctx->ack_lock, 0, 1);
}

NET_L2_INIT(NSP_L2, nsp_l2_recv, nsp_l2_send, nsp_l2_enable, nsp_l2_flags);
