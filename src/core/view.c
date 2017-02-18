/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#include "../include/ayahesa.h"

#include "assets.h" //TODO: for now

#include <dlfcn.h>

#define TOk_INCLUDE     "@include("
#define TOk_FUNCTION    "@function("
#define TOk_IF          "@if("
#define TOk_ENDIF       "@endif"
#define TOk_CLOSE       ')'

static void
replace_string(struct kore_buf *b, char *src, void *dst, size_t len)
{
    char        *key, *end, *tmp, *p;
    size_t      blen, off, off2, nlen, klen;

    off = 0;
    klen = strlen(src);
    for (;;) {
        blen = b->offset;
        nlen = blen + len;
        p = (char *)b->data;

        key = kore_mem_find(p + off, b->offset - off, src, klen);
        if (key == NULL)
            break;

        end = key + klen;
        off = key - p;
        off2 = ((char *)(b->data + b->offset) - end);

        tmp = kore_malloc(nlen);
        memcpy(tmp, p, off);
        if (dst != NULL)
            memcpy((tmp + off), dst, len);
        memcpy((tmp + off + len), end, off2);

        kore_free(b->data);
        b->data = (u_int8_t *)tmp;
        b->offset = off + len + off2;
        b->length = nlen;

        off = off + len;
    }
}

static char **
split_arguments(char *cmdline, size_t cmdlinesz, int *argc)
{
    char **argv = (char **)kore_calloc(16, sizeof(char *));
    *argc = 0;

    size_t i; char *next = cmdline;
    for (i=0; i<cmdlinesz && *argc < 16; ++i) {
        if (cmdline[i] == ',') {
            cmdline[i++] = '\0';

            /* Arguments */
            argv[(*argc)++] = next;
            next = cmdline + i;
        }
    }

    return argv;
}

static char *
include_asset(const char *name)
{
    char buffer[64];

    snprintf(buffer, 64, "asset_%s_html", name);
    puts(buffer);

    void *hndl = dlopen(NULL, RTLD_LAZY);
    if (!hndl) {
        puts("dlopen");
        return NULL;
    }

    char *asset = dlsym(hndl, buffer);
    if (!asset) {
        puts("dlsym");
        dlclose(hndl);
        return NULL;
    }

    dlclose(hndl);
    return asset;
}

static void
call_vfunc(int argc, char *argv[])
{
    typedef char *(*decl_vfunc)(int argc, char *argv[]);
    decl_vfunc vfunc;

    void *hndl = dlopen(NULL, RTLD_LAZY);
    if (!hndl) {
        puts("dlopen");
        return;
    }

    *(char **)(&vfunc) = dlsym(hndl, argv[0]);
    if (!vfunc) {
        puts("dlsym");
        dlclose(hndl);
        return;
    }

    vfunc(argc, argv);

    dlclose(hndl);
}

static char *
parser(struct kore_buf *buffer, char *token, size_t tokensz, size_t *argumentsz)
{
    char *include = kore_mem_find(buffer->data, buffer->length, token, tokensz);
    if (!include)
        return NULL;

    /* Find closing bracket */
    char *pos_end = strchr(include, TOk_CLOSE);
    if (!pos_end)
        return NULL;

    char *pos_start = include + tokensz;
    printf("pos_end %c\n", pos_end[0]);
    printf("pos_start %c\n", pos_start[0]);

    *argumentsz = pos_end - pos_start;
    printf("Argument sz %zu\n", *argumentsz);
    printf("Argument '%.*s'\n", (int)*argumentsz, pos_start);

    return pos_start;
}

static void
process_include(struct kore_buf *buffer)
{
    char            *argument;
    size_t          argumentsz;

    //for (;;) {
        argumentsz = 0;
        argument = parser(buffer, TOk_INCLUDE, sizeof(TOk_INCLUDE) - 1, &argumentsz);
        if (!argument || !argumentsz)
            return;

        argument[argumentsz] = '\0';
        char *asset = include_asset(argument);
        if (!asset)
            return;

        replace_string(buffer, "@include(body", asset, strlen(asset));//TODO: replace between offsets
    //}
}

char *foo_bar(int argc, char *argv[]);
char *
foo_bar(int argc, char *argv[])
{
    puts("JUPS");
    return "kaas";
}

static void
process_function(struct kore_buf *buffer)
{
    char            *argument;
    size_t          argumentsz;

    //for (;;) {
        argumentsz = 0;
        argument = parser(buffer, TOk_FUNCTION, sizeof(TOk_FUNCTION) - 1, &argumentsz);
        if (!argument || !argumentsz)
            return;

        argument[argumentsz] = '\0';
        printf("Execute: %s\n", argument);

        int argc;
        char **argv = split_arguments(argument, argumentsz, &argc);
        if (!argc)
            return;

        call_vfunc(argc, argv);

        kore_free(argv);

        // replace_string(buffer, "@function(get_server_name, 3, 5)", asset, strlen(asset));
    //}
}

/* Easy substitution */
static void
process_vars(struct kore_buf *buffer)
{
    kore_buf_replace_string(buffer, "@year", "2017", 4);
    kore_buf_replace_string(buffer, "@instance", "SRV1", 4);
    kore_buf_replace_string(buffer, "@date", "Sun Feb  5 00:48:22 CET 2017", 28);
    kore_buf_replace_string(buffer, "@name", "Ayahesa", 7);
    kore_buf_replace_string(buffer, "@user", "Eve", 3);
    kore_buf_replace_string(buffer, TOk_IF, "", 0);//TODO
    kore_buf_replace_string(buffer, TOk_ENDIF, "", 0);//TODO
}

int
view(struct http_request *request, const char *view)
{
    struct kore_buf     *buffer;
    size_t              length = 0;

    buffer = kore_buf_alloc(asset_len_test_html);
    kore_buf_append(buffer, asset_test_html, asset_len_test_html);

    /* Check for macros */
    char *escape = kore_mem_find(buffer->data, buffer->length, "@", 1);
    if (escape) {
        /* Parse view in order */
        process_include(buffer);
        process_vars(buffer);
        process_function(buffer);
    }

    /* Convert and respond */
    char *str = kore_buf_stringify(buffer, &length);
    http_response_header(request, "content-type", "text/html");
    http_response(request, 200, str, length);

    kore_buf_cleanup(buffer);

    return (KORE_RESULT_OK);
}
