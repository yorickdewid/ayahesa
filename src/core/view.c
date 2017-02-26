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

#include "util.h"
#include <time.h>
#include <dlfcn.h>

#define TOk_INCLUDE        "@include("
#define TOk_FUNCTION       "@function("
#define TOk_USER_VAR       "@var("
#define TOk_TEMPLATE       "@template("
#define TOk_ENDTEMPLATE    "@endtemplate"
#define TOk_IF             "@if("
#define TOk_ELSE           "@else"
#define TOk_ENDIF          "@endif"
#define TOk_CLOSE          ')'
#define TOk_COMMENT_OPEN   "@/*"
#define TOk_COMMENT_CLOSE  "*/@"

static char **
split_arguments(char *cmdline, size_t cmdlinesz, int *argc)
{
    char **argv = (char **)kore_calloc(16, sizeof(char *));
    *argc = 0;

    /* Signle argument chec */
    if (!strchr(cmdline, ',')) {
        (*argc)++;
        argv[0] = cmdline;
        return argv;
    }

    size_t i; char *next = cmdline;
    for (i=0; i<cmdlinesz && *argc < 16; ++i) {
        if (cmdline[i] == ',') {
            cmdline[i++] = '\0';

            /* Arguments */
            argv[(*argc)++] = trim(next);
            next = cmdline + i;
        }
    }

    /* Last argument */
    argv[(*argc)++] = trim(next);
    next = cmdline + i;

    return argv;
}

static char *
include_asset(const char *name)
{
    char buffer[64];

    snprintf(buffer, 64, "asset_%s_html", name);

    void *hndl = dlopen(NULL, RTLD_LAZY);
    if (!hndl)
        return NULL;

    char *asset = dlsym(hndl, buffer);
    if (!asset) {
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
    if (!hndl)
        return NULL;

    *(char **)(&vfunc) = dlsym(hndl, argv[0]);
    if (!vfunc) {
        dlclose(hndl);
        return NULL;
    }

    dlclose(hndl);
    return vfunc(argc, argv);
}

static int
get_vvar(char *var)
{
    void *hndl = dlopen(NULL, RTLD_LAZY);
    if (!hndl)
        return -1;

    void *vvar = dlsym(hndl, var);
    if (!vvar) {
        dlclose(hndl);
        return -1;
    }

    dlclose(hndl);
    return (*(int *)vvar);
}

static char *
parser(struct kore_buf *buffer, char *token, size_t tokensz, size_t *argumentsz, char **posx_start, size_t *posx_length)
{
    char *include = kore_mem_find(buffer->data, buffer->offset, token, tokensz);
    if (!include)
        return NULL;

    /* Find closing bracket */
    char *pos_end = strchr(include, TOk_CLOSE);
    if (!pos_end)
        return NULL;

    char *pos_start = include + tokensz;

    *argumentsz = pos_end - pos_start;

    *posx_start = include;
    *posx_length = (pos_start + *argumentsz) - *posx_start;

    return pos_start;
}

static void
process_template(struct kore_buf **buffer)
{
    char            *argument;
    char            *pos_start;
    size_t          argumentsz;
    size_t          pos_length;

    argumentsz = 0;
    argument = parser(*buffer, TOk_TEMPLATE, sizeof(TOk_TEMPLATE) - 1, &argumentsz, &pos_start, &pos_length);
    if (!argument || !argumentsz)
        return;

    argument[argumentsz] = '\0';
    argument = trim(argument);

    /* Find template */
    char *asset = include_asset(argument);
    if (!asset)
        return;

    /* Block beginning and end */
    char *begin = argument + argumentsz + 1;
    char *end = kore_mem_find((*buffer)->data, (*buffer)->offset, TOk_ENDTEMPLATE, sizeof(TOk_ENDTEMPLATE) - 1);
    if (!end)
        return;

    end[0] = '\0';

    /* Convert asset into buffer */
    size_t assetsz = strlen(asset);
    struct kore_buf *template_buffer = kore_buf_alloc(assetsz);
    kore_buf_append(template_buffer, asset, assetsz);

    /* Replace yield and swap buffers */
    aya_buf_replace_first_string(template_buffer, "@yield", begin, strlen(begin));
    kore_buf_cleanup(*buffer);
    *buffer = template_buffer;
}

static void
process_comment(struct kore_buf *buffer)
{
    /* Keep processing comments */
    for (;;) {
        /* Comment block beginning and end */
        char *begin = kore_mem_find(buffer->data, buffer->offset, TOk_COMMENT_OPEN, sizeof(TOk_COMMENT_OPEN) - 1);
        if (!begin)
            return;

        char *end = kore_mem_find(buffer->data, buffer->offset, TOk_COMMENT_CLOSE, sizeof(TOk_COMMENT_CLOSE) - 1);
        if (!end)
            return;

        /* Replace substring */
        aya_buf_replace_string(buffer, begin, (end - begin) + sizeof(TOk_COMMENT_CLOSE) - 1, NULL, 0);
    }
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
        argument = trim(argument);
        char *asset = include_asset(argument);
        if (!asset)
            return;

        /* Replace substring */
        aya_buf_replace_string(buffer, pos_start, pos_length, asset, strlen(asset));
    }
}

