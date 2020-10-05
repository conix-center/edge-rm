
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


/* CoAP socket fd */
static int sock = NULL;
struct sockaddr_in6 addr6;

struct pollfd fds[1];
static int nfds = 0;
uint32_t token_int = 0;

#define BLOCK_WISE_TRANSFER_SIZE_GET 2048

static struct coap_block_context blk_ctx;

static const char * const path[] = { "ping", NULL };

static void wait(void)
{
	if (poll(fds, nfds, 10) < 0) {
		LOG_ERR("Error in poll:%d", errno);
	}
}

static void prepare_fds(void)
{
	fds[nfds].fd = sock;
	fds[nfds].events = POLLIN;
}

int create_coap_socket(void) {
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

	return 0;
}

int connect_coap_socket(void)
{
	int ret = 0;
	ret = connect(sock, (struct sockaddr *)&addr6, sizeof(addr6));
	if (ret < 0) {
		LOG_ERR("Cannot connect to UDP remote : %d", errno);
		return -errno;
	}

	prepare_fds();

	return 0;
}

int process_coap_reply(uint8_t* return_code, uint8_t* recv_payload, uint32_t* recv_len)
{
	struct coap_packet reply;
	uint8_t* data;
	int rcvd;
	int ret;

	wait();

	//allocate data for the entire packet
	data = (uint8_t *)k_malloc(MAX_COAP_MSG_LEN);
     	if (!data) {
     	    return -ENOMEM;
     	}

	//recv the payload
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

	//get the packet
	ret = coap_packet_parse(&reply, data, rcvd, NULL, 0);
	if (ret < 0) {
		LOG_ERR("Invalid data received");
		goto end;
	}

	//pull out return code
	*return_code = coap_header_get_code(&reply);

	//get the payload
	uint8_t* dummy = coap_packet_get_payload(&reply, recv_len);

	//copy the data into recv_payload buffer
	memcpy(recv_payload, dummy, *recv_len);

end:
	k_free(data);

	return ret;
}

int send_coap_request(uint8_t* data, uint32_t len)
{

	//If the socket isn't created then create it
	int ret;
	if(sock == NULL) {
		ret = create_coap_socket();
		if (ret < 0) {
			LOG_ERR("Error creating socket");
			return ret;
		}
	}

	//Create a connection using the socket
	ret = connect_coap_socket();
	if (ret < 0) {
		LOG_ERR("Error connecting to socket");
		return ret;
	}

	//Form and send the COAP packet
	struct coap_packet request;
	const char * const *p;
	uint8_t *p_data;
	int r;

	p_data = (uint8_t *)k_malloc(MAX_COAP_MSG_LEN);
	if (!p_data) {
		return -ENOMEM;
	}

	uint8_t* token = (uint8_t *)k_malloc(8);
	if (!token) {
		k_free(p_data);
		return -ENOMEM;
	}

	//generate a hex token
	snprintk(token, 8, "%08x",token_int);
	token_int++;

	r = coap_packet_init(&request, p_data, MAX_COAP_MSG_LEN,
			     1, COAP_TYPE_CON, 8, token,
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

	//append the octetstream content type option

	r = coap_packet_append_payload_marker(&request);
	if (r < 0) {
		LOG_ERR("Unable to append payload marker");
		goto end;
	}

	r = coap_packet_append_payload(&request, data, len);
	if (r < 0) {
		LOG_ERR("Not able to append payload");
		goto end;
	}

	net_hexdump("Request", request.data, request.offset);

	r = send(sock, request.data, request.offset, 0);
	if (r <= 0) {
		LOG_ERR("Error sending on socket: %d",r);
		goto end;
	}

end:
	k_free(p_data);
	k_free(token);

	return 0;
}
