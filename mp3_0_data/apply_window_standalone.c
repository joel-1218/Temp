/*
 * Copyright (c) 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

static inline float round_sample(float *sum)
{
    float sum1=*sum;
    *sum = 0;
    return sum1;
}

#define MACS(rt, ra, rb) rt+=(ra)*(rb)
#define MULS(ra, rb) ((ra)*(rb))
#define MULH3(x, y, s) ((s)*(y)*(x))
#define MLSS(rt, ra, rb) rt-=(ra)*(rb)
#define MULLx(x, y, s) ((y)*(x))
#define FIXHR(x)        ((float)(x))
#define FIXR(x)        ((float)(x))
#define SHR(a,b)       ((a)*(1.0f/(1<<(b))))

/** Window for MDCT. Actually only the elements in [0,17] and
    [MDCT_BUF_SIZE/2, MDCT_BUF_SIZE/2 + 17] are actually used. The rest
    is just to preserve alignment for SIMD implementations.
*/
#define SUM8(op, sum, w, p)               \
{                                         \
    op(sum, (w)[0 * 64], (p)[0 * 64]);    \
    op(sum, (w)[1 * 64], (p)[1 * 64]);    \
    op(sum, (w)[2 * 64], (p)[2 * 64]);    \
    op(sum, (w)[3 * 64], (p)[3 * 64]);    \
    op(sum, (w)[4 * 64], (p)[4 * 64]);    \
    op(sum, (w)[5 * 64], (p)[5 * 64]);    \
    op(sum, (w)[6 * 64], (p)[6 * 64]);    \
    op(sum, (w)[7 * 64], (p)[7 * 64]);    \
}

#define SUM8P2(sum1, op1, sum2, op2, w1, w2, p) \
{                                               \
    float tmp;\
    tmp = p[0 * 64];\
    op1(sum1, (w1)[0 * 64], tmp);\
    op2(sum2, (w2)[0 * 64], tmp);\
    tmp = p[1 * 64];\
    op1(sum1, (w1)[1 * 64], tmp);\
    op2(sum2, (w2)[1 * 64], tmp);\
    tmp = p[2 * 64];\
    op1(sum1, (w1)[2 * 64], tmp);\
    op2(sum2, (w2)[2 * 64], tmp);\
    tmp = p[3 * 64];\
    op1(sum1, (w1)[3 * 64], tmp);\
    op2(sum2, (w2)[3 * 64], tmp);\
    tmp = p[4 * 64];\
    op1(sum1, (w1)[4 * 64], tmp);\
    op2(sum2, (w2)[4 * 64], tmp);\
    tmp = p[5 * 64];\
    op1(sum1, (w1)[5 * 64], tmp);\
    op2(sum2, (w2)[5 * 64], tmp);\
    tmp = p[6 * 64];\
    op1(sum1, (w1)[6 * 64], tmp);\
    op2(sum2, (w2)[6 * 64], tmp);\
    tmp = p[7 * 64];\
    op1(sum1, (w1)[7 * 64], tmp);\
    op2(sum2, (w2)[7 * 64], tmp);\
}

void ff_mpadsp_apply_window_float(float *synth_buf, float *window,
                                  int *dither_state, float *samples,
                                  ptrdiff_t incr)
{
    register const float *w, *w2, *p;
    int j;
    float *samples2;
    float sum, sum2;

    /* copy to avoid wrap */
    memcpy(synth_buf + 512, synth_buf, 32 * sizeof(*synth_buf));

    samples2 = samples + 31 * incr;
    w = window;
    w2 = window + 31;

    sum = *dither_state;
    p = synth_buf + 16;
    SUM8(MACS, sum, w, p);
    p = synth_buf + 48;
    SUM8(MLSS, sum, w + 32, p);
    *samples = round_sample(&sum);
    samples += incr;
    w++;

    /* we calculate two samples at the same time to avoid one memory
       access per two sample */
    for(j=1;j<16;j++) {
        sum2 = 0;
        p = synth_buf + 16 + j;
        SUM8P2(sum, MACS, sum2, MLSS, w, w2, p);
        p = synth_buf + 48 - j;
        SUM8P2(sum, MLSS, sum2, MLSS, w + 32, w2 + 32, p);

        *samples = round_sample(&sum);
        samples += incr;
        sum += sum2;
        *samples2 = round_sample(&sum);
        samples2 -= incr;
        w++;
        w2--;
    }

    p = synth_buf + 32;
    SUM8(MLSS, sum, w + 32, p);
    *samples = round_sample(&sum);
    *dither_state= sum;
}
