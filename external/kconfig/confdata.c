// includes
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>

#define LKC_DIRECT_LINK
#include "lkc.h"

static void conf_warning(const char *fmt, ...)
        __attribute__ ((format (printf, 1, 2)));

static void conf_message(const char *fmt, ...)
        __attribute__ ((format (printf, 1, 2)));

static const char *conf_filename;
static int conf_lineno, conf_warning, conf_unsaved;

const char conf_defname[] = ".defconfig";

static void conf_warning(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "%s:%d:warning: ", conf_filename, conf_lineno);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	conf_warnings++;
}

static void conf_default_message_callback(const char *fmt, va_list ap)
{
	printf("#\n# ");
	vprintf(fmt, ap);
	printf("\n#\n");
}

static void (*conf_message_callback) (const char *fmt, va_list ap) =
        conf_default_message_callback;

void conf_set_message_callback(void (*fn) (const char *fmt, va_list ap))
{
    conf_message_callback = fn;
}

static void conf_message(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (conf_message_callback)
        conf_message_callback(fmt, ap);
}

const char *conf_get_configname(void)
{
    char *name = getenv("F9_CONFIG");

    return name ? name : ".config";
}