/*
 * Copyright (c) 2012-2017, Yorick de Wid <ydw at x3 dot quenza dot net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * 2012-08: Version 1.0
 * 2017-01: Version 1.3
 *          - QUID version 7
 *          - Fix string functions
 *          - Testcases
 * 2017-01: Version 1.4
 *          - User defined tags
 *
 * TODO:
 * - Last digit in timestamp
 * - Move randon cipher counter
 */

#ifndef __FreeBSD__
#define _DEFAULT_SOURCE
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __WIN32__
#include <unistd.h>
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>
#endif

#include <quid.h>

#include "chacha.h"

#define UIDS_PER_TICK   1024             /* Generate identifiers per tick interval */
#define EPOCH_DIFF      11644473600LL    /* Conversion needed for EPOCH to UTC */
#define RANDFILE        ".rnd"           /* File descriptor for random seed */
#define MEM_SEED_CYCLE  65536            /* Generate new memory seed after interval */
#define RND_SEED_CYCLE  4096             /* Generate new random seed after interval */
#define SEEDSZ          16               /* Seed size */
#define QUIDMAGIC       0x80             /* QUID Timestamp magic */

#define VERSION_REV4    0xa000
#define VERSION_REV7    0xb000

typedef unsigned long long cuuid_time_t;

/*
 * Temporary node structure
 */
typedef struct {
    uint8_t node[6];     /* Allocate 6 nodes */
} cuuid_node_t;

/*
 * Prototypes
 */
static void             format_quid_rev4(cuuid_t *, unsigned short, cuuid_time_t, cuuid_node_t);
static void             format_quid_rev7(cuuid_t *, unsigned short, cuuid_time_t);
static void             encrypt_node(uint64_t, uint8_t, uint8_t, cuuid_node_t *);
static void             get_current_time(cuuid_time_t *);
static unsigned short   true_random(void);

static int max_mem_seed = MEM_SEED_CYCLE;
static int max_rnd_seed = RND_SEED_CYCLE;

static const uint8_t padding[3] = {0x12, 0x82, 0x7b};

/* Set memory seed cycle */
void quid_set_mem_seed(int cnt) {
    max_mem_seed = cnt;
}

/* Set rnd seed cycle */
void quid_set_rnd_seed(int cnt) {
    max_rnd_seed = cnt;
}

/* Compare two QUID structures */
int quid_cmp(const cuuid_t *s1, const cuuid_t *s2) {
    return s1->time_low == s2->time_low
        && s1->time_mid == s2->time_mid
        && s1->time_hi_and_version == s2->time_hi_and_version
        && s1->clock_seq_hi_and_reserved == s2->clock_seq_hi_and_reserved
        && s1->clock_seq_low == s2->clock_seq_low
        && s1->node[0] == s2->node[0]
        && s1->node[1] == s2->node[1]
        && s1->node[2] == s2->node[2]
        && s1->node[3] == s2->node[3]
        && s1->node[4] == s2->node[4]
        && s1->node[5] == s2->node[5];
}

/* Retrieve system time */
static void get_system_time(cuuid_time_t *cuuid_time) {
#ifdef __WIN32___
    ULARGE_INTEGER time;

    GetSystemTimeAsFileTime((FILETIME *)&time);
    time.QuadPart += (unsigned __int64) (1000*1000*10)
                    * (unsigned __int64) (60 * 60 * 24)
                    * (unsigned __int64) (17+30+31+365*18+5);
    *cuuid_time = time.QuadPart;
#else
    struct timeval tv;
    uint64_t result = EPOCH_DIFF;
    gettimeofday(&tv, NULL);
    result += tv.tv_sec;
    result *= 10000000LL;
    result += tv.tv_usec * 10;
    *cuuid_time = result;
#endif
}

