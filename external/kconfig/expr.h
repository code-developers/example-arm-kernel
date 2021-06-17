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


}