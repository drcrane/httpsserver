#ifndef DBG_H_
#define DBG_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "leakdetector.h"

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, ...) if(!(A)) { log_err(__VA_ARGS__); errno=0; goto error; }

#define sentinel(...)  { log_err(__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "%s", "Out of memory.")

#define check_debug(A, ...) if(!(A)) { debug(__VA_ARGS__); errno=0; goto error; }

#endif /* DBG_H_ */