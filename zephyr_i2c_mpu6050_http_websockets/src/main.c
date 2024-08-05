/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/websocket.h>
#include <base64.h>
// #include <sha1.h>
#include <zephyr/net/tls_credentials.h>
#include <mbedtls/sha1.h>
// #include <zephyr.h>
#include <zephyr/net/net_if.h>
#include <zephyr/device.h>
#include "mpu6050_i2c.h"
#include <mbedtls/sha256.h>
// #include <mbedtls/sha1.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <errno.h>
#include <zephyr/shell/shell.h>
#include <zephyr/random/random.h>
#include <arpa/inet.h>
#include <netinet/in.h>
// #include <zephyr/net_linkaddr.h>
// #include <sys/printk.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000
#define STACK_SIZE    2048
#define PRIORITY      7
// #define WIFI_SSID     "Airtouch664"
// #define WIFI_PASS     "newpassword00110"
#define WIFI_SSID     "."
#define WIFI_PASS     "hope1234"
LOG_MODULE_REGISTER(wifi_demo, LOG_LEVEL_INF);

// LATEST SERVER

#define SERVER_PORT  8080
#define SERVER_ADDR4 "192.168.43.43"
// Latest SERVER

// LATEST SERVER

static const char lorem_ipsum[] =
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
	"turpis risus, pulvinar sit amet varius ac, pellentesque quis purus."
	" "
	"Nam consequat purus in lacinia fringilla. Morbi volutpat, tellus "
	"nec tempus dapibus, ante sem aliquam dui, eu feugiat libero diam "
	"at leo. Sed suscipit egestas orci in ultrices. Integer in elementum "
	"ligula, vel sollicitudin velit. Nullam sit amet eleifend libero. "
	"Proin sit amet consequat tellus, vel vulputate arcu. Curabitur quis "
	"lobortis lacus. Sed faucibus vestibulum enim vel elementum. Vivamus "
	"enim nunc, auctor in purus at, aliquet pulvinar eros. Cras dapibus "
	"nec quam laoreet sagittis. Quisque dictum ante odio, at imperdiet "
	"est convallis a. Morbi mattis ut orci vitae volutpat."
	"I ma good adpokapo sadf asdf asdf asf sadf asdf sadf asdf asdfa "
	"adf afsa fsda fsadf safsag asgfg fg dfgdfghfdhg df gfdsg dsf"
	"\n";

#define MAX_RECV_BUF_LEN (sizeof(lorem_ipsum) - 1)

const int ipsum_len = MAX_RECV_BUF_LEN;
static uint8_t recv_buf_ipv4[MAX_RECV_BUF_LEN + 20];

#define EXTRA_BUF_SPACE 30

static uint8_t temp_recv_buf_ipv4[MAX_RECV_BUF_LEN + EXTRA_BUF_SPACE];

static int setup_socket(sa_family_t family, const char *server, int port, int *sock,
			struct sockaddr *addr, socklen_t addr_len)
{
	const char *family_str = family == AF_INET ? "IPv4" : "IPv6";
	int ret = 0;

	memset(addr, 0, addr_len);

	if (family == AF_INET) {
		net_sin(addr)->sin_family = AF_INET;
		net_sin(addr)->sin_port = htons(port);
		inet_pton(family, server, &net_sin(addr)->sin_addr);
	} else {
		net_sin6(addr)->sin6_family = AF_INET6;
		net_sin6(addr)->sin6_port = htons(port);
		inet_pton(family, server, &net_sin6(addr)->sin6_addr);
	}

	*sock = socket(family, SOCK_STREAM, IPPROTO_TCP);

	if (*sock < 0) {
		LOG_ERR("Failed to create %s HTTP socket (%d)", family_str, -errno);
	}

	return ret;

fail:
	if (*sock >= 0) {
		close(*sock);
		*sock = -1;
	}

	return ret;
}

static int connect_socket(sa_family_t family, const char *server, int port, int *sock,
			  struct sockaddr *addr, socklen_t addr_len)
{
	int ret;

	ret = setup_socket(family, server, port, sock, addr, addr_len);
	if (ret < 0 || *sock < 0) {
		return -1;
	}

	ret = connect(*sock, addr, addr_len);
	if (ret < 0) {
		LOG_ERR("Cannot connect to %s remote (%d)", family == AF_INET ? "IPv4" : "IPv6",
			-errno);
		ret = -errno;
	}

	return ret;
}

static int connect_cb(int sock, struct http_request *req, void *user_data)
{
	LOG_INF("Websocket %d for %s connected.", sock, (char *)user_data);

	return 0;
}

static size_t how_much_to_send(size_t max_len)
{
	size_t amount;

	do {
		amount = sys_rand32_get() % max_len;
	} while (amount == 0U);

	return amount;
}

static ssize_t sendall_with_ws_api(int sock, const void *buf, size_t len)
{
	return websocket_send_msg(sock, buf, len, WEBSOCKET_OPCODE_DATA_TEXT, true, true,
				  SYS_FOREVER_MS);
}

static ssize_t sendall_with_bsd_api(int sock, const void *buf, size_t len)
{
	return send(sock, buf, len, 0);
}

static void recv_data_wso_api(int sock, size_t amount, uint8_t *buf, size_t buf_len,
			      const char *proto)
{
	uint64_t remaining = ULLONG_MAX;
	int total_read;
	uint32_t message_type;
	int ret, read_pos;

	read_pos = 0;
	total_read = 0;

	while (remaining > 0) {
		ret = websocket_recv_msg(sock, buf + read_pos, buf_len - read_pos, &message_type,
					 &remaining, 0);
		if (ret < 0) {
			if (ret == -EAGAIN) {
				k_sleep(K_MSEC(50));
				continue;
			}

			LOG_DBG("%s connection closed while "
				"waiting (%d/%d)",
				proto, ret, errno);
			break;
		}

		read_pos += ret;
		total_read += ret;
	}

	if (remaining != 0 || total_read != amount ||
	    /* Do not check the final \n at the end of the msg */
	    memcmp(lorem_ipsum, buf, amount - 1) != 0) {
		LOG_ERR("%s data recv failure %zd/%d bytes (remaining %" PRId64 ")", proto, amount,
			total_read, remaining);
		LOG_HEXDUMP_DBG(buf, total_read, "received ws buf");
		LOG_HEXDUMP_DBG(lorem_ipsum, total_read, "sent ws buf");
	} else {
		LOG_DBG("%s recv %d bytes", proto, total_read);
	}
}

static void recv_data_bsd_api(int sock, size_t amount, uint8_t *buf, size_t buf_len,
			      const char *proto)
{
	int remaining;
	int ret, read_pos;

	remaining = amount;
	read_pos = 0;

	while (remaining > 0) {
		ret = recv(sock, buf + read_pos, buf_len - read_pos, 0);
		if (ret <= 0) {
			if (errno == EAGAIN || errno == ETIMEDOUT) {
				k_sleep(K_MSEC(50));
				continue;
			}

			LOG_DBG("%s connection closed while "
				"waiting (%d/%d)",
				proto, ret, errno);
			break;
		}

		read_pos += ret;
		remaining -= ret;
	}

	if (remaining != 0 ||
	    /* Do not check the final \n at the end of the msg */
	    memcmp(lorem_ipsum, buf, amount - 1) != 0) {
		LOG_ERR("%s data recv failure %zd/%d bytes (remaining %d)", proto, amount, read_pos,
			remaining);
		LOG_HEXDUMP_DBG(buf, read_pos, "received bsd buf");
		LOG_HEXDUMP_DBG(lorem_ipsum, read_pos, "sent bsd buf");
	} else {
		LOG_DBG("%s recv %d bytes", proto, read_pos);
	}
}

