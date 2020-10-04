
#include <logging/log.h>
LOG_MODULE_REGISTER(net_coap_client_sample, LOG_LEVEL_DBG);

#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <net/socket.h>
#include <net/net_mgmt.h>
#include <net/net_ip.h>
#include <net/udp.h>
#include <net/coap.h>

#include "net_private.h"
#include "coap_help.h"

#define PEER_PORT 5683
#define MAX_COAP_MSG_LEN 256

/* CoAP socket fd */
static int sock;

struct pollfd fds[1];
static int nfds;

#define BLOCK_WISE_TRANSFER_SIZE_GET 2048

static struct coap_block_context blk_ctx;

static const char * const path[] = { "ping", NULL };

static void wait(void)
{
	if (poll(fds, nfds, -1) < 0) {
		LOG_ERR("Error in poll:%d", errno);
	}
}

static void prepare_fds(void)
{
	fds[nfds].fd = sock;
	fds[nfds].events = POLLIN;
	nfds++;
}

int start_coap_client(void)
{
	int ret = 0;
	struct sockaddr_in6 addr6;

	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(PEER_PORT);
	addr6.sin6_scope_id = 0U;

	inet_pton(AF_INET6, CONFIG_NET_CONFIG_PEER_IPV6_ADDR,
		  &addr6.sin6_addr);

	sock = socket(addr6.sin6_family, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		LOG_ERR("Failed to create UDP socket %d", errno);
		return -errno;
	}

	ret = connect(sock, (struct sockaddr *)&addr6, sizeof(addr6));
	if (ret < 0) {
		LOG_ERR("Cannot connect to UDP remote : %d", errno);
		return -errno;
	}

	prepare_fds();

	return 0;
}

int process_coap_reply(uint8_t* data, uint32_t* len)
{
	struct coap_packet reply;
	int rcvd;
	int ret;

	wait();

	rcvd = recv(sock, data, MAX_COAP_MSG_LEN, MSG_DONTWAIT);
	if (rcvd == 0) {
		ret = -EIO;
		goto end;
	}

	if (rcvd < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			ret = 0;
		} else {
			ret = -errno;
		}

		goto end;
	}

	net_hexdump("Response", data, rcvd);

	ret = coap_packet_parse(&reply, data, rcvd, NULL, 0);
	if (ret < 0) {
		LOG_ERR("Invalid data received");
	}

end:
	k_free(data);

	return ret;
}

int send_coap_request(uint8_t* data, uint32_t len)
{
	struct coap_packet request;
	const char * const *p;
	uint8_t *p_data;
	int r;

	p_data = (uint8_t *)k_malloc(MAX_COAP_MSG_LEN);
	if (!p_data) {
		return -ENOMEM;
	}

        uint32_t coap_len = MAX_COAP_MSG_LEN;
        if(len < MAX_COAP_MSG_LEN) {
            coap_len = len;
        } 

	r = coap_packet_init(&request, p_data, MAX_COAP_MSG_LEN,
			     1, COAP_TYPE_CON, 8, coap_next_token(),
			     COAP_METHOD_POST, coap_next_id());
	if (r < 0) {
		LOG_ERR("Failed to init CoAP message");
		goto end;
	}

	for (p = path; p && *p; p++) {
		r = coap_packet_append_option(&request, COAP_OPTION_URI_PATH,
					      *p, strlen(*p));
		if (r < 0) {
			LOG_ERR("Unable add option to request");
			goto end;
		}
	}

	r = coap_packet_append_payload_marker(&request);
	if (r < 0) {
		LOG_ERR("Unable to append payload marker");
		goto end;
	}

	r = coap_packet_append_payload(&request, data, coap_len);
	if (r < 0) {
		LOG_ERR("Not able to append payload");
		goto end;
	}

	net_hexdump("Request", request.data, request.offset);

	r = send(sock, request.data, request.offset, 0);
	printk("Send return value: %d",r);

end:
	k_free(p_data);

	return 0;
}
