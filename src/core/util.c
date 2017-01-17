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

static int basic_auth_count = 0;

char *
generate_instance_id(void)
{
    static const char ar[] = "abcdefghijklmnopqrstuvwxyz";
    unsigned int rnd;

    char *str = (char *)kore_malloc(10);

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
    char *undecode = NULL;
    size_t undecodelen;
    char *header_auth = NULL;

    if (basic_auth_count >= 5) {
        sleep(2);
        basic_auth_count = 0;
    }

    http_request_header(request, "authorization", &header_auth);
	if (header_auth == NULL) {
        basic_auth_count++;
		return 0;
    }
	
    char *method = strtok(header_auth, " ");
    if (method == NULL) {
        basic_auth_count++;
        return 0;
    }

    if (!strcmp(strtolower(method), "basic")) {
        char *code = strtok(NULL, " ");

        kore_base64_decode(code, (u_int8_t **)&undecode, &undecodelen);
        if (undecodelen != strlen(auth))
            return 0;
        if (!strncmp(undecode, auth, undecodelen))
            return 1;
    }

    basic_auth_count++;
    return 0;
}

char *
http_remote_addr(struct http_request *request)
{
    static char astr[INET6_ADDRSTRLEN];
    char *real_ip = NULL;

    http_request_header(request, "x-forwarded-for", &real_ip);
	if (real_ip != NULL) {
		return real_ip;
    }

    http_request_header(request, "x-real-ip", &real_ip);
	if (real_ip != NULL) {
		return real_ip;
    }

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

int
jrpc_write_string(struct jsonrpc_request *req, void *ctx)
{
	const unsigned char *str = (unsigned char *)ctx;

	return yajl_gen_string(req->gen, str, strlen((const char *)str));
}

//TODO: remove ?
int
jrpc_write_string_array_params(struct jsonrpc_request *req, void *ctx)
{
	int status = 0;
    size_t i;

	if (!YAJL_GEN_KO(status = yajl_gen_array_open(req->gen))) {
		for (i = 0; i < req->params->u.array.len; ++i) {
			yajl_val yajl_str = req->params->u.array.values[i];
			char	 *str = YAJL_GET_STRING(yajl_str);

			if (YAJL_GEN_KO(status = yajl_gen_string(req->gen,
			    (unsigned char *)str, strlen(str))))
				break;
		}
		if (status == 0)
			status = yajl_gen_array_close(req->gen);
	}

	return status;
}
