#include <stdio.h>
#include <stdlib.h>
#include <zephyr/logging/log.h>
#include <zephyr/net_buf.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_service.h>

#include <zephyr/net/nsp.h>

#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(nsp_sock, LOG_LEVEL_DBG);

ZBUS_CHAN_DECLARE(nsp_in_chan, nsp_out_chan);

#define PRIORITY  k_thread_priority_get(k_current_get())
#define STACKSIZE 1024

int nsp_socket_send(const struct device *dev, struct nsp_pkt *pkt)
{
	struct net_if *iface = net_if_lookup_by_dev(dev);
	struct msghdr msg = {0};
	struct iovec io_vector;
	int ret = 0;
	int fd;

	struct sockaddr_ll socket_sll = {
		.sll_ifindex = net_if_get_by_iface(iface),
		.sll_family = AF_PACKET,
	};

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0) {
		LOG_ERR("*** Failed to create RAW socket: %s", strerror(errno));
		ret = -errno;
		goto out;
	}

	if (bind(fd, (const struct sockaddr *)&socket_sll, sizeof(struct sockaddr_ll))) {
		LOG_ERR("*** Failed to bind packet socket: %s", strerror(errno));
		ret = -errno;
		goto release_fd;
	}

	io_vector.iov_base = pkt;
	io_vector.iov_len = pkt->len + 4;
	msg.msg_iov = &io_vector;
	msg.msg_iovlen = 1;

	if (sendmsg(fd, &msg, 0) != pkt->len + 4) {
		LOG_ERR("*** Failed to send: %s", strerror(errno));
		ret = -errno;
		goto release_fd;
	}

release_fd:
	close(fd);
out:
	return ret;
}

static k_tid_t rx_tid;
static K_THREAD_STACK_DEFINE(rx_stack, STACKSIZE);
static struct k_thread rx_data;

static void rx(int *rx_fd)
{
        int fd = POINTER_TO_INT(rx_fd);
	uint8_t buffer[248] = {0};

	struct sockaddr_in src_addr;
	socklen_t addr_len = sizeof(src_addr);

        int ret;

	struct msghdr msg = {0};
	struct iovec iov = {0};

	struct nsp_pkt rxpkt = {0};

        LOG_INF("NSP: starting recv thread for fd=%d", fd);

        while (1) {
                memset(&msg, 0, sizeof(msg));
                memset(&iov, 0, sizeof(iov));
                memset(&rxpkt, 0, sizeof(rxpkt));

		iov.iov_base = buffer;
		iov.iov_len = sizeof(buffer);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;

		ret = recvfrom(fd, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&src_addr, &addr_len);
		if (ret <= 0) {
			LOG_ERR("*** Failed to recv: %s (%d)", strerror(errno), errno);
			ret = -errno;
			/* TODO: do better here */
			break;
		}

		memcpy(&rxpkt, buffer, ret);
		ret = zbus_chan_pub(&nsp_in_chan, &rxpkt, K_SECONDS(1));
	}

	close(fd);
}

int nsp_transport_socket_register(const struct device *rxdev, const struct device *txdev)
{
	struct net_if *iface = net_if_lookup_by_dev(rxdev);
	int ret = 0;
	int fd;

	struct sockaddr_ll socket_sll = {
		.sll_ifindex = net_if_get_by_iface(iface),
		.sll_family = AF_PACKET,
	};

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0) {
		LOG_ERR("*** Failed to create RAW socket: %s", strerror(errno));
		ret = -errno;
		goto out;
	}

	if (bind(fd, (const struct sockaddr *)&socket_sll, sizeof(struct sockaddr_ll))) {
		LOG_ERR("*** Failed to bind packet socket: %s", strerror(errno));
		ret = -errno;
		goto cleanup;
	}

	rx_tid = k_thread_create(&rx_data, rx_stack,
			K_THREAD_STACK_SIZEOF(rx_stack),
			(k_thread_entry_t)rx,
			INT_TO_POINTER(fd),
			NULL,
			NULL, PRIORITY, 0, K_NO_WAIT);
	if (!rx_tid) {
		ret = -ENOENT;
		errno = -ret;
		LOG_ERR("*** Failed to create rx thread: %s", strerror(errno));
		goto cleanup;
	}

	return ret;

cleanup:
	close(fd);
out:
	return ret;
}
