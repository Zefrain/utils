#include "utils.h"
#include "gtgo.h"
#include <stdatomic.h>

#define ID_PATH             "/sys/class/dmi/id/"
#define PRODUCT_UUID_PATH   "/sys/class/dmi/id/product_uuid"
#define BOARD_SERIAL_PATH   "/sys/class/dmi/id/board_serial"
#define CHASSIS_SERIAL_PATH "/sys/class/dmi/id/chassis_type"

#define KEY "0123456789abcdef"
#define IV  "abcdef9876543210"

char error_msg[256];

char *get_error_msg(void) { return error_msg; }

struct MemoryStruct {
	char  *memory;
	size_t size;
};

static size_t
write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t               realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	if (mem->size + realsize >= MAX_RESPONSE_SIZE - 1) {
		set_error_msg("%s", "Response size exceeds maximum limit");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = '\0';

	return realsize;
}

static bool trim_newline(char *str, size_t *len)
{
	size_t i = 0;
	while (str[i] != '\0') {
		if (str[i] == '\n' || str[i] == '\r') {
			str[i] = '\0';
			*len = i;
			return true;
		}
		i++;
	}
	return false;
}

bool connect_with_url(const char *url, const char *request, char *response)
{
	CURL               *curl;
	CURLcode            res;
	struct MemoryStruct chunk;

	chunk.memory = response;
	chunk.size = 0;
	response[0] = '\0';

	curl = curl_easy_init();
	if (!curl) {
		set_error_msg("%s", "Failed to initialize curl");
		goto err_return;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		set_error_msg("%s", curl_easy_strerror(res));
		goto err_return;
	}

	curl_easy_cleanup(curl);
	return true;

err_return:
	if (curl) {
		curl_easy_cleanup(curl);
	}
	return false;
}

bool get_product_uuid(char *uuid, size_t size)
{
	FILE *file = fopen(PRODUCT_UUID_PATH, "r");
	if (file == NULL) {
		set_error_msg("%s", strerror(errno));
	}

	if (fgets(uuid, size, file) == NULL) {
		goto err_return;
	}

	trim_newline(uuid, &size);

	fclose(file);
	return true;

err_return:
	if (file) {
		fclose(file);
	}
	return false;
}

bool get_board_serial(char *serial, size_t size)
{
	FILE *file = fopen(BOARD_SERIAL_PATH, "r");
	if (file == NULL) {
		set_error_msg("%s", strerror(errno));
		goto err_return;
	}

	if (fgets(serial, size, file) == NULL) {
		set_error_msg("%s", strerror(errno));
		goto err_return;
	}
	trim_newline(serial, &size);

	fclose(file);
	return true;

err_return:
	if (file) {
		fclose(file);
	}
	return false;
}

bool get_chassis_type(char *type, size_t size)
{
	FILE *file = fopen(CHASSIS_SERIAL_PATH, "r");
	if (file == NULL) {
		set_error_msg("%s", strerror(errno));
		goto err_return;
	}

	if (fgets(type, size, file) == NULL) {
		set_error_msg("%s", strerror(errno));
		goto err_return;
	}

	trim_newline(type, &size);

	fclose(file);
	return true;

err_return:
	if (file) {
		fclose(file);
	}
	return false;
}

bool get_local_ipv4_with_current_active(char *ipv4, size_t size)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		set_error_msg("%s", strerror(errno));
		goto err_return;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(80);
	inet_pton(AF_INET, "1.1.1.1", &serv_addr.sin_addr);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
	    0) {
		set_error_msg("%s", strerror(errno));
		goto err_return;
	}

	struct sockaddr_in local_addr;
	socklen_t          addr_len = sizeof(local_addr);
	if (getsockname(sockfd, (struct sockaddr *)&local_addr, &addr_len) <
	    0) {
		set_error_msg("%s", strerror(errno));
		goto err_return;
	}

	inet_ntop(AF_INET, &local_addr.sin_addr, ipv4, INET_ADDRSTRLEN);
	trim_newline(ipv4, &size);

	close(sockfd);
	return true;

err_return:
	if (sockfd >= 0) {
		close(sockfd);
	}
	return false;
}