/* Retrieve timestamp from QUID */
static void quid_timeval(cuuid_t *cuuid, struct timeval *tv) {
    cuuid_time_t cuuid_time;
    uint16_t versubtr = 0;
    long int usec;
    time_t sec;

    /* Determine version substraction */
    switch (cuuid->version) {
        case QUID_REV4:
            versubtr = VERSION_REV4;
            break;
        case QUID_REV7:
            versubtr = VERSION_REV7;
            break;
    }

    /* Reconstruct timestamp */
    cuuid_time = (uint64_t)cuuid->time_low | (uint64_t)cuuid->time_mid << 32 | 
    (uint64_t)((cuuid->time_hi_and_version ^ QUIDMAGIC) - versubtr) << 48;

    /* Timestamp to timeval */
    usec = (cuuid_time/10) % 1000000LL;
    sec = (((cuuid_time/10) - usec)/1000000LL) - EPOCH_DIFF;

    tv->tv_sec = sec;
    tv->tv_usec = usec;
}

/* Retrieve timestamp */
struct tm *quid_timestamp(cuuid_t *cuuid) {
    struct timeval tv;

    quid_timeval(cuuid, &tv);

    /* Localtime */
    return localtime(&tv.tv_sec);
}

/* Retrieve microtime */
long quid_microtime(cuuid_t *cuuid) {
    struct timeval tv;

    quid_timeval(cuuid, &tv);

    /* Microseconds */
    return tv.tv_usec;
}

/* Retrieve user tag */
const char *quid_tag(cuuid_t *cuuid) {
    cuuid_node_t node;
    static char tag[3];

    /* Skip older formats */
    if (cuuid->version != QUID_REV7)
        return "Not implemented";

    memcpy(&node, &cuuid->node, sizeof(cuuid_node_t));
    encrypt_node(cuuid->time_low, cuuid->clock_seq_hi_and_reserved, cuuid->clock_seq_low, &node);

    /* Must match version */
    if (node.node[0] != QUID_REV7)
        return "Invalid";

    /* Check for padding */
    if (node.node[3] == padding[0] &&
        node.node[4] == padding[1] &&
        node.node[5] == padding[2])
        return "None";

    tag[0] = node.node[3];
    tag[1] = node.node[4];
    tag[2] = node.node[5];

    return tag;
}

/* Retrieve category */
uint8_t quid_category(cuuid_t *cuuid) {
    cuuid_node_t node;

    /* Determine category per version */
    switch (cuuid->version) {
        case QUID_REV4:
            return cuuid->node[2];
        case QUID_REV7: {
            memcpy(&node, &cuuid->node, sizeof(cuuid_node_t));
            encrypt_node(cuuid->time_low, cuuid->clock_seq_hi_and_reserved, cuuid->clock_seq_low, &node);
            return node.node[2];
        }
    }

    /* Invalid */
    return 0x0;
}

/* Retrieve flag if any */
uint8_t quid_flag(cuuid_t *cuuid) {
    cuuid_node_t node;

    /* Determine category per version */
    switch (cuuid->version) {
        case QUID_REV4:
            return cuuid->node[1];
        case QUID_REV7: {
            memcpy(&node, &cuuid->node, sizeof(cuuid_node_t));
            encrypt_node(cuuid->time_low, cuuid->clock_seq_hi_and_reserved, cuuid->clock_seq_low, &node);
            return node.node[1];
        }
    }

    /* Invalid */
    return 0x0;
}

/*
 * Read seed or create if not exist (Obsolete)
 * Compilers and flatforms may zero stack
 * and/or heap memory beforehand causing low
 * entropy QUID nodes.
 */
static void get_memory_seed(cuuid_node_t *node) {
    static int mem_seed_count = 0;
    static cuuid_node_t saved_node;
    uint8_t seed[SEEDSZ];
    FILE *fp;

    if (!mem_seed_count) {
        fp = fopen(RANDFILE, "rb");
        if (fp) {
            if (fread(&saved_node, sizeof(saved_node), 1, fp) < 1)
                abort();
            fclose(fp);
        } else {
            seed[0] |= 0x01;
            memcpy(&saved_node, seed, sizeof(saved_node));

            fp = fopen(RANDFILE, "wb");
            if (fp) {
                if (fwrite(&saved_node, sizeof(saved_node), 1, fp) < 1)
                    abort();
                fclose(fp);
            }
        }
    }

    /* Advance counters */
    mem_seed_count = (mem_seed_count == max_mem_seed) ? 0 : mem_seed_count + 1;

    *node = saved_node;
}