static void
process_function(struct kore_buf *buffer)
{
    char            *argument;
    char            *pos_start;
    size_t          argumentsz;
    size_t          pos_length;

    for (;;) {
        argumentsz = 0;
        argument = parser(buffer, TOk_FUNCTION, sizeof(TOk_FUNCTION) - 1, &argumentsz, &pos_start, &pos_length);
        if (!argument || !argumentsz)
            return;

        int argc;
        argument[argumentsz] = '\0';
        argument = trim(argument);
        char **argv = split_arguments(argument, argumentsz, &argc);
        if (!argc)
            return;

        char *str = call_vfunc(argc, argv);

        kore_free(argv);

        /* Replace substring */
        aya_buf_replace_string(buffer, pos_start, pos_length, str, strlen(str));
    }
}

static void
process_statement(struct kore_buf *buffer)
{
    char            *argument;
    char            *pos_start;
    size_t          argumentsz;
    size_t          pos_length;

    for (;;) {
        argumentsz = 0;

        /* Find IF token */
        argument = parser(buffer, TOk_IF, sizeof(TOk_IF) - 1, &argumentsz, &pos_start, &pos_length);
        if (!argument || !argumentsz)
            return;
        
        argument[argumentsz] = '\0';
        argument = trim(argument);

        /* Find ENDIF token */
        char *end = kore_mem_find(argument, buffer->offset - pos_length, TOk_ENDIF, sizeof(TOk_ENDIF) - 1);
        if (!end)
            return;

        /* Find ELSE token */
        char *middle = kore_mem_find(argument, end - pos_start, TOk_ELSE, sizeof(TOk_ELSE) - 1);

        /* Evaluate condition */
        int res = get_vvar(argument);
        if (res < 0)
            return;

        size_t replace_len = (size_t)(end - pos_start);
        if (!middle) {
            if (res)
                replace_len = pos_length;
        } else {
            if (res) {
                replace_len = pos_length;
            } else {
                replace_len = (size_t)(middle - pos_start);
            }
        }

        /* Replace substring */
        aya_buf_replace_string(buffer, pos_start, replace_len, NULL, 0);

        if (middle) {
            /* Find ELSE token in new buffer */
            middle = kore_mem_find(buffer->data, buffer->offset, TOk_ELSE, sizeof(TOk_ELSE) - 1);
            if (!middle)
                return;

            /* Find ENDIF token in new buffer */
            end = kore_mem_find(buffer->data, buffer->offset, TOk_ENDIF, sizeof(TOk_ENDIF) - 1);
            if (!end)
                return;

            aya_buf_replace_string(buffer, middle, end - middle, NULL, 0);
        }
        
        aya_buf_replace_first_string(buffer, TOk_ENDIF, NULL, 0);
    }
}

/* Easy substitution */
static void
process_user_vars(struct http_request *request, struct kore_buf *buffer)
{
    char            *argument;
    char            *pos_start;
    size_t          argumentsz;
    size_t          pos_length;

    /* Must have request data */
    if (!request->hdlr_extra)
        return;

    /* Must have session tree */
    struct request_data *session = (struct request_data *)request->hdlr_extra;
    if (!session->session)
        return;

    for (;;) {
        argumentsz = 0;
        argument = parser(buffer, TOk_USER_VAR, sizeof(TOk_USER_VAR) - 1, &argumentsz, &pos_start, &pos_length);
        if (!argument || !argumentsz)
            return;

        argument[argumentsz] = '\0';
        argument = trim(argument);

        char *value = NULL; size_t valuesz = 0;
        tree_get_str(session->session, argument, &value);
        if (value)
            valuesz = strlen(value);

        /* Replace substring */
        aya_buf_replace_string(buffer, pos_start, pos_length, value, valuesz);
    }
}

/* Easy substitution */
static void
process_predef_vars(struct http_request *request, struct kore_buf *buffer)
{
    char domainname[128];
    char methodname[8];

    size_t auth_usersz = 0; 
    char *auth_user = NULL;
    char *principal = http_auth_principal(request);
    if (principal) {
        auth_user = principal;
        auth_usersz = strlen(auth_user);
    }

    char *year = aya_itoa(date_year());
    char *instance = app_instance();
    char *date = kore_time_to_date(time(NULL));
    char *appname = app_name();
    char *uptime = app_uptime();
    char *env = app_environment();
    char *uri = request->path;
    char *query_string = request->query_string ? request->query_string : "-";
    char *host = request->host;
    char *agent = request->agent;
    char *remote_addr = http_remote_addr(request);

    const char *domain = app_domainname();
    aya_strlcpy(domainname, domain, 128);
    const char *method = http_method_text(request->method);
    aya_strlcpy(methodname, method, 8);

    kore_buf_replace_string(buffer, "@year", year, 4);
    kore_buf_replace_string(buffer, "@framework", VERSION, strlen(VERSION));
    kore_buf_replace_string(buffer, "@instance", instance, strlen(instance));
    kore_buf_replace_string(buffer, "@date", date, strlen(date));
    kore_buf_replace_string(buffer, "@name", appname, strlen(appname));
    kore_buf_replace_string(buffer, "@user", auth_user, auth_usersz);
    kore_buf_replace_string(buffer, "@uptime", uptime, strlen(uptime));
    kore_buf_replace_string(buffer, "@env", env, strlen(env));
    kore_buf_replace_string(buffer, "@domain", domainname, strlen(domainname));
    kore_buf_replace_string(buffer, "@method", methodname, strlen(methodname));
    kore_buf_replace_string(buffer, "@uri", uri, strlen(uri));
    kore_buf_replace_string(buffer, "@query_string", query_string, strlen(query_string));
    kore_buf_replace_string(buffer, "@host", host, strlen(host));
    kore_buf_replace_string(buffer, "@agent", agent, strlen(agent));
    kore_buf_replace_string(buffer, "@remote_addr", remote_addr, strlen(remote_addr));
}

int
http_view(struct http_request *request, int code, const char *view)
{
    struct kore_buf     *buffer;
    size_t              length = 0;

    /* Retrieve base view */
    char *baseview = include_asset(view);
    if (!baseview) {
        char *report = "View Not Found";

        http_response_header(request, "content-type", "text/plain");
        http_response(request, 500, report, strlen(report));
        return_ok();
    }

    size_t baseviewsz = strlen(baseview);
    buffer = kore_buf_alloc(baseviewsz);
    kore_buf_append(buffer, baseview, baseviewsz);

    /* Check for macros */
    char *escape = kore_mem_find(buffer->data, buffer->offset, "@", 1);
    if (escape) {
        /* Parse view in order */
        //TODO: check for cache
        process_template(&buffer);
        process_include(buffer);
        process_comment(buffer);
        //TODO: save current buffer in cache
        process_function(buffer);
        process_statement(buffer);
        process_user_vars(request, buffer);
        process_predef_vars(request, buffer);
    }

    /* Convert and respond */
    char *str = kore_buf_stringify(buffer, &length);
    http_response_header(request, "content-type", "text/html");
    http_response(request, code, str, length);

    kore_buf_cleanup(buffer);
    return_ok();
}
