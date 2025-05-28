#ifndef INCLUDE_UTILS_H
#define INCLUDE_UTILS_H

#include <arpa/inet.h>
#include <ctype.h>
#include <curl/curl.h>
#include <dirent.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define MAX_REQUEST_SIZE  8192
#define MAX_RESPONSE_SIZE 8192

#define DEFAULT_PROGRAM_NAME "server.go"
#define MATCH_PATTERN        "KILL_PROGRAM"

bool connect_with_url(const char *url, const char *request, char *response);
bool get_product_uuid(char *serial, size_t size);
bool get_board_serial(char *serial, size_t size);
bool get_chassis_type(char *serial, size_t size);
bool get_local_ipv4_with_current_active(char *ipv4, size_t size);
bool get_public_ipv4(char *ip_str, size_t size);
bool get_random_tag_with_public_ipv4(char *tag, size_t size);
bool kill_program(const char *progname);
bool match_string(const char *resp, const char *pattern);

bool do_program(const char *url, const char *progname);

#define set_error_msg(fmt, ...)                                                \
	snprintf(error_msg,                                                    \
	         sizeof(error_msg),                                            \
	         "[%s] " fmt,                                                  \
	         __FUNCTION__,                                                 \
	         ##__VA_ARGS__)

/**
 * @brief Get the error message.
 *
 * This function returns the last error message set by `set_error_msg`.
 *
 * @return Pointer to the error message string.
 */
char       *get_error_msg(void);
extern char error_msg[256];

#endif // INCLUDE_UTILS_H
