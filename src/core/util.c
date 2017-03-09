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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>

static int basic_auth_count = 0;

char *
generate_instance_id(void)
{
    static const char ar[] = "abcdefghijklmnopqrstuvwxyz";
    unsigned int rnd;

    char *str = (char *)aya_malloc(10);

    str[0] = ar[rand() % strlen(ar)];
    str[1] = ar[rand() % strlen(ar)];
    str[2] = '-';

    int i;
    for (i=3; i<9; ++i) {
        rnd = rand() % strlen(ar);
        str[i] = toupper(ar[rnd]);
    }

    str[9] = '\0';

    return str;
}

void
random_string(unsigned char buffer[], size_t size, int nonblock)
{
    int devfp = open(nonblock ? "/dev/urandom" : "/dev/random", O_RDONLY);
    size_t datalen = 0;

    while (datalen < size) {
        ssize_t result = read(devfp, buffer + datalen, size - datalen);
        if (result < 0)
            break;

        datalen += result;
    }

    close(devfp);
}

int
date_year(void)
{
    time_t timeval;

    time(&timeval);
    struct tm *tp = gmtime(&timeval);
    return tp->tm_year + 1900;
}

const char *
file_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return NULL;

    return dot + 1;
}

const char *
http_get_cookie(struct http_request *request, const char *name)
{
    char *cookiejar = NULL;
    http_request_header(request, "cookie", &cookiejar);
    if (cookiejar == NULL)
        return NULL;

    char *pch;
    char iskey = 1;
    char isfound = 0;

    pch = strtok(cookiejar, " ;=");
    while (pch != NULL) {
        if (iskey) {
            if (!strcmp(pch, name))
                isfound = 1;
            iskey = 0;
        } else {
            if (isfound)
                return pch;
            iskey = 1;
        }

        pch = strtok(NULL, " ;=");
    }

    return NULL;
}

int
http_basic_auth(struct http_request *request, const char *auth)
{
    char        *undecode = NULL;
    size_t      undecodelen;
    char        *header_auth = NULL;

    /* Bruteforce countermeasures */
    if (basic_auth_count >= 20) {
        sleep(5);
    } else if (basic_auth_count >= 5) {
        sleep(1);
    }

    http_request_header(request, "authorization", &header_auth);
    if (!header_auth) {
        basic_auth_count++;
        return 0;
    }
    
    char *method = strtok(header_auth, " ");
    if (!method) {
        basic_auth_count++;
        return 0;
    }

    if (!strcmp(strtolower(method), "basic")) {
        char *code = strtok(NULL, " ");

        kore_base64_decode(code, (u_int8_t **)&undecode, &undecodelen);
        if (undecodelen != strlen(auth)) {
            basic_auth_count++;
            aya_free(undecode);
            return 0;
        }

        if (!strncmp(undecode, auth, undecodelen)) {
            basic_auth_count = 0;
            aya_free(undecode);
            return 1;
        }
    }

    basic_auth_count++;
    return 0;
}

char *
http_remote_addr(struct http_request *request)
{
    static char     astr[INET6_ADDRSTRLEN];
    char            *real_ip = NULL;

    http_request_header(request, "x-forwarded-for", &real_ip);
    if (real_ip)
        return real_ip;

    http_request_header(request, "x-real-ip", &real_ip);
    if (real_ip)
        return real_ip;

    switch (request->owner->addrtype) {
        case AF_INET:
            inet_ntop(AF_INET, &(request->owner->addr.ipv4.sin_addr), astr, INET_ADDRSTRLEN);
            break;
        case AF_INET6:
            inet_ntop(AF_INET6, &(request->owner->addr.ipv6.sin6_addr), astr, INET6_ADDRSTRLEN);
            break;
        default:
            return NULL;

    }

    return astr;
}

char *
http_auth_principal(struct http_request *request)
{
    struct request_data *data = NULL;
    if (!request->hdlr_extra)
        return NULL;

    data = (struct request_data *)request->hdlr_extra;
    if (!data->auth.principal)
        return NULL;

    return data->auth.principal;
}

void
http_report(struct http_request *request, int code, char *msg)
{
    if (!request->hdlr_extra)
        request->hdlr_extra = aya_calloc(1, sizeof(struct request_data));

    /* Setup session tree */
    struct request_data *session = (struct request_data *)request->hdlr_extra;
    session->session = (app_t *)aya_calloc(1, sizeof(app_t));
    session->session->type = T_NULL;
    session->session->flags = 0;
    session->session->key = NULL;
    tree_new_root(session->session);

    /* Push view variables */
    tree_put_str(session->session, "background-clr", "#ed143d");
    tree_put_str(session->session, "title", msg);
    tree_put_str(session->session, "code", aya_itoa(code));
    tree_put_str(session->session, "msg", msg);

    /* Parse view */
    http_view(request, code, "report");
}

int
jrpc_write_string(struct jsonrpc_request *req, void *ctx)
{
    const unsigned char *str = (unsigned char *)ctx;

    return yajl_gen_string(req->gen, str, strlen((const char *)str));
}

void
aya_buf_replace_string(struct kore_buf *b, char *pos_start, size_t pos_length,void *dst, size_t len)
{
    char        *pos_end, *tmp;
    size_t      pre_len, post_len, new_len;

    new_len = b->offset + len;
    pos_end = pos_start + pos_length;
    pre_len = pos_start - (char *)b->data;
    post_len = ((char *)(b->data + b->offset) - pos_end);

    tmp = aya_malloc(new_len);
    memcpy(tmp, b->data, pre_len);
    if (dst != NULL)
        memcpy((tmp + pre_len), dst, len);
    memcpy((tmp + pre_len + len), pos_end, post_len);

    aya_free(b->data);
    b->data = (u_int8_t *)tmp;
    b->offset = pre_len + len + post_len;
    b->length = new_len;
}

void
aya_buf_replace_first_string(struct kore_buf *b, char *src, void *dst, size_t len)
{
    char        *pos_start, *pos_end, *tmp;
    size_t      pre_len, post_len, new_len, pos_length;

    pos_length = strlen(src);
    pos_start = kore_mem_find(b->data, b->offset, src, pos_length);
    if (pos_start == NULL)
        return;

    new_len = b->offset + len;
    pos_end = pos_start + pos_length;
    pre_len = pos_start - (char *)b->data;
    post_len = ((char *)(b->data + b->offset) - pos_end);

    tmp = aya_malloc(new_len);
    memcpy(tmp, b->data, pre_len);
    if (dst != NULL)
        memcpy((tmp + pre_len), dst, len);
    memcpy((tmp + pre_len + len), pos_end, post_len);

    aya_free(b->data);
    b->data = (u_int8_t *)tmp;
    b->offset = pre_len + len + post_len;
    b->length = new_len;
}
