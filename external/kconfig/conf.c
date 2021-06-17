// includes
#include <locale.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/time.h>

#define LKC_DIRECT_LINK
#include "lkc.h"

static void conf(struct menu **menu);
static void check_conf(struct menu **menu);

enum input_mode {
	oldaskconfig,
	silentoldconfig,
	oldconfig,
	allnoconfig,
	allyesconfig,
	allmodconfig,
	alldefconfig,
	randconfig,
	defconfig,
	savedefconfig,
	listnewconfig,
	oldnoconfig,
} input_mode = oldaskconfig;

char *defconfig_file;

static int indent = 1;
static int valid_stdin = 1;
static int sync_kconfig;
static int conf_cnt;
static char line[128];
static struct menu *rootEntry;


static void print_help(struct menu *menu)
{
    struct gstr help = str_new();

    menu_get_ext_help(menu, &help);

    	printf("\n%s\n", str_get(&help));
        str_free(&help);
}

static void strip(char *str)
{
	char *p = str;
	int l;

	while ((isspace(*p)))
		p++;
	l = strlen(p);
	if (p != str)
		memmove(str, p, l + 1);
	if (!l)
		return;
	p = str + l - 1;
	while ((isspace(*p)))
		*p-- = 0;
}

static void check_stdin(void)
{
	if (!valid_stdin) {
		printf(_("aborted!\n\n"));
		printf(_("Console input/output is redirected. "));
		printf(_("Run 'make oldconfig' to update configuration.\n\n"));
		exit(1);
	}
}

static int conf_askvalue(struct symbol *sym, const char *def)
{
	enum symbol_type type = sym_get_type(sym);

	if (!sym_has_value(sym))
		printf(_("(NEW) "));
	
	line[0] = '\n';
	line[1] = 0;

	if (!sym_if_changable(sym)) {
		printf("%s\n", def);
		line[0] = '\n';
		line[1] = 0;
		return 0;
	}
}