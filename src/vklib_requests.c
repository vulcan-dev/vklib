// todo: dynamic allocation for requests (completed for headers and body)
// todo: linux compatibility

#include "vklib_requests.h"
#include "vklib_compat.h"

#include <errno.h>
#include <stdio.h>

#ifdef _WIN32 // Windows
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#else // Linux
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define BUFFER_SIZE 10240

void win_cleanup() {
#ifdef _WIN32 // Windows
	WSACleanup();
#endif
}

const char* parse_domain_name(const char* url) {
	char* domain_name = (char*)malloc(strlen(url) + 1);
	strcpy(domain_name, url);

	if (strncmp(domain_name, "http://", 7) == 0) {
		domain_name += 7;
	} else if (strncmp(domain_name, "https://", 8) == 0) {
		domain_name += 8;
	}

	char* port_number = strchr(domain_name, ':');
	if (port_number != NULL) {
		*port_number = '\0';
	}

	char* path = strchr(domain_name, '/');
	if (path != NULL) {
		*path = '\0';
	}

	return domain_name;
}

const char* parse_path(const char* url) {
	char* path = (char*)malloc(strlen(url) + 1);
	strcpy(path, url);

	if (strncmp(path, "http://", 7) == 0) {
		path += 7;
	} else if (strncmp(path, "https://", 8) == 0) {
		path += 8;
	}

	char* domain_name = strchr(path, '/');
	if (domain_name != NULL) {
		return domain_name;
	} else {
		return "/";
	}
}

int get_port(const char* url) {
	char* port_number = (char*)malloc(strlen(url) + 1);
	strcpy(port_number, url);

	if (strncmp(port_number, "http://", 7) == 0) {
		port_number += 7;
	} else if (strncmp(port_number, "https://", 8) == 0) {
		port_number += 8;
	}

	char* port_separator = strchr(port_number, ':');
	if (port_separator != NULL) {
		port_number = port_separator + 1;
		char* path = strchr(port_number, '/');
		if (path != NULL) {
			*path = '\0';
		}
		return atoi(port_number);
	} else {
		return 80;
	}
}

int is_key_value(const char* line) {
	char* key = (char*)malloc(strlen(line) + 1);
	char* value = (char*)malloc(strlen(line) + 1);
	char* separator = ':';

	int separator_found = 0;
	int key_found = 0;
	for (int i = 0; i < strlen(line); i++) {
		char c = line[i];
		if (c == '{' || c == '}') {
			return 0;
		}

		if (c == ':') {
			key_found = 1;
		}
		
		if (key_found == 1) {
			value[i] = c;
			if (c == '\n') {
				break;
			}
		} else {
			key[i] = c;
		}
	}

	char* output = (char*)malloc(strlen(key) + strlen(value) + 1);
	strcpy(output, key);
	strcat(output, value);

	return 1;
}

/*
    * Makes a get request to the specified url.
    * @param url The url to make the request to.
	* @return The response body or nil and error message if unsuccessful.
*/
int request_get(lua_State* L) {
	const char* url = luaL_checkstring(L, 1);

	#ifdef _WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	#endif

	char* domain_name = parse_domain_name(url);
	char* path = parse_path(url);
	int port = get_port(url);

    char buffer[BUFFER_SIZE];
    char request[512];

    char request_template[] = "GET %s HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36\r\n\r\n";
    struct protoent *protoent;
	struct sockaddr_in serv_addr;
	struct hostent *hostent;
	int sockfd;
	int bytes_sent;
	int bytes_received;

	protoent = getprotobyname("tcp");
	if (protoent == NULL) {
		win_cleanup();
		lua_pushnil(L);
		lua_pushstring(L, "Could not get protocol information.");
		return 2;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
	if (sockfd < 0) {
		win_cleanup();
		lua_pushnil(L);
		lua_pushstring(L, "Could not create socket.");
		return 2;
	}

	hostent = gethostbyname(domain_name);
	if (hostent == NULL) {
		win_cleanup();
		lua_pushnil(L);
		lua_pushstring(L, "Could not get host information.");
		return 2;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = *(long*)hostent->h_addr;
	memset(&(serv_addr.sin_zero), '\0', 8);
	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) < 0) {
		win_cleanup();
		lua_pushnil(L);
		lua_pushstring(L, "Could not connect to host.");
		return 2;
	}

	sprintf(request, request_template, path, domain_name);
	bytes_sent = send(sockfd, request, strlen(request), 0);
	if (bytes_sent < 0) {
		win_cleanup();
		lua_pushnil(L);
		lua_pushstring(L, "Could not send request.");
		return 2;
	}

	bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (bytes_received < 0) {
		win_cleanup();
		lua_pushnil(L);
		lua_pushstring(L, "Could not receive response.");
		return 2;
	}

	win_cleanup();

	char* response_code = strtok(buffer, "\r\n");
	if (response_code == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "Could not get response code.");
		return 2;
	}

	char* headers = (char*)malloc(1);
	char* body = (char*)malloc(1);

	headers[0] = '\0';
	body[0] = '\0';

	char* line = strtok(NULL, "\r\n");
	int is_header = 1;
	int header_length = 0;
	int body_length = 0;
	do {
		if (line == NULL) {
			break;
		}

		if (is_key_value(line) == 0) {
			is_header = 0;
		}

		if (is_header) {
			header_length += strlen(line) + 3;
			if (header_length > strlen(headers)) {
				headers = (char*)realloc(headers, header_length);
			}
			
			strcat(headers, line);
			strcat(headers, "\r\n");
			headers[header_length - 1] = '\0';
		} else {
			body_length += strlen(line) + 3;
			if (body_length > strlen(body)) {
				body = (char*)realloc(body, body_length);
			}

			strcat(body, line);
			strcat(body, "\r\n");
			body[body_length - 1] = '\0';
		}

		line = strtok(NULL, "\r\n");
	} while (line != NULL);

	if (body == NULL) {
		lua_pushnil(L);
		lua_pushstring(L, "Could not get response body.");
		return 2;
	}

	lua_newtable(L);
	lua_pushstring(L, "code");
	lua_pushstring(L, response_code);
	lua_settable(L, -3);
	lua_pushstring(L, "body");
	lua_pushlstring(L, body, strlen(body));
	lua_settable(L, -3);
	lua_pushstring(L, "headers");
	lua_pushlstring(L, headers, strlen(headers));
	lua_settable(L, -3);

	free(headers);
	free(body);

	return 1;
}