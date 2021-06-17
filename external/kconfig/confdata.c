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

const char *conf_get_autoconfig_name(void)
{
    return getenv("KCONFIG_AUTOCONFIG");
}

static char *conf_expand_value(const char *in)
{
	struct symbol *sym;
	const char *src;
	static char res_value[SYMBOL_MAXLENGTH];
	char *dst, name[SYMBOL_MAXLENGTH];

	res_value[0] = 0;
	dst = name;
	while ((src = strchr(in, '$'))) {
		strncat(res_value, in, src - in);
		src++;
		dst = name;
		while (isalnum(*src) || *src == '_')
			*dst++ = *src++;
		*dst = 0;
		sym = sym_lookup(name, 0);
		sym_calc_value(sym);
		strcat(res_value, sym_get_string_value(sym));
		in = src;
	}
	strcat(res_value, in);

	return res_value;
}

char* conf_get_default_confname(void)
{
	struct stat buf;
	static char fullname[PATH_MAX+1];
	char *env, *name;

	name = conf_expand_value(conf_defname);
	env = getenv(SRCTREE)
	if (env) {
		sprtinf(fullname, "%s/%s", env, name);
		if (!stat(fullname, &buf))
			return fullname;
	}
	return name;
}