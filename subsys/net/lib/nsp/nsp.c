#include <stdlib.h>
#include <zephyr/net/nsp.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nsp, LOG_LEVEL_DBG);

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
	ZBUS_MSG_INIT(0)
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
	ZBUS_MSG_INIT(0)
);


void nsp_pkt_format_header(char **header, const struct nsp_pkt *pkt)
{
	 asprintf(header, "{ 0x%02x->0x%02x %c%c%c 0x%02x (%4d) }", pkt->src, pkt->dst,
                pkt->pf ? 'P' : '-',
                pkt->b  ? 'B' : '-',
                pkt->a  ? 'A' : '-',
                pkt->cmdid, pkt->buf->len);
}

void nsp_pkt_hexdump(const char *header, const struct nsp_pkt *pkt)
{
	char *pkthdr = NULL;

	nsp_pkt_format_header(&pkthdr, pkt);
	Z_LOG_HEXDUMP(LOG_LEVEL_WRN, pkt->buf->data, pkt->buf->len, "%s%s", header, pkthdr);
	free(pkthdr);
}

int nsp_send(const struct nsp_pkt *pkt)
{
	return zbus_chan_pub(&nsp_out_chan, pkt, K_NO_WAIT);
}