/*
 * Run the nods agains ChaCha in order to XOR encrypt or decrypt
 * The key and IV are stretched to match the stream input. The derivations
 * are by no means secure and are only applied to increase diffusion. The stream
 * cipher may use low rounds as the primary goal is entropy, not privacy.
 */
void encrypt_node(uint64_t prekey, uint8_t preiv1, uint8_t preiv2, cuuid_node_t *node) {
    chacha_ctx ctx;
    uint8_t key[16];
    uint8_t iv[8];

    /* Weak key stretching */
    key[0] = 0x0;
    key[1] = (uint8_t)prekey | (uint8_t)(prekey >> 16);
    key[2] = (uint8_t)prekey ^ (uint8_t)(prekey >> 8);
    key[3] = (uint8_t)prekey;
    
    key[4] = 0x1;
    key[5] = (uint8_t)(prekey >> 8) | (uint8_t)(prekey >> 24);
    key[6] = (uint8_t)(prekey >> 8) ^ (uint8_t)(prekey >> 16);
    key[7] = (uint8_t)(prekey >> 8);
    
    key[8] = 0x2;
    key[9] = (uint8_t)(prekey >> 16) | (uint8_t)prekey;
    key[10] = (uint8_t)(prekey >> 16) ^ (uint8_t)(prekey >> 24);
    key[11] = (uint8_t)(prekey >> 16);
    
    key[12] = 0x3;
    key[13] = (uint8_t)(prekey >> 24) | (uint8_t)(prekey >> 8);
    key[14] = (uint8_t)(prekey >> 24) ^ (uint8_t)prekey;
    key[15] = (uint8_t)(prekey >> 24);

    /* IV derivation */
    iv[0] = 0x0;
    iv[1] = 0x1;
    iv[2] = preiv1 ^ preiv2;
    iv[3] = preiv1;

    iv[4] = 0x4;
    iv[5] = 0x5;
    iv[6] = preiv2 & preiv1;
    iv[7] = preiv2;

    /* Prepare stream */
    chacha_init_ctx(&ctx, 4);
    chacha_init(&ctx, key, 128, iv, 0);

    chacha_xor(&ctx, (uint8_t *)node, sizeof(cuuid_node_t));
}

/* QUID format REV4 */
int quid_create_rev4(cuuid_t *uid, uint8_t flag, uint8_t subc) {
    cuuid_time_t    timestamp;
    unsigned short  clockseq;
    cuuid_node_t    node;

    get_current_time(&timestamp);
    get_memory_seed(&node);
    clockseq = true_random();

    /* Format QUID */
    format_quid_rev4(uid, clockseq, timestamp, node);

    /* Set flags and subclasses */
    uid->node[1] = flag;
    uid->node[2] = subc;

    return QUID_OK;
}

/* QUID format REV7 */
int quid_create_rev7(cuuid_t *uid, uint8_t flag, uint8_t subc, char tag[3]) {
    cuuid_time_t    timestamp;
    unsigned short  clockseq;
    cuuid_node_t    node;

    get_current_time(&timestamp);
    clockseq = true_random();

    /* Format QUID */
    format_quid_rev7(uid, clockseq, timestamp);

    /* Prepare nodes */
    node.node[0] = QUID_REV7;
    node.node[1] = flag;
    node.node[2] = subc;
    node.node[3] = padding[0];
    node.node[4] = padding[1];
    node.node[5] = padding[2];

    /* Set tag if passed */
    if (tag &&
        tag[0] != 0x0 &&
        tag[1] != 0x0 &&
        tag[2] != 0x0) {
        node.node[3] = tag[0];
        node.node[4] = tag[1];
        node.node[5] = tag[2];
    }

    /* Encrypt nodes */
    encrypt_node(uid->time_low, uid->clock_seq_hi_and_reserved, uid->clock_seq_low, &node);
    memcpy(&uid->node, &node, sizeof(uid->node));

    return QUID_OK;
}

