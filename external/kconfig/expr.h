#ifndef EXPR_H
#define EXPR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

struct file {
    struct file *next;
    struct file *parent;
    const char *name;
    int lineno;
};

typedef enum tristate {
    no, mod, yes
} tristate;

enum expr_type {
	E_NONE, E_OR, E_AND, E_NOT, E_EQUAL, E_UNEQUAL, E_LIST, E_SYMBOL, E_RANGE
};

union expr_data {
    struct expr *expr;
    struct symbol *sym;
};

}