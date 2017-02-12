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

struct mimetype {
    char ext[16];
    char document[64];
    char mime[64];
};

const char *mime_type(const char *);
const char *mime_document(const char *);

static const struct mimetype mimelist[] = {
    /* audio */
    {"aac", "AAC audio file", "audio/aac"},
    {"mid", "Musical Instrument Digital Interface (MIDI)", "audio/midi"},
    {"midi", "Musical Instrument Digital Interface (MIDI)", "audio/midi"},
    {"oga", "OGG audio", "audio/ogg"},
    {"wav", "Waveform Audio Format", "audio/x-wav"},
    {"weba", "WEBM audio", "audio/x-webm"},

    /* video */
    {"avi", "AVI: Audio Video Interleave", "video/x-msvideo"},
    {"mpeg", "MPEG Video", "video/mpeg"},
    {"ogv", "OGG video", "video/ogg"},
    {"webm", "WEBM video", "video/webm"},
    {"3gp", "3GPP audio/video container", "video/3gpp"},
    {"3g2", "3GPP2 audio/video container", "video/3gpp2"},

    /* image */
    {"jpeg", "JPEG images", "image/jpeg"},
    {"jpg", "JPEG images", "image/jpeg"},
    {"gif", "Graphics Interchange Format (GIF)", "image/gif"},
    {"ico", "Icon format", "image/x-icon"},
    {"svg", "Scalable Vector Graphics (SVG)", "image/svg+xml"},
    {"tif", "Tagged Image File Format (TIFF)", "image/tiff"},
    {"tiff", "Tagged Image File Format (TIFF)", "image/tiff"},
    {"webp", "WEBP image", "image/webp"},

    /* font */
    {"ttf", "TrueType Font", "font/ttf"},
    {"woff", "Web Open Font Format (WOFF)", "font/woff"},
    {"woff2", "Web Open Font Format (WOFF)", "font/woff2"},

    /* application */
    {"abw", "AbiWord document", "application/x-abiword"},
    {"bz", "BZip archive", "application/x-bzip"},
    {"bz2", "BZip2 archive", "application/x-bzip2"},
    {"csh", "C-Shell script", "application/x-csh"},
    {"bin", "Binary data", "application/octet-stream"},
    {"exe", "Binary data", "application/octet-stream"},
    {"dll", "Binary data", "application/octet-stream"},
    {"json", "JSON format", "application/json"},
    {"js", "JavaScript (ECMAScript)", "application/javascript"},
    {"xhtml", "HyperText Markup Language (HTML)", "application/xhtml+xml"},
    {"arc", "Archive document (multiple files embedded)", "application/octet-stream"},
    {"azw", "Amazon Kindle eBook format", "application/vnd.amazon.ebook"},
    {"doc", "Microsoft Word", "application/msword"},
    {"epub", "Electronic publication (EPUB)", "application/epub+zip"},
    {"jar", "Java Archive (JAR)", "application/java-archive"},
    {"mpkg", "Apple Installer Package", "application/vnd.apple.installer+xml"},
    {"odp", "OpenDocuemnt presentation document", "application/vnd.oasis.opendocument.presentation"},
    {"ods", "OpenDocuemnt spreadsheet document", "application/vnd.oasis.opendocument.spreadsheet"},
    {"odt", "OpenDocuemnt text document", "application/vnd.oasis.opendocument.text"},
    {"ogx", "OGG", "application/ogg"},
    {"pdf", "Adobe Portable Document Format (PDF)", "application/pdf"},
    {"ppt", "Microsoft PowerPoint", "application/vnd.ms-powerpoint"},
    {"rar", "RAR archive", "application/x-rar-compressed"},
    {"rtf", "Rich Text Format (RTF)", "application/rtf"},
    {"sh", "Bourne shell script", "application/x-sh"},
    {"swf", "Adobe Flash document", "application/x-shockwave-flash"},
    {"tar", "Tape Archive (TAR)", "application/x-tar"},
    {"vsd", "Microsft Visio", "application/vnd.visio"},
    {"xls", "Microsft Excel", "application/vnd.ms-excel"},
    {"xml", "XML", "application/xml"},
    {"xul", "XUL", "application/vnd.mozilla.xul+xml"},
    {"zip", "ZIP archive", "application/zip"},
    {"7z", "7-zip archive", "application/x-7z-compresse"},

    /* text */
    {"ics", "iCalendar format", "text/calendar"},
    {"txt", "Text document", "text/plain"},
    {"css", "Cascading Style Sheets (CSS)", "text/css"},
    {"csv", "Comma-separated values (CSV)", "text/csv"},
    {"htm", "HyperText Markup Language (HTML)", "text/html"},
    {"html", "HyperText Markup Language (HTML)", "text/html"},
    {"appcache", "Application cache", "text/cache-manifest"},
};

const char *
mime_type(const char *ext)
{
    unsigned int i;

    for (i=0; i < sizeof(mimelist)/sizeof(mimelist[0]); ++i) {
        if (!strcmp(ext, mimelist[i].ext))
            return mimelist[i].mime;
    }

    /* Default MIME */
    return "application/octet-stream";
}

const char *
mime_document(const char *ext)
{
    unsigned int i;

    for (i=0; i < sizeof(mimelist)/sizeof(mimelist[0]); ++i) {
        if (!strcmp(ext, mimelist[i].ext))
            return mimelist[i].document;
    }

    return NULL;
}