/* Default constructor */
int quid_create(cuuid_t *cuuid, uint8_t flag, uint8_t subc, char tag[3]) {
    memset(cuuid, '\0', sizeof(cuuid_t));
    if (cuuid->version == QUID_REV4)
        return quid_create_rev4(cuuid, flag, subc);

    /* Default to latest */
    cuuid->version = QUID_REV7;
    return quid_create_rev7(cuuid, flag, subc, tag);
}

/*
 * Format QUID from the timestamp, clocksequence, and node ID
 * Structure succeeds version 3 (REV1)
 */
void format_quid_rev4(cuuid_t* uid, uint16_t clock_seq, cuuid_time_t timestamp, cuuid_node_t node) {
    uid->time_low = (uint64_t)(timestamp & 0xffffffff);
    uid->time_mid = (uint16_t)((timestamp >> 32) & 0xffff);

    uid->time_hi_and_version = (uint16_t)((timestamp >> 48) & 0xFFF);
    uid->time_hi_and_version ^= QUIDMAGIC;
    uid->time_hi_and_version |= VERSION_REV4;

    uid->clock_seq_low = (clock_seq & 0xff);
    uid->clock_seq_hi_and_reserved = (clock_seq & 0x3f00) >> 8;
    uid->clock_seq_hi_and_reserved |= QUIDMAGIC;

    memcpy(&uid->node, &node, sizeof(uid->node));
    uid->node[0] = true_random();
    uid->node[1] = QUID_REV4;
    uid->node[5] = (true_random() & 0xff);
}

/*
 * Format QUID from the timestamp, clocksequence, and node ID
 * Structure succeeds version 7 (REV7)
 */
void format_quid_rev7(cuuid_t* uid, uint16_t clock_seq, cuuid_time_t timestamp) {
    uid->time_low = (uint64_t)(timestamp & 0xffffffff);
    uid->time_mid = (uint16_t)((timestamp >> 32) & 0xffff);

    uid->time_hi_and_version = (uint16_t)((timestamp >> 48) & 0xfff);
    uid->time_hi_and_version ^= QUIDMAGIC;
    uid->time_hi_and_version |= VERSION_REV7;

    uid->clock_seq_low = (clock_seq & 0xff);
    uid->clock_seq_hi_and_reserved = (clock_seq & 0x4e00) >> 8;
    uid->clock_seq_hi_and_reserved |= QUIDMAGIC;
}

/* Get current time including cpu clock */
void get_current_time(cuuid_time_t *timestamp) {
    static int              inited = 0;
    static cuuid_time_t     time_last;
    static uint16_t         ids_this_tick;
    cuuid_time_t            time_now;

    if (!inited) {
        get_system_time(&time_now);
        ids_this_tick = UIDS_PER_TICK;
        inited = 1;
    }

    for (;;) {
        get_system_time(&time_now);

        if (time_last != time_now) {
            ids_this_tick = 0;
            time_last = time_now;
            break;
        }

        if (ids_this_tick < UIDS_PER_TICK) {
            ids_this_tick++;
            break;
        }
    }

    *timestamp = time_now + ids_this_tick;
}

/* Get hardware tick count */
static double get_tick_count(void) {
#ifdef __WIN32___
    return GetTickCount();
#else
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now))
        return 0;

    return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
#endif
}

/* Create true random as prescribed by the IEEE */
static uint16_t true_random(void) {
    static int rnd_seed_count = 0;
    cuuid_time_t time_now;

    if (!rnd_seed_count) {
        get_system_time(&time_now);
        time_now = time_now / UIDS_PER_TICK;
        srand((uint32_t)(((time_now >> 32) ^ time_now) & 0xffffffff));
    }

    /* Advance counters */
    rnd_seed_count = (rnd_seed_count == max_rnd_seed) ? 0 : rnd_seed_count + 1;

    return (rand() + get_tick_count());
}

