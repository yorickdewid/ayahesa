/**
 * Copyright (C) 2017 Quenza Inc.
 * All Rights Reserved
 *
 * This file is part of the Ayahesa core.
 *
 * Content can not be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef _AYAHESA_BASE64URL_H_
#define _AYAHESA_BASE64URL_H_

/** Base64url encode the given data.
 *
 * This function encodes the data using Base64 URL-safe encoding according to RFC 4648 section 5
 * http://tools.ietf.org/html/rfc4648#section-5
 *
 * Stores the encoded unsigned char sequence in the provided buffer. The caller is responsible for
 * supplying a buffer of sufficient length.
 *
 * The length can be calculated using  result_len = data_len * 4/3
 *
 * The result will not be \0-terminated.
 */
unsigned char *base64url_encode(const unsigned char *data, size_t data_len, unsigned char *result, size_t *result_len);

/** Base64url decode the given data.
 *
 * This function decodes the data using Base64 URL-safe decoding according to RFC 4648 section 5
 * http://tools.ietf.org/html/rfc4648#section-5
 *
 * Stores the decoded unsigned char sequence in the provided buffer. The caller is responsible for
 * supplying a buffer of sufficient length.
 *
 * The length can be calculated using  result_len = data_len * 3/4
 *
 * The result will not be \0-terminated.
 */
void base64url_decode(const unsigned char *data, size_t data_len, unsigned char *result, size_t *result_len);

#endif // _AYAHESA_BASE64URL_H_
