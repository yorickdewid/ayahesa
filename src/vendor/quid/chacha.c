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

#include <stdlib.h>
#include <memory.h>

#include "chacha.h"

/* Basic 32-bit operators */
#define ROTATE(v,c) ((uint32_t)((v) << (c)) | ((v) >> (32 - (c))))
#define XOR(v,w) ((v) ^ (w))
#define ADDITION(v,w) ((uint32_t)((v) + (w)))
#define PLUSONE(v) (ADDITION((v), 1))

/* Little endian machine assumed (x86-64) */
#define U32TO8_LITTLE(p, v) (((uint32_t*)(p))[0] = v)
#define U8TO32_LITTLE(p) (((uint32_t*)(p))[0])

/* ARX */
#define QUARTERROUND(a, b, c, d) \
    x[a] = ADDITION(x[a],x[b]); x[d] = ROTATE(XOR(x[d],x[a]),16); \
    x[c] = ADDITION(x[c],x[d]); x[b] = ROTATE(XOR(x[b],x[c]),12); \
    x[a] = ADDITION(x[a],x[b]); x[d] = ROTATE(XOR(x[d],x[a]), 8); \
    x[c] = ADDITION(x[c],x[d]); x[b] = ROTATE(XOR(x[b],x[c]), 7);

static uint8_t SIGMA[16] = "expand 32-byte k";
static uint8_t TAU[16]   = "expand 16-byte k";

static void doublerounds(uint8_t output[64], const uint32_t input[16], uint8_t rounds) {
    uint32_t x[16];
    int32_t i;

    /* Copy block */
    for (i = 0; i < 16; ++i) {
        x[i] = input[i];
    }

    /* Scramble that shit */
    for (i = rounds ; i > 0 ; i -= 2) {
        /* Vertical vertices */
        QUARTERROUND( 0, 4, 8,12)
        QUARTERROUND( 1, 5, 9,13)
        QUARTERROUND( 2, 6,10,14)
        QUARTERROUND( 3, 7,11,15)

        /* Diagonal vertices */
        QUARTERROUND( 0, 5,10,15)
        QUARTERROUND( 1, 6,11,12)
        QUARTERROUND( 2, 7, 8,13)
        QUARTERROUND( 3, 4, 9,14)

#if defined(Y_EXT)
        /* Horizontal vertices (extension) */
        QUARTERROUND( 0, 1, 2, 3)
        QUARTERROUND( 4, 5, 6, 7)
        QUARTERROUND( 8, 9,10,11)
        QUARTERROUND(12,13,14,15)
#endif
    }

    /* Merge input with scramble to prevent unwinding */
    for (i = 0; i < 16; ++i) {
        x[i] = ADDITION(x[i], input[i]);
    }

    /* Copy to output */
    for (i = 0; i < 16; ++i) {
        U32TO8_LITTLE(output + 4 * i, x[i]);
    }
}

void chacha_init(chacha_ctx *ctx, uint8_t *key, uint32_t keylen, uint8_t *iv, uint32_t counter) {
    switch (keylen) {
        case 256:
            ctx->state[0]  = U8TO32_LITTLE(SIGMA + 0);
            ctx->state[1]  = U8TO32_LITTLE(SIGMA + 4);
            ctx->state[2]  = U8TO32_LITTLE(SIGMA + 8);
            ctx->state[3]  = U8TO32_LITTLE(SIGMA + 12);
            ctx->state[4]  = U8TO32_LITTLE(key + 0);
            ctx->state[5]  = U8TO32_LITTLE(key + 4);
            ctx->state[6]  = U8TO32_LITTLE(key + 8);
            ctx->state[7]  = U8TO32_LITTLE(key + 12);
            ctx->state[8]  = U8TO32_LITTLE(key + 16);
            ctx->state[9]  = U8TO32_LITTLE(key + 20);
            ctx->state[10] = U8TO32_LITTLE(key + 24);
            ctx->state[11] = U8TO32_LITTLE(key + 28);
            break;
        case 128:
            ctx->state[0]  = U8TO32_LITTLE(TAU + 0);
            ctx->state[1]  = U8TO32_LITTLE(TAU + 4);
            ctx->state[2]  = U8TO32_LITTLE(TAU + 8);
            ctx->state[3]  = U8TO32_LITTLE(TAU + 12);
            ctx->state[4]  = U8TO32_LITTLE(key + 0);
            ctx->state[5]  = U8TO32_LITTLE(key + 4);
            ctx->state[6]  = U8TO32_LITTLE(key + 8);
            ctx->state[7]  = U8TO32_LITTLE(key + 12);
            ctx->state[8]  = U8TO32_LITTLE(key + 0);
            ctx->state[9]  = U8TO32_LITTLE(key + 4);
            ctx->state[10] = U8TO32_LITTLE(key + 8);
            ctx->state[11] = U8TO32_LITTLE(key + 12);
            break;
        default:
            abort();
    }

    /* Reset block counter and add IV to state */
    ctx->state[12] = counter;
    ctx->state[13] = 0;
    ctx->state[14] = U8TO32_LITTLE(iv + 0);
    ctx->state[15] = U8TO32_LITTLE(iv + 4);
}

/* Operate like a block cipher, used for testing */
void chacha_next(chacha_ctx *ctx, const uint8_t m[64], uint8_t c[64]) {
    uint8_t x[64];
    unsigned int i;

    /* Update the internal state and increase the block counter */
    doublerounds(x, ctx->state, ctx->rounds);
    ctx->state[12] = PLUSONE(ctx->state[12]);
    if (!ctx->state[12])
        ctx->state[13] = PLUSONE(ctx->state[13]);

    /* XOR the input block with the new temporal state to
     * create the transformed block */
    for (i = 0; i < 64; ++i)
        c[i] = m[i] ^ x[i];
}

void chacha_xor(chacha_ctx *ctx, uint8_t *input, size_t len) {
    uint8_t block[64];
    unsigned int i;

    /* Upper block limit */
    if (len > 64)
        abort();

    /* Update the internal state and increase the block counter */
    doublerounds(block, ctx->state, ctx->rounds);
    ctx->state[12] = PLUSONE(ctx->state[12]);
    if (!ctx->state[12])
        ctx->state[13] = PLUSONE(ctx->state[13]);

    for (i = 0; i < len; ++i)
        input[i] = input[i] ^ block[i];
}

void chacha_init_ctx(chacha_ctx *ctx, uint8_t rounds) {
    /* Not too crazy */
    if (rounds < 2)
        abort();

    memset(ctx->state, '\0', 16);
    ctx->rounds = rounds;
}
