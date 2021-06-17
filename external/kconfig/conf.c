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
