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

#define BUFFER_SIZE 1024

/*-------------------------------------------------------------------------*\
* Structures
\*-------------------------------------------------------------------------*/
typedef struct t_header {
	char* key;
	char* value;
	struct t_header* next;
} t_header;

typedef struct t_header_arr {
    t_header* header;
} t_header_arr;

/*-------------------------------------------------------------------------*\
* Helpers
\*-------------------------------------------------------------------------*/
void win_cleanup() {
#ifdef _WIN32 // Windows
	WSACleanup();
#endif
}

t_header* get_header(const char* str) {
    t_header* header = malloc(sizeof(t_header));
    header->key = (char*)malloc(strlen(str) + 1);
    header->value = (char*)malloc(strlen(str) + 1);
    
    header->key[0] = '\0';
    header->value[0] = '\0';
    
    char seperator_found = 0;
    char key_found = 0;
    int len = strlen(str);
    
    for (int i = 0; i < len; i++) {
        char c = str[i];
        if (c == '{' || c == '}') {  // we don't want json for http headers
            return NULL;
        } else if (c == ':' && key_found == 0) { // key has been found
            key_found = 1;
            i = i + 1;
            continue;
        }
        
        if (key_found == 0) { // we want to find the key
            header->key[i] = c;
            header->key[i + 1] = '\0';
        } else {
            int idx = i - strlen(header->key) - 2;
            header->value[idx] = c;
            header->value[idx + 1] = '\0';
            
            if (c == '\n') {
                break;
            }
        }
    }
    
    if (header->key == NULL || header->value == NULL) {
		printf("Error: header key or value is null\n");
        return NULL;
	}

	// check if the key is valid
	if (header->key[0] == ' ' || header->key[0] == '\t' || header->key[0] == '\n'
		|| header->value[0] == ' ' || header->value[0] == '\t' || header->value[0] == '\n') {
		return NULL;
	}
    
    return header;
}

/*-------------------------------------------------------------------------*\
* Parsers (Domain, Path, Port)
\*-------------------------------------------------------------------------*/
const char* parse_domain_name(const char* url) {
	TIMER_START();
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

	TIMER_END();
	return domain_name;
}

const char* parse_path(const char* url) {
	TIMER_START();
	char* path = (char*)malloc(strlen(url) + 1);
	strcpy(path, url);

	if (strncmp(path, "http://", 7) == 0) {
		path += 7;
	} else if (strncmp(path, "https://", 8) == 0) {
		path += 8;
	}

	char* domain_name = strchr(path, '/');
	if (domain_name != NULL) {
		TIMER_END();
		return domain_name;
	} else {
		TIMER_END();
		return "/";
	}
}

int get_port(const char* url) {
	TIMER_START();
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
		TIMER_END();
		return atoi(port_number);
	} else {
		TIMER_END();
		return 80;
	}
}

/*
    * Makes a get request to the specified url.
    * @param url The url to make the request to.
	* @return The response body or nil and error message if unsuccessful.
*/
typedef struct t_response {
	char* body;
	int status_code;
	t_header_arr* headers;
} t_response;

t_response* threaded_get(const char* url) {

}

int request_get(lua_State* L) {
	TIMER_START();
	const char* url = luaL_checkstring(L, 1);

	#ifdef _WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	#endif

	char* domain_name = parse_domain_name(url);
	char* path = parse_path(url);
	int port = get_port(url);

    char* buffer = (char*)malloc(BUFFER_SIZE);
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

	int status = 0;
	int cur_size = 0;

	do {
		if (bytes_received >= cur_size) {
			char* tmp = (char*)realloc(buffer, cur_size + BUFFER_SIZE);
			if (tmp == NULL) {
				win_cleanup();
				lua_pushnil(L);
				lua_pushstring(L, "Could not allocate memory.");
				return 2;
			}
			buffer = tmp;
			cur_size += BUFFER_SIZE;
		}
		
		status = recv(sockfd, buffer + bytes_received, cur_size - bytes_received, 0);
		if (status < 0) {
			win_cleanup();
			lua_pushnil(L);
			lua_pushstring(L, "Could not receive response.");
			return 2;
		}
		bytes_received += status;
	} while (status > 0);

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

	t_header_arr* headers_array = (t_header_arr*)malloc(sizeof(t_header_arr) * 32);
	int headers_array_length = 0;

	do {
		if (line == NULL) {
			break;
		}

		t_header* header = get_header(line);
		if (header == NULL) {
			is_header = 0;
		} else {
			// convert key to lowercase
			char* key = header->key;
			int key_length = strlen(key);
			for (int i = 0; i < key_length; i++) {
				key[i] = tolower(key[i]);
			}

			headers_array[headers_array_length].header = malloc(sizeof(t_header));
			memcpy(headers_array[headers_array_length].header, header, sizeof(t_header));
			headers_array[headers_array_length].header->key = key;
			headers_array_length++;
		}
		free(header);

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
	lua_newtable(L);
	for (int i = 0; i < headers_array_length; i++) {
		lua_pushstring(L, headers_array[i].header->key);
		lua_pushstring(L, headers_array[i].header->value);
		lua_settable(L, -3);
	}
	lua_settable(L, -3);

	free(headers_array);
	free(headers);
	free(body);

	TIMER_END();

	return 1;
}