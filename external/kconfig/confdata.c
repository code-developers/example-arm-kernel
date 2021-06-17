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

static int conf_set_sym_val(struct symbol *sym, int def, int def_flags, char *p)
{
	char *p2;

	switch (sym->type) {
	case S_TRISTATE:
		if (p[0] == 'm') {
			sym->def[def].tri = mod;
			sym->flags |= def_flags;
			break;
		}
	case S_BOOLEAN:
		if (p[0] == 'y') {
			sym->def[def].tri = yes;
			sym->flags |= def_flags;
			break;
		}
		if (p[0] == 'n') {
			sym->def[def].tri = no;
			sym->flags |= def_flags;
			break;
		}
		conf_warning("symbol value '%s' invalid for %s", p, sym->name);
		break;
	case S_OTHER:
		if (*p != '"') {
			for (p2 = p; *p2 && !isspace(*p2); p2++)
				;
			sym->type = S_STRING;
			goto done;
		}
	case S_STRING:
		p2 = p;
		while (*p2++ != '\n');
		*p2 = '\0';
#if 0
		if (*p++ != '"')
			break;
		for (p2 = p; (p2 = strpbrk(p2, "\"\\")); p2++) {
			if (*p2 == '"') {
				*p2 = 0;
				break;
			}
			memmove(p2, p2 + 1, strlen(p2));
		}
		if (!p2) {
			conf_warning("invalid string found");
			return 1;
		}
#endif
	case S_INT:
	case S_HEX:
	done:
		if (sym_string_valid(sym, p)) {
			sym->def[def].val = strdup(p);
			sym->flags |= def_flags;
		} else {
			conf_warning("symbol value '%s' invalid for %s", p, sym->name);
			return 1;
		}
		break;
	default:
		;
	}
	return 0;
}