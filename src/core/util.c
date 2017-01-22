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

char *
http_report(int code, char *title, size_t *length)
{
	struct kore_buf		*buffer;
	char 				strcode[4];
	char 				*strtime;

	static const char *default_report =
		"<html>"
		"<head>"
		"<title>Ayahesa Report</title>"
		"<style>"
		"table{font-family:arial,sans-serif;border-collapse:collapse;width:100%}"
		"th{background-color:#ed143d;color:#fff}"
		"td,th{border:1px solid #cccccc;text-align:left;padding:8px}"
		"tr:nth-child(even){background-color:#f1f1f1}"
		"</style>"
		"</head>"
		"<body>"
		"<h1>$title$</h1>"
		"<table>"
		"<tr><th colspan=\"2\">Ayahesa report details</th></tr>"
		"<tr><td>Code</td><td>$code$</td></tr>"
		"<tr><td>Message</td><td>$title$</td></tr>"
		"<tr><td>Occurrence</td><td>$time$</td></tr>"
		"<tr><td>Framework version</td><td>" VERSION "</td></tr>"
		"</table>"
		"</body>"
		"</html>";

	snprintf(strcode, 4, "%d", code);
	strtime = kore_time_to_date(time(NULL)),

	buffer = kore_buf_alloc(strlen(default_report));
	kore_buf_append(buffer, default_report, strlen(default_report));
	kore_buf_replace_string(buffer, "$title$", title, strlen(title));
	kore_buf_replace_string(buffer, "$code$", strcode, 3);
	kore_buf_replace_string(buffer, "$time$", strtime, strlen(strtime));
	return (char *)kore_buf_release(buffer, length);
}

int
jrpc_write_string(struct jsonrpc_request *req, void *ctx)
{
	const unsigned char *str = (unsigned char *)ctx;

	return yajl_gen_string(req->gen, str, strlen((const char *)str));
}
