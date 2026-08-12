#ifndef UTILS_H
#define UTILS_H

#define nullchk(x) if (!(x)) { fprintf(stderr, "null check failed: %s at %s:%d\n", #x, __FILE__, __LINE__); return; }
#define nullchkr(x, r) if (!(x)) { fprintf(stderr, "null check failed: %s at %s:%d\n", #x, __FILE__, __LINE__); return r; }

typedef enum {
	ERROR,
	INFO,
	WARN,
} WarnLevel;

#define lprintf(x, str, ...) do {				 \
	 printlog(x, __func__, str, ##__VA_ARGS__);\
} while (0)								



#define BUF_SMALL 128
#define BUF_MID 256
#define BUF_LARGE 512

#include <cjson/cJSON.h>
#include <stdbool.h>

extern char *minecraft_path ;


int streq(const char *a, const char *b);
bool is_allowed_on_linux(cJSON *library);
char *read_file(char *path);
char *get_jar_path(char *libname);
char *get_asset_index(cJSON *json);
bool file_exists(char *file);
char *build_classpath(cJSON *version_json);
void list_installed(void);
void printlog(WarnLevel level, const char *affected, const char *msg, ...);
void mkdirs(const char *path);


#endif
