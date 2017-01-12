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
#include "ini.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define INI_MAX_LINE 200
#define MAX_SECTION 50
#define MAX_NAME 50

#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
static char *
find_chars_or_comment(char *s, const char *chars)
{
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) &&
           !(was_space && strchr(";", *s))) {
        was_space = isspace((unsigned char)(*s));
        s++;
    }

    return (char *)s;
}

/* See documentation in header file. */
int
ini_parse_stream(ini_reader reader, void *stream, ini_handler handler,
                     void *user)
{
    char *line;
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char *start;
    char *end;
    char *name;
    char *value;
    int lineno = 0;
    int error = 0;

    line = (char *)kore_malloc(INI_MAX_LINE);
    if (!line) {
        return -2;
    }

    /* Scan through stream line by line */
    while (reader(line, INI_MAX_LINE, stream) != NULL) {
        lineno++;

        start = line;

        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }

        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python configparser, allow both ; and # comments at the
               start of a line */
        } else if (*prev_name && *start && start > line) {
            /* Non-blank line with leading whitespace, treat as continuation
               of previous name's value (as per Python configparser). */
            if (!HANDLER(user, section, prev_name, start) && !error)
                error = lineno;
        } else if (*start == '[') {
            /* A "[section]" line */
            end = find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        } else if (*start) {
            /* Not a comment, must be a name[=:]value pair */
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = end + 1;
                end = find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
                value = lskip(value);
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!HANDLER(user, section, name, value) && !error)
                    error = lineno;
            } else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }

        /* Stop parsing on first error. */
        if (error)
            break;
    }

    kore_free(line);

    return error;
}

/* See documentation in header file. */
int
ini_parse_file(FILE *file, ini_handler handler, void *user)
{
    return ini_parse_stream((ini_reader)fgets, file, handler, user);
}

/* See documentation in header file. */
int
ini_parse(const char *filename, ini_handler handler, void *user)
{
    FILE *file;
    int error;

    file = fopen(filename, "r");
    if (!file)
        return -1;

    error = ini_parse_file(file, handler, user);
    fclose(file);

    return error;
}
