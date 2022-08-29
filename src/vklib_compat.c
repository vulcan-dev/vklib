#include "vklib_compat.h"

#include <errno.h>
#include <string.h>

const char* get_error(const char* usr_msg, int err_code) {
    char sys_msg[64];
    char result[1024];

#ifdef _WIN32
    if (strerror_s(sys_msg, sizeof(sys_msg), err_code) != 0) {
        strncpy_s(sys_msg, sizeof(sys_msg), "Unknown error", sizeof "Unknown error");
        sys_msg[sizeof sys_msg - 1] = '\0';
    }
#else
    if (strerror_r(err_code, sys_msg, sizeof(sys_msg)) != 0) {
        strncpy(sys_msg, "Unknown error", sizeof "Unknown error");
        sys_msg[sizeof sys_msg - 1] = '\0';
    }
#endif

    snprintf(result, sizeof(result), "%s: %s", usr_msg, sys_msg);
    return result;
}