#pragma once

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n",\
        __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) fprintf(stderr,\
        "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__,\
        clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr,\
        "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__,\
        clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr,\
        "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) {\
    log_err(M, ##__VA_ARGS__); errno = 0; goto error; }

#define sentinel(M, ...) { log_err(M, ##__VA_ARGS__);\
    errno = 0; goto error; }

#define check_mem(A) check(A, "Out of memory")

#define check_debug(A, M, ...) if(!(A)) {\
    debug(M, ##__VA_ARGS__); errno = 0; goto error; }

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

#define expect(A, M, ...)\
    if (!(A)) {\
        printf("\033[91mFailed: ");\
        printf("[ERROR] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
        exit(1);\
    }\

#define TEST(F) \
    printf("[TEST] (%s:%d) %s ...\n", __FILE__, __LINE__, #F);\
    F();\
    printf("\x1b[32mPassed\033[0m\n");\