static bool send_and_wait_msg(int sock, size_t amount, const char *proto, uint8_t *buf,
			      size_t buf_len)
{
	static int count;
	int ret;

	if (sock < 0) {
		return true;
	}

	/* Terminate the sent data with \n so that we can use the
	 *      websocketd --port=9001 cat
	 * command in server side.
	 */
	memcpy(buf, lorem_ipsum, amount);
	buf[amount] = '\n';

	/* Send every 2nd message using dedicated websocket API and generic
	 * BSD socket API. Real applications would not work like this but here
	 * we want to test both APIs. We also need to send the \n so add it
	 * here to amount variable.
	 */
	if (count % 2) {
		ret = sendall_with_ws_api(sock, buf, amount + 1);
	} else {
		ret = sendall_with_bsd_api(sock, buf, amount + 1);
	}

	if (ret <= 0) {
		if (ret < 0) {
			LOG_ERR("%s failed to send data using %s (%d)", proto,
				(count % 2) ? "ws API" : "socket API", ret);
		} else {
			LOG_DBG("%s connection closed", proto);
		}

		return false;
	} else {
		LOG_DBG("%s sent %d bytes", proto, ret);
	}

	if (count % 2) {
		recv_data_wso_api(sock, amount + 1, buf, buf_len, proto);
	} else {
		recv_data_bsd_api(sock, amount + 1, buf, buf_len, proto);
	}

	count++;

	return true;
}

// LATEST SERVER

const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

void start_server()
{
}

static struct net_if *iface;

static struct net_mgmt_event_callback wifi_cb;

static struct net_mgmt_event_callback wifi_conn_callback;

void wifi_connect()
{

	struct wifi_connect_req_params wifi_params = {
		.ssid = WIFI_SSID,
		.ssid_length = sizeof(WIFI_SSID) - 1,
		.psk = WIFI_PASS,
		.psk_length = sizeof(WIFI_PASS) - 1,
		.channel = WIFI_CHANNEL_ANY,
		.security = WIFI_SECURITY_TYPE_PSK,
	};

	int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, net_if_get_default(), &wifi_params,
			   sizeof(struct wifi_connect_req_params));
	if (ret) {
		printk("Wi-Fi connection failed\n");
	} else {
		printk("Connecting to Wi-Fi...\n");
	}
}

static void wifi_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event,
			       struct net_if *iface)
{
	switch (mgmt_event) {
	case NET_EVENT_WIFI_CONNECT_RESULT:
		printk("Wi-Fi Connected!\n");
		k_sleep(K_SECONDS(5)); // Wait for Wi-Fi connection to stabilize
		net_dhcpv4_start(iface);

		break;
	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		printk("Wi-Fi Disconnected\n");
		break;

	default:
		break;
	}
}

void blink_led0(void)
{

	while (1) {

		k_sleep(K_MSEC(500)); // Additional delay to avoid rapid toggling
	}
}

void blink_led1(void)
{

	while (1) {

		k_sleep(K_MSEC(500)); // Additional delay to avoid rapid toggling
	}
}

