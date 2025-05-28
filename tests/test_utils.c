#include "utils.h"
#include "gtgo.h"

#define KEY "0123456789abcdef"
#define IV  "abcdef9876543210"

void test_get_local_ipv4_with_current_active()
{
	char ip_str[INET_ADDRSTRLEN];
	if (get_local_ipv4_with_current_active(ip_str, sizeof(ip_str)) == 0) {
		printf("Local IPv4: %s\n", ip_str);
	} else {
		printf("Failed to get local IPv4\n");
	}
}

void test_get_product_serial()
{
	char serial[256];
	if (get_product_uuid(serial, sizeof(serial)) == 0) {
		printf("Product Serial: %s\n", serial);
	} else {
		printf("Failed to get product serial\n");
	}
}

void test_get_board_serial()
{
	char serial[256];
	if (get_board_serial(serial, sizeof(serial)) == 0) {
		printf("Board Serial: %s\n", serial);
	} else {
		printf("Failed to get board serial\n");
	}
}

void test_get_chassis_serial()
{
	char serial[256];
	if (get_chassis_type(serial, sizeof(serial)) == 0) {
		printf("Chassis Serial: %s\n", serial);
	} else {
		printf("Failed to get chassis serial\n");
	}
}
void test_get_public_ipv4()
{
	char ipv4[INET_ADDRSTRLEN];
	if (get_public_ipv4(ipv4, sizeof(ipv4)) == 0) {
		printf("Public IPv4: %s\n", ipv4);
	} else {
		printf("Failed to get public IPv4\n");
	}
}
void test_get_random_tag_with_public_ipv4()
{
	char tag[256];
	if (get_random_tag_with_public_ipv4(tag, sizeof(tag)) == 0) {
		printf("Random Tag: %s\n", tag);
	} else {
		printf("Failed to get random tag\n");
	}
}

void test_kill_program()
{
	if (match_string("abc", "abc")) {
		const char *progname = "server.py";
		if (kill_program(progname) == 0) {
			printf("Program %s killed successfully\n", progname);
		} else {
			perror("Failed to kill program");
		}
	} else {
		printf("Strings do not match\n");
	}
}

void test_process()
{
	char  product_uuid[256];
	char  board_serial[256];
	char  chassis_type[256];
	char  public_ipv4[INET_ADDRSTRLEN];
	char  local_ipv4[INET_ADDRSTRLEN];
	char  random_tag[256];
	char  request[MAX_REQUEST_SIZE];
	char  response[MAX_RESPONSE_SIZE];
	char *ciphertext = NULL;

	if (get_product_uuid(product_uuid, sizeof(product_uuid)) == 0) {
		printf("Product Serial: %s\n", product_uuid);
	} else {
		printf("Failed to get product serial\n");
		goto err_return;
	}

	if (get_board_serial(board_serial, sizeof(board_serial)) == 0) {
		printf("Board Serial: %s\n", board_serial);
	} else {
		printf("Failed to get board serial\n");
		goto err_return;
	}

	if (get_chassis_type(chassis_type, sizeof(chassis_type)) == 0) {
		printf("Chassis Serial: %s\n", chassis_type);
	} else {
		printf("Failed to get chassis serial\n");
		goto err_return;
	}

	if (get_public_ipv4(public_ipv4, sizeof(public_ipv4)) == 0) {
		printf("Public IPv4: %s\n", public_ipv4);
	} else {
		printf("Failed to get public IPv4\n");
		goto err_return;
	}

	if (get_local_ipv4_with_current_active(local_ipv4,
	                                       sizeof(local_ipv4)) == 0) {
		printf("Local IPv4: %s\n", local_ipv4);
	} else {
		printf("Failed to get local IPv4\n");
		goto err_return;
	}

	if (get_random_tag_with_public_ipv4(random_tag, sizeof(random_tag)) ==
	    0) {
		printf("Random Tag: %s\n", random_tag);
	} else {
		printf("Failed to get random tag\n");
		goto err_return;
	}

	snprintf(request,
	         sizeof(request),
	         "product_uuid=%s&board_serial=%s&chassis_type=%s&"
	         "public_ipv4=%s&local_ipv4=%s&random_tag=%s",
	         product_uuid,
	         board_serial,
	         chassis_type,
	         public_ipv4,
	         local_ipv4,
	         random_tag);

	fatal_enc(request, &ciphertext, KEY);

	/* char b64_encoded[512]; */
	/* memset(b64_encoded, 0, sizeof(b64_encoded)); */
	/* base64_encode(ciphertext, b64_encoded, sizeof(b64_encoded)); */
	/* printf("Base64 Encoded: %s\n", b64_encoded); */

	if (connect_with_url(
		"http://localhost:8000/decrypt", ciphertext, response) == 0) {
		printf("Response: %s\n", response);
		if (match_string(response, MATCH_PATTERN)) {
			if (kill_program("server.go") == true) {
				printf("Program killed successfully\n");
			} else {
				perror("Failed to kill program");
			}
		}
	} else {
		printf("Pattern not found in response\n");
		goto err_return;
	}

	if (ciphertext) {
		free(ciphertext);
	}
	return;
err_return:
	if (ciphertext) {
		free(ciphertext);
	}
	printf("Exit with error occurs");
	return;
}

void test_do_program()
{
	const char *url = "http://localhost:8000/decrypt";
	const char *progname = "server.go";
	if (do_program(url, progname)) {
		printf("Program executed successfully\n");
	} else {
		printf("Error occured: %s", get_error_msg());
	}
  return;
}

int main()
{
	/* test_get_product_serial(); */
	/* test_get_board_serial(); */
	/* test_get_chassis_serial(); */
	/* test_get_public_ipv4(); */
	/* test_get_local_ipv4_with_current_active(); */
	/* test_get_random_tag_with_public_ipv4(); */
	/* test_conn_with_url(); */
	/* test_kill_program(); */

	/* test_process(); */

	test_do_program();

	return 0;
}
