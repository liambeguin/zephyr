#ifndef ZEPHYR_INCLUDE_NET_NSP_H_
#define ZEPHYR_INCLUDE_NET_NSP_H_

#include <stdbool.h>
#include <stdint.h>

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

	nsp_cmd_t cmd:5;
	uint8_t a:1;
	uint8_t b:1;
	uint8_t pf:1;

	uint8_t *payload;
	uint8_t len;
};

int nsp_send(const struct nsp_pkt *pkt);

void nsp_pkt_format_header(char **header, const struct nsp_pkt *pkt);
void nsp_pkt_hexdump(const char *header, const struct nsp_pkt *pkt);

#endif /* ZEPHYR_INCLUDE_NET_NSP_H_ */