K_THREAD_STACK_DEFINE(stack0, STACK_SIZE);
K_THREAD_STACK_DEFINE(stack1, STACK_SIZE);
struct k_thread thread0;
struct k_thread thread1;
int main(void)
{

	// net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler, NET_EVENT_IPV4_ADDR_ADD);
	// net_mgmt_add_event_callback(&wifi_cb);
	net_mgmt_init_event_callback(&wifi_conn_callback, wifi_event_handler,
				     NET_EVENT_WIFI_CONNECT_RESULT |
					     NET_EVENT_WIFI_DISCONNECT_RESULT);

	net_mgmt_add_event_callback(&wifi_conn_callback);

	wifi_connect();

	k_sleep(K_SECONDS(5));
	if (!device_is_ready(i2c_dev)) {
		printk("I2C device not ready\n");
		return 0;
	} else {
		printk("I2C device is ready\n");
	}

	basic_configuration(i2c_dev);

	int16_t accel_x = read_accel_data(i2c_dev, ACCEL_XOUT_H);
	int16_t accel_y = read_accel_data(i2c_dev, ACCEL_YOUT_H);
	int16_t accel_z = read_accel_data(i2c_dev, ACCEL_ZOUT_H);
	printk("Accelerometer X: %d\n", accel_x);
	printk("Accelerometer Y: %d\n", accel_y);
	printk("Accelerometer Z: %d\n", accel_z);
	// k_sleep(K_SECONDS(10));

	start_server();
	k_sleep(K_SECONDS(15));

	// Latest Server

	const char *extra_headers[] = {// "Origin: http://foobar\r\n",
				       NULL};
	int sock4 = -1, sock6 = -1;
	int websock4 = -1, websock6 = -1;
	int32_t timeout = 3 * MSEC_PER_SEC;
	struct sockaddr_in6 addr6;
	struct sockaddr_in addr4;
	size_t amount;
	int ret;

	if (IS_ENABLED(CONFIG_NET_IPV4)) {
		(void)connect_socket(AF_INET, SERVER_ADDR4, SERVER_PORT, &sock4,
				     (struct sockaddr *)&addr4, sizeof(addr4));
	}

	if (sock4 < 0 && sock6 < 0) {
		LOG_ERR("Cannot create HTTP connection.");
		k_sleep(K_FOREVER);
	}

	if (sock4 >= 0 && IS_ENABLED(CONFIG_NET_IPV4)) {
		struct websocket_request req;

		memset(&req, 0, sizeof(req));

		req.host = SERVER_ADDR4;
		req.url = "/";
		req.optional_headers = extra_headers;
		req.cb = connect_cb;
		req.tmp_buf = temp_recv_buf_ipv4;
		req.tmp_buf_len = sizeof(temp_recv_buf_ipv4);

		websock4 = websocket_connect(sock4, &req, timeout, "IPv4");
		if (websock4 < 0) {
			LOG_ERR("Cannot connect to %s:%d", SERVER_ADDR4, SERVER_PORT);
			close(sock4);
		}
	}

	if (websock4 < 0 && websock6 < 0) {
		LOG_ERR("No IPv4 or IPv6 connectivity");
		k_sleep(K_FOREVER);
	}

	LOG_INF("Websocket IPv4 %d IPv6 %d", websock4, websock6);
	size_t len;
	char buffer[10];

	while (1) {
		// amount = how_much_to_send(ipsum_len);

		// custom code
		int16_t accel_x = read_accel_data(i2c_dev, ACCEL_XOUT_H);
		int16_t accel_y = read_accel_data(i2c_dev, ACCEL_YOUT_H);
		int16_t accel_z = read_accel_data(i2c_dev, ACCEL_ZOUT_H);

		len = sprintf(buffer, "%d", accel_x);


		if (len > 0 && len < (sizeof(buffer) - 1)) {
			buffer[len] = '\n';     // Add newline character
			len++;                  // Increase length to include newline
		}


		int length = strlen(lorem_ipsum) + 3 * (strlen(buffer)) + 1;
		char *result = (char *)malloc(length * sizeof(char));
		strcpy(result, lorem_ipsum);
		strcat(result, buffer);
		len = sprintf(buffer, "%d", accel_y);
		if (len > 0 && len < (sizeof(buffer) - 1)) {
			buffer[len] = '\n';     // Add newline character
			buffer[len + 1] = '\0'; // Null-terminate the string
			len++;                  // Increase length to include newline
		}
		strcat(result, buffer);
		len = sprintf(buffer, "%d", accel_z);

		strcat(result, buffer);

		memcpy(recv_buf_ipv4, result, strlen(result) + 4);

		recv_buf_ipv4[strlen(result)] = '\n';
		free(result);

		/* Send every 2nd message using dedicated websocket API and generic
		 * BSD socket API. Real applications would not work like this but here
		 * we want to test both APIs. We also need to send the \n so add it
		 * here to amount variable.
		 */
		ret = sendall_with_ws_api(websock4, recv_buf_ipv4, length + 4 + 1);

		// custom code
		// if (websock4 >= 0 && !send_and_wait_msg(websock4, amount, "IPv4", recv_buf_ipv4,
		// 					sizeof(recv_buf_ipv4))) {
		// 	break;
		// }

		// k_sleep(K_MSEC(10));
	}

	if (websock4 >= 0) {
		close(websock4);
	}

	if (sock4 >= 0) {
		close(sock4);
	}

	k_sleep(K_FOREVER);

	// Latest Server

	return 0;
}
