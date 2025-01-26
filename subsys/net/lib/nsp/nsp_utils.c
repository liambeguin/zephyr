#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/nsp.h>

LOG_MODULE_REGISTER(nsp, LOG_LEVEL_DBG);

void nsp_pkt_hexdump(const char *header, const struct nsp_pkt *pkt)
{
	Z_LOG_HEXDUMP(LOG_LEVEL_WRN, pkt->payload, pkt->len,
			"%s{ 0x%02x->0x%02x %c%c%c 0x%02x (%4d)", header,
			pkt->src, pkt->dst,
			pkt->pf ? 'P' : '-',
			pkt->b  ? 'B' : '-',
			pkt->a  ? 'A' : '-',
			pkt->cmd, pkt->len);
}

void nsp_pkt_print(const char *header, struct nsp_pkt pkt, bool char_payload)
{
	char *pretty = NULL;
        size_t i = 0;

	 asprintf(&pretty, "%s{ 0x%02x->0x%02x %c%c%c 0x%02x (%4d)[", header, pkt.src, pkt.dst,
                pkt.pf ? 'P' : '-',
                pkt.b  ? 'B' : '-',
                pkt.a  ? 'A' : '-',
                pkt.cmd, pkt.len);

        if (pkt.len && char_payload)
                asprintf(&pretty, "%s %*s", pretty, (int)pkt.len, pkt.payload);
        else
                for (i = 0; i < pkt.len; i++)
                        asprintf(&pretty, "%s%s%02x", pretty, (i % 4 || i == 0) ? " " : "  ",
                               pkt.payload[i]);

        asprintf(&pretty, "%s ] }", pretty);

	LOG_WRN("%s", pretty);
	free(pretty);
}

