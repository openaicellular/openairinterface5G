/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "PHY/defs_common.h"
#include "tools_defs.h"
#include <stdio.h>

static const int16_t conjug[8] __attribute__((aligned(16))) = {-1, 1, -1, 1, -1, 1, -1, 1};
static const int16_t conjug2[8] __attribute__((aligned(16))) = {1, -1, 1, -1, 1, -1, 1, -1};

static void mult_cpx_conj_vector_core(int16_t *x1, int16_t *x2, int16_t *y, uint32_t N, int output_shift, bool madd)
{
  // Multiply elementwise the complex conjugate of x1 with x2.
  // x1       - input 1    in the format  |Re0 Im0 Re1 Im1|,......,|Re(N-2)  Im(N-2) Re(N-1) Im(N-1)|
  //            We assume x1 with a dinamic of 15 bit maximum
  //
  // x2       - input 2    in the format  |Re0 Im0 Re1 Im1|,......,|Re(N-2)  Im(N-2) Re(N-1) Im(N-1)|
  //            We assume x2 with a dinamic of 14 bit maximum
  ///
  // y        - output     in the format  |Re0 Im0 Re1 Im1|,......,|Re(N-2)  Im(N-2) Re(N-1) Im(N-1)|
  //
  // N        - the size f the vectors (this function does N cpx mpy. WARNING: N>=4;
  //
  // output_shift  - shift to be applied to generate output
  //
  // madd - add the output to y

  DevAssert(N % 4 == 0);
  const simde__m128i *x1_128 = (simde__m128i *)x1;
  const simde__m128i *x2_128 = (simde__m128i *)x2;
  simde__m128i *y_128 = (simde__m128i *)y;
  // we compute 4 cpx multiply for each loop
  for (unsigned int i = 0; i < N / 4; i++) {
    simde__m128i tmp_re, tmp_im, tmpy0, tmpy1;
    tmp_re = simde_mm_madd_epi16(*x1_128,*x2_128);
    tmp_im = simde_mm_shufflelo_epi16(*x1_128, SIMDE_MM_SHUFFLE(2,3,0,1));
    tmp_im = simde_mm_shufflehi_epi16(tmp_im, SIMDE_MM_SHUFFLE(2,3,0,1));
    tmp_im = simde_mm_sign_epi16(tmp_im,*(simde__m128i*)&conjug[0]);
    tmp_im = simde_mm_madd_epi16(tmp_im,*x2_128);
    tmp_re = simde_mm_srai_epi32(tmp_re,output_shift);
    tmp_im = simde_mm_srai_epi32(tmp_im,output_shift);
    tmpy0  = simde_mm_unpacklo_epi32(tmp_re,tmp_im);
    tmpy1  = simde_mm_unpackhi_epi32(tmp_re,tmp_im);
    if (madd)
      *y_128 = simde_mm_adds_epi16(*y_128, simde_mm_packs_epi32(tmpy0, tmpy1));
    else
      *y_128 = simde_mm_packs_epi32(tmpy0, tmpy1);
    x1_128++;
    x2_128++;
    y_128++;
  }
}

void mult_cpx_conj_vector(int16_t *x1, int16_t *x2, int16_t *y, uint32_t N, int output_shift)
{
  mult_cpx_conj_vector_core(x1, x2, y, N, output_shift, false);
}

void multadd_cpx_conj_vector(int16_t *x1, int16_t *x2, int16_t *y, uint32_t N, int output_shift)
{
  mult_cpx_conj_vector_core(x1, x2, y, N, output_shift, true);
}

static void mult_cpx_vector_core(const c16_t *x1,
                                 const c16_t *x2,
                                 c16_t *y,
                                 const uint32_t N,
                                 const int output_shift,
                                 const bool madd)
{
  // Multiply elementwise the complex values of x1 with x2.
  // Add the result to y.
  // x1       - input 1    in the format  |Re0 Im0 Re1 Im1|,......,|Re(N-2)  Im(N-2) Re(N-1) Im(N-1)|
  //            We assume x1 with a dinamic of 15 bit maximum
  //
  // x2       - input 2    in the format  |Re0 Im0 Re1 Im1|,......,|Re(N-2)  Im(N-2) Re(N-1) Im(N-1)|
  //            We assume x2 with a dinamic of 14 bit maximum
  ///
  // y        - output     in the format  |Re0 Im0 Re1 Im1|,......,|Re(N-2)  Im(N-2) Re(N-1) Im(N-1)|
  //
  // N        - the size f the vectors (this function does N cpx mpy. WARNING: N>=4;
  //
  // output_shift  - shift to be applied to generate output
  //
  // madd - add the output to y
  DevAssert(N % 4 == 0);
  const simde__m128i *x1_128 = (simde__m128i *)x1;
  const simde__m128i *x2_128 = (simde__m128i *)x2;
  simde__m128i *y_128 = (simde__m128i *)y;
  for (unsigned int i = 0; i < N / 4; i++) {
    simde__m128i tmp_re, tmp_im, tmpy0, tmpy1;
    tmp_re = simde_mm_sign_epi16(*x1_128, *(simde__m128i *)conjug2);
    tmp_re = simde_mm_madd_epi16(tmp_re, *x2_128);
    tmp_im = simde_mm_shufflelo_epi16(*x1_128, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
    tmp_im = simde_mm_shufflehi_epi16(tmp_im, SIMDE_MM_SHUFFLE(2, 3, 0, 1));
    tmp_im = simde_mm_madd_epi16(tmp_im, *x2_128);
    tmp_re = simde_mm_srai_epi32(tmp_re, output_shift);
    tmp_im = simde_mm_srai_epi32(tmp_im, output_shift);
    tmpy0 = simde_mm_unpacklo_epi32(tmp_re, tmp_im);
    tmpy1 = simde_mm_unpackhi_epi32(tmp_re, tmp_im);
    if (madd)
      *y_128 = simde_mm_adds_epi16(*y_128, simde_mm_packs_epi32(tmpy0, tmpy1));
    else
      *y_128 = simde_mm_packs_epi32(tmpy0, tmpy1);
    x1_128++;
    x2_128++;
    y_128++;
  }
}

void mult_cpx_vector(const c16_t *x1, const c16_t *x2, c16_t *y, const uint32_t N, const int output_shift)
{
  mult_cpx_vector_core(x1, x2, y, N, output_shift, false);
}

void multadd_cpx_vector(const c16_t *x1, const c16_t *x2, c16_t *y, const uint32_t N, const int output_shift)
{
  mult_cpx_vector_core(x1, x2, y, N, output_shift, true);
}