/* Strip special characters from string */
static void strip_special_chars(char *s) {
    char *pr = s, *pw = s;

    while (*pr) {
        *pw = *pr++;
        if ((*pw != '-') &&
            (*pw != '{') &&
            (*pw != '}') &&
            (*pw != ' '))
            pw++;
    }

    *pw = '\0';
}

/* Check if string validates as hex */
static int ishex(char *s) {
    while (*s) {
        if (!isxdigit(*s))
            return 0;
        s++;
    }

    return 1;
}

/* Parse string into QUID */
static void strtoquid(char *str, cuuid_t *u) {
    char octet1[9];
    char octet[5];
    char node[3];

    octet1[8] = '\0';
    octet[4] = '\0';
    node[2] = '\0';

    octet1[0] = str[0];
    octet1[1] = str[1];
    octet1[2] = str[2];
    octet1[3] = str[3];
    octet1[4] = str[4];
    octet1[5] = str[5];
    octet1[6] = str[6];
    octet1[7] = str[7];
    u->time_low = strtol(octet1, NULL, 16);

    octet[0] = str[8];
    octet[1] = str[9];
    octet[2] = str[10];
    octet[3] = str[11];
    u->time_mid = (int)strtol(octet, NULL, 16);

    octet[0] = str[12];
    octet[1] = str[13];
    octet[2] = str[14];
    octet[3] = str[15];
    u->time_hi_and_version = (int)strtol(octet, NULL, 16);

    node[0] = str[16];
    node[1] = str[17];
    u->clock_seq_hi_and_reserved = (char)strtol(node, NULL, 16);

    node[0] = str[18];
    node[1] = str[19];
    u->clock_seq_low = (char)strtol(node, NULL, 16);

    node[0] = str[20];
    node[1] = str[21];
    u->node[0] = (char)strtol(node, NULL, 16);

    node[0] = str[22];
    node[1] = str[23];
    u->node[1] = (char)strtol(node, NULL, 16);

    node[0] = str[24];
    node[1] = str[25];
    u->node[2] = (char)strtol(node, NULL, 16);

    node[0] = str[26];
    node[1] = str[27];
    u->node[3] = (char)strtol(node, NULL, 16);

    node[0] = str[28];
    node[1] = str[29];
    u->node[4] = (char)strtol(node, NULL, 16);

    node[0] = str[30];
    node[1] = str[31];
    u->node[5] = (char)strtol(node, NULL, 16);
}

/* Validate quid as genuine identifier */
int quid_validate(cuuid_t *cuuid) {
    if ((cuuid->time_hi_and_version & VERSION_REV7) == VERSION_REV7)
        cuuid->version = QUID_REV7;
    else if ((cuuid->time_hi_and_version & VERSION_REV4) == VERSION_REV4)
        cuuid->version = QUID_REV4;
    else
        return QUID_ERROR;

    /* In any version, these must be filled */
    if (!cuuid->node[2] || !cuuid->node[5])
        return QUID_ERROR;

    return QUID_OK;
}

/* Convert string to identifier */
int quid_parse(char *quid, cuuid_t *cuuid) {
    int len;

    /* Remove all special characters */
    strip_special_chars(quid);
    len = strlen(quid);

    /* Fail if invalid length */
    if (len != QUID_LEN)
        return QUID_ERROR;

    /* Fail if not hex */
    if (!ishex(quid))
        return QUID_ERROR;

    strtoquid(quid, cuuid);
    if (!quid_validate(cuuid))
       return QUID_ERROR;

    return QUID_OK;
}

/* Convert quid to string */
void quid_tostring(const cuuid_t *u, char str[QUID_FULLLEN + 1]) {
    snprintf(str, QUID_FULLLEN + 1,
        "{%.8lx-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x}",
        u->time_low,
        u->time_mid,
        u->time_hi_and_version,
        u->clock_seq_hi_and_reserved,
        u->clock_seq_low,
        u->node[0],
        u->node[1],
        u->node[2],
        u->node[3],
        u->node[4],
        u->node[5]);
}
