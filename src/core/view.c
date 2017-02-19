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

#include <dlfcn.h>

#define TOk_INCLUDE     "@include("
#define TOk_FUNCTION    "@function("
#define TOk_IF          "@if("
#define TOk_ENDIF       "@endif"
#define TOk_CLOSE       ')'

static void
replace_string(struct kore_buf *b, char *pos_start, size_t pos_length,void *dst, size_t len)
{
    char        *pos_end, *tmp;
    size_t      pre_len, post_len, new_len;

    new_len = b->offset + len;
    pos_end = pos_start + pos_length;
    pre_len = pos_start - (char *)b->data;
    post_len = ((char *)(b->data + b->offset) - pos_end);

    tmp = kore_malloc(new_len);
    memcpy(tmp, b->data, pre_len);
    if (dst != NULL)
        memcpy((tmp + pre_len), dst, len);
    memcpy((tmp + pre_len + len), pos_end, post_len);

    kore_free(b->data);
    b->data = (u_int8_t *)tmp;
    b->offset = pre_len + len + post_len;
    b->length = new_len;
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
            argv[(*argc)++] = next;//TODO: skip whitespace
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

static char *
call_vfunc(int argc, char *argv[])
{
    typedef char *(*decl_vfunc)(int argc, char *argv[]);
    decl_vfunc vfunc;

    void *hndl = dlopen(NULL, RTLD_LAZY);
    if (!hndl) {
        puts("dlopen");
        return NULL;
    }

    *(char **)(&vfunc) = dlsym(hndl, argv[0]);
    if (!vfunc) {
        puts("dlsym");
        dlclose(hndl);
        return NULL;
    }

    char *response = vfunc(argc, argv);

    dlclose(hndl);
    return response;
}

static char *
parser(struct kore_buf *buffer, char *token, size_t tokensz, size_t *argumentsz, char **posx_start, size_t *posx_length)
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

    *posx_start = include;
    printf("Start char: %c\n", (*posx_start)[0]);
    *posx_length = (pos_start + *argumentsz) - *posx_start;
    printf("Length: %zu\n", *posx_length );
    printf("End char: %c\n", (*posx_start + *posx_length)[0]);

    return pos_start;
}

static void
process_include(struct kore_buf *buffer)
{
    char            *argument;
    char            *pos_start;
    size_t          argumentsz;
    size_t          pos_length;

    /* Keep processing includes */
    for (;;) {
        argumentsz = 0;
        argument = parser(buffer, TOk_INCLUDE, sizeof(TOk_INCLUDE) - 1, &argumentsz, &pos_start, &pos_length);
        if (!argument || !argumentsz)
            return;

        argument[argumentsz] = '\0';
        char *asset = include_asset(argument);
        if (!asset)
            return;

        /* Replace substring */
        replace_string(buffer, pos_start, pos_length, asset, strlen(asset));
    }
}

char *foo_bar(int argc, char *argv[]);
char *
foo_bar(int argc, char *argv[])
{
    puts("JUPS");
    return "This is foo <b>bar</b>";
}

static void
process_function(struct kore_buf *buffer)
{
    char            *argument;
    char            *pos_start;
    size_t          argumentsz;
    size_t          pos_length;

    //for (;;) {
        argumentsz = 0;
        argument = parser(buffer, TOk_FUNCTION, sizeof(TOk_FUNCTION) - 1, &argumentsz, &pos_start, &pos_length);
        if (!argument || !argumentsz)
            return;

        argument[argumentsz] = '\0';
        printf("Execute: %s\n", argument);

        int argc;
        char **argv = split_arguments(argument, argumentsz, &argc);
        if (!argc)
            return;

        char *str = call_vfunc(argc, argv);

        kore_free(argv);

        replace_string(buffer, pos_start, pos_length, str, strlen(str));
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

    /* Retrieve base view */
    char *baseview = include_asset(view);
    if (!baseview) {
        size_t len;
        char *report = http_report(500, "View Not Found", &len);

        http_response_header(request, "content-type", "text/html");
        http_response(request, 500, report, len);
        kore_free(report);
        return (KORE_RESULT_OK);
    }

    size_t baseviewsz = strlen(baseview);
    buffer = kore_buf_alloc(baseviewsz);
    kore_buf_append(buffer, baseview, baseviewsz);

    /* Check for macros */
    char *escape = kore_mem_find(buffer->data, buffer->length, "@", 1);
    if (escape) {
        /* Parse view in order */
        //TODO: check for cache
        process_include(buffer);
        //TODO: save current buffer in cache
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
