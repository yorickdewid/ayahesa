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
#include "base64.h"
#include "util.h"

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

/* Strip whitespace chars off end of given string, in place. Return s. */
char *
rstrip(char* s)
{
    char *p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
char *
lskip(char *s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
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
	char *header_auth = NULL;

    if (basic_auth_count >= 5) {
        sleep(5);
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
        char *undecode = (char *)base64_decode(code, strlen(code));
        if (!strcmp(undecode, auth)) {
            kore_free(undecode);
            return 1;
        }

        kore_free(undecode);
    }

    basic_auth_count++;
    return 0;
}

int
jrpc_write_string(struct jsonrpc_request *req, void *ctx)
{
	const unsigned char *str = (unsigned char *)ctx;

	return yajl_gen_string(req->gen, str, strlen((const char *)str));
}

int
jrpc_write_string_array_params(struct jsonrpc_request *req, void *ctx)
{
	int status = 0;

	if (!YAJL_GEN_KO(status = yajl_gen_array_open(req->gen))) {
		for (size_t i = 0; i < req->params->u.array.len; i++) {
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