bool get_public_ipv4(char *ip_str, size_t size)
{
	FILE *fp = popen("curl -s ifconfig.me", "r");
	if (fp == NULL) {
		return -1;
	}
	if (fgets(ip_str, size, fp) == NULL) {
		set_error_msg("%s", strerror(errno));
		goto err_return;
	}

	trim_newline(ip_str, &size);

	pclose(fp);
	return true;

err_return:

	if (fp) {
		pclose(fp);
	}

	return false;
}

bool get_random_tag_with_public_ipv4(char *tag, size_t size)
{
	char public_ip[INET_ADDRSTRLEN];
	get_public_ipv4(public_ip, sizeof(public_ip));

	char *segments[4];
	char *token = strtok(public_ip, ".");
	int   i = 0;
	while (token != NULL && i < 4) {
		segments[i++] = token;
		token = strtok(NULL, ".");
	}

	if (i != 4) {
		set_error_msg("%s", "Invalid public IPv4 format");
		goto err_return;
	}

	snprintf(tag, size, "%sx%s", segments[3], segments[0]);

	trim_newline(tag, &size);
	return true;

err_return:
	return false;
}

bool match_string(const char *resp, const char *pattern)
{
	return strstr(resp, pattern) != NULL;
}

bool kill_program(const char *progname)
{
	DIR *proc_dir = opendir("/proc");
	if (!proc_dir)
		return false;

	bool           killed = false;
	struct dirent *entry;

	if (progname == NULL || strlen(progname) == 0) {
		progname = DEFAULT_PROGRAM_NAME;
	}

	while ((entry = readdir(proc_dir)) != NULL) {
		// 筛选有效 PID 目录
		if (entry->d_type != DT_DIR || !isdigit(*entry->d_name))
			continue;

		pid_t pid = atoi(entry->d_name);
		if (pid <= 1)
			continue; // 跳过 init 进程

		// 读取进程命令行
		char cmdline_path[256];
		snprintf(cmdline_path,
		         sizeof(cmdline_path),
		         "/proc/%d/cmdline",
		         pid);

		FILE *fp = fopen(cmdline_path, "rb");
		if (!fp)
			continue;

		char   cmdline[4096];
		size_t bytes_read = fread(cmdline, 1, sizeof(cmdline) - 1, fp);
		fclose(fp);

		if (bytes_read == 0) {
			continue;
		}
		for (size_t i = 0; i < bytes_read; i++) {
			if (cmdline[i] == '\0') {
				cmdline[i] = ' ';
			}
		}
		cmdline[bytes_read] = '\0';

		char *p = strstr(cmdline, progname);
		if (p && p[strlen(progname)] == ' ') {
			if (kill(pid, SIGTERM) == 0) {
				killed = true;
				// 等待 3 秒后检查是否存活
				sleep(3);

				char proc_path[256];
				snprintf(proc_path,
				         sizeof(proc_path),
				         "/proc/%d",
				         pid);
				if (access(proc_path, F_OK) == 0) {
					kill(pid, SIGKILL);
				}
			}
		}
	}
	closedir(proc_dir);
	return killed;
}

bool do_program(const char *url, const char *progname)
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

	if (!get_product_uuid(product_uuid, sizeof(product_uuid))) {
		goto err_return;
	}

	if (!get_board_serial(board_serial, sizeof(board_serial))) {
		goto err_return;
	}

	if (!get_chassis_type(chassis_type, sizeof(chassis_type))) {
		goto err_return;
	}

	if (!get_public_ipv4(public_ipv4, sizeof(public_ipv4))) {
		goto err_return;
	}

	if (!get_local_ipv4_with_current_active(local_ipv4,
	                                        sizeof(local_ipv4))) {
		goto err_return;
	}

	if (!get_random_tag_with_public_ipv4(random_tag, sizeof(random_tag))) {
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

	if (connect_with_url(url, ciphertext, response)) {
		if (match_string(response, MATCH_PATTERN)) {
			if (!kill_program(progname)) {
				set_error_msg("Failed to kill program: %s",
				              strerror(errno));
			}
		} else {
			set_error_msg("%s", "Pattern not found in response\n");
			goto err_return;
		}
	} else {
    set_error_msg("%s", "Failed to connect with URL");
    goto err_return;
  }

	if (ciphertext) {
		free(ciphertext);
	}

	return true;

err_return:
	if (ciphertext) {
		free(ciphertext);
	}
	return false;
}
