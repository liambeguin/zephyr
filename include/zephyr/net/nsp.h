#ifndef ZEPHYR_INCLUDE_NET_NSP_H_
#define ZEPHYR_INCLUDE_NET_NSP_H_

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/net_buf.h>

#define NSP_BUFSIZE 240
#define NSP_HDRSIZE 3

typedef enum {
	PING,
	INIT,
	PEEK,
	POKE,
	TELEMETRY,
	CUSTOM_START,
} nsp_cmd_t;

struct nsp_pkt {
	uint8_t src;
	uint8_t dst;

	union {
		uint8_t cmd;
		struct {
			nsp_cmd_t cmdid: 5;
			uint8_t a: 1;
			uint8_t b: 1;
			uint8_t pf: 1;
		};
	};

	struct net_buf *buf;
};

NET_BUF_POOL_FIXED_DEFINE(nsp_pkt_pool, CONFIG_NSP_PACKET_COUNT, NSP_BUFSIZE +
		NSP_HDRSIZE, 0, NULL);


#define NSP_PKT_DEFINE(_name)                                                                      \
	struct net_buf *nsp_buf_##_name = net_buf_alloc(&nsp_pkt_pool, K_FOREVER);                 \
	struct nsp_pkt _name = {                                                                   \
		.buf = nsp_buf_##_name,                                                            \
	};

int nsp_send(const struct nsp_pkt *pkt);

void nsp_pkt_format_header(char **header, const struct nsp_pkt *pkt);
void nsp_pkt_hexdump(const char *header, const struct nsp_pkt *pkt);

#endif /* ZEPHYR_INCLUDE_NET_NSP_H_ */
