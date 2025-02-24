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
ZBUS_MSG_SUBSCRIBER_DEFINE(nsp_tx_msg_sub);
ZBUS_CHAN_ADD_OBS(nsp_out_chan, nsp_tx_msg_sub, 2);

static void nsp_sock_send_task(void *ptr1, void *ptr2, void *ptr3)
{
        ARG_UNUSED(ptr1);
        ARG_UNUSED(ptr2);
        ARG_UNUSED(ptr3);

	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(nsp_tx));
	struct net_if *iface = net_if_lookup_by_dev(dev);
	static uint8_t buffer[NSP_BUFSIZE] = {0};
	const struct zbus_channel *chan;
	struct nsp_pkt txpkt = {0};
	struct msghdr msg = {0};
	struct iovec io_vector;
	int len = 0;
	int fd;

	struct sockaddr_ll socket_sll = {
		.sll_ifindex = net_if_get_by_iface(iface),
		.sll_family = AF_PACKET,
	};

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0) {
		LOG_ERR("*** Failed to create RAW socket: %s", strerror(errno));
		return;
	}

	if (bind(fd, (const struct sockaddr *)&socket_sll, sizeof(struct sockaddr_ll))) {
		LOG_ERR("*** Failed to bind packet socket: %s", strerror(errno));
		goto release_fd;
	}

        while (!zbus_sub_wait_msg(&nsp_tx_msg_sub, &chan, &txpkt, K_FOREVER)) {
                if (chan != &nsp_out_chan)
			continue;

		// header
		net_buf_push_u8(txpkt.buf, txpkt.cmd);
		net_buf_push_u8(txpkt.buf, txpkt.dst);
		net_buf_push_u8(txpkt.buf, txpkt.src);

		len = txpkt.buf->len;
		memcpy(buffer, net_buf_pull_mem(txpkt.buf, len), len);

		io_vector.iov_base = buffer;
		io_vector.iov_len = len;
		msg.msg_iov = &io_vector;
		msg.msg_iovlen = 1;

		if (sendmsg(fd, &msg, 0) != len) {
			LOG_ERR("*** Failed to send: %s", strerror(errno));
			// TODO: do better
			goto release_fd;
		}

		// clear after send
		memset(buffer, 0, len);
		memset(&io_vector, 0, sizeof(io_vector));
		memset(&msg, 0, sizeof(msg));
		net_buf_unref(txpkt.buf);
        }

release_fd:
	close(fd);
}
K_THREAD_DEFINE(nsp_sock_send_task_id, 800, nsp_sock_send_task, NULL, NULL, NULL, 2, 0, 0);

static void nsp_sock_recv_task(void *ptr1, void *ptr2, void *ptr3)
{
        ARG_UNUSED(ptr1);
        ARG_UNUSED(ptr2);
        ARG_UNUSED(ptr3);

	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(nsp_rx));
	struct net_if *iface = net_if_lookup_by_dev(dev);
	static uint8_t buffer[NSP_BUFSIZE] = {0};
        int ret;
	int fd;

	struct sockaddr_in src_addr;
	socklen_t addr_len = sizeof(src_addr);
	struct msghdr msg = {0};
	struct iovec iov = {0};

	struct nsp_pkt rxpkt = {0};

	struct sockaddr_ll socket_sll = {
		.sll_ifindex = net_if_get_by_iface(iface),
		.sll_family = AF_PACKET,
	};

	fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (fd < 0) {
		LOG_ERR("*** Failed to create RAW socket: %s", strerror(errno));
		return;
	}

	if (bind(fd, (const struct sockaddr *)&socket_sll, sizeof(struct sockaddr_ll))) {
		LOG_ERR("*** Failed to bind packet socket: %s", strerror(errno));
		goto cleanup;
	}

        LOG_INF("NSP: starting recv thread for fd=%d", fd);

        while (1) {
                memset(&msg, 0, sizeof(msg));
                memset(&iov, 0, sizeof(iov));
                memset(buffer, 0, sizeof(buffer));

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

		rxpkt.buf = net_buf_alloc(&nsp_pkt_pool, K_FOREVER);

		if (ret < NSP_HDRSIZE) {
			LOG_ERR("Packet too short");
			continue;
		}

		rxpkt.src = buffer[0];
		rxpkt.dst = buffer[1];
		rxpkt.cmd = buffer[2];
		net_buf_add_mem(rxpkt.buf, &buffer[3], ret - NSP_HDRSIZE);

		ret = zbus_chan_pub(&nsp_in_chan, &rxpkt, K_MSEC(200));
		if (ret) {
			LOG_ERR("*** Failed to publish: %s (%d)", strerror(-ret), ret);
			/* TODO: do better here */
		}
	}

cleanup:
	close(fd);
}
K_THREAD_DEFINE(nsp_sock_recv_task_id, 800, nsp_sock_recv_task, NULL, NULL, NULL, 1, 0, 0);
// TODO:  update to a DT_FOREACH and pass devices as arg
/* DT_FOREACH_PROP_ELEM(DT_PATH(nsp), nsp_tx_interfaces,  NSP_SOCK_TX_INIT); */
