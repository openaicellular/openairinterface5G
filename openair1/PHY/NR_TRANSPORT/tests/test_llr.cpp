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

#include "gtest/gtest.h"
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <numeric>
extern "C" {
void nr_ulsch_16qam_llr(int32_t *rxdataF_comp, int32_t *ul_ch_mag, int16_t *ulsch_llr, uint32_t nb_re, uint8_t symbol);
struct configmodule_interface_s;
struct configmodule_interface_s *uniqCfg = NULL;

void exit_function(const char *file, const char *function, const int line, const char *s, const int assert)
{
  if (assert) {
    abort();
  } else {
    exit(EXIT_SUCCESS);
  }
}
#include "openair1/PHY/TOOLS/tools_defs.h"
}
#include <cstdio>
#include "common/utils/LOG/log.h"
#include <cstdlib>
#include <memory>

constexpr bool is_power_of_two(uint64_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

size_t align_up(size_t a, size_t b)
{
  return (a + b - 1) / b * b;
}

int16_t saturating_sub(int16_t a, int16_t b)
{
  int32_t result = (int32_t)a - (int32_t)b;

  if (result < INT16_MIN) {
    return INT16_MIN;
  } else if (result > INT16_MAX) {
    return INT16_MAX;
  } else {
    return (int16_t)result;
  }
}

// Template adaptations for std::vector. This is needed because the avx functions expect 256 bit alignment.
template <typename T, size_t alignment>
class AlignedAllocator {
 public:
  static_assert(is_power_of_two(alignment), "Alignment should be power of 2");
  static_assert(alignment >= 8, "Alignment must be at least 8 bits");
  using value_type = T;

  AlignedAllocator() = default;

  AlignedAllocator(const AlignedAllocator &) = default;

  AlignedAllocator &operator=(const AlignedAllocator &) = default;

  template <typename U>
  struct rebind {
    using other = AlignedAllocator<U, alignment>;
  };

  T *allocate(size_t n)
  {
    size_t alignment_bytes = alignment / 8;
    void *ptr = ::aligned_alloc(alignment_bytes, align_up(n * sizeof(T), alignment_bytes));
    return static_cast<T *>(ptr);
  }

  void deallocate(T *p, size_t n)
  {
    ::free(p);
  }
};

// Using 512-aligned vector in case some functions use avx-512
template <typename T>
using AlignedAllocator512 = AlignedAllocator<T, 512>;
template <typename T>
using AlignedVector512 = std::vector<T, AlignedAllocator512<T>>;

void nr_ulsch_16qam_llr_ref(c16_t *rxdataF_comp, int32_t *ul_ch_mag, int16_t *ulsch_llr, uint32_t nb_re, uint8_t symbol)
{
  int16_t *rxDataF_i16 = (int16_t *)rxdataF_comp;
  int16_t *ul_ch_mag_i16 = (int16_t *)ul_ch_mag;
  for (auto i = 0U; i < nb_re; i++) {
    int16_t real = rxDataF_i16[2 * i];
    int16_t imag = rxDataF_i16[2 * i + 1];
    int16_t mag_real = ul_ch_mag_i16[2 * i];
    int16_t mag_imag = ul_ch_mag_i16[2 * i];
    ulsch_llr[4 * i] = real;
    ulsch_llr[4 * i + 1] = imag;
    ulsch_llr[4 * i + 2] = saturating_sub(mag_real, std::abs(real));
    ulsch_llr[4 * i + 3] = saturating_sub(mag_imag, std::abs(imag));
  }
}

TEST(test_llr, verify_reference_implementation_qam16)
{
  AlignedVector512<c16_t> rxdataF_comp;
  AlignedVector512<uint16_t> ul_ch_mag;
  AlignedVector512<uint64_t> ulsch_llr;
  uint8_t symbol = 0;
  AlignedVector512<uint32_t> nb_res = {16, 32, 24, 40, 48};
  for (auto i = 0U; i < nb_res.size(); i++) {
    uint32_t nb_re = nb_res[i];
    // Trick to force the vectors to be 512-bit aligned
    rxdataF_comp.resize(nb_re);
    std::fill(rxdataF_comp.begin(), rxdataF_comp.end(), (c16_t){(int16_t)rand(), (int16_t)rand()});
    ul_ch_mag.resize(nb_re * 2);
    std::fill(ul_ch_mag.begin(), ul_ch_mag.end(), (int16_t)rand());

    AlignedVector512<uint64_t> ulsch_llr_ref;
    ulsch_llr_ref.resize(nb_re);
    std::fill(ulsch_llr_ref.begin(), ulsch_llr_ref.end(), 0);
    nr_ulsch_16qam_llr_ref((c16_t *)rxdataF_comp.data(),
                           (int32_t *)ul_ch_mag.data(),
                           (int16_t *)ulsch_llr_ref.data(),
                           nb_re,
                           symbol);

    ulsch_llr.resize(nb_re);
    std::fill(ulsch_llr.begin(), ulsch_llr.end(), 0);
    nr_ulsch_16qam_llr((int32_t *)rxdataF_comp.data(), (int32_t *)ul_ch_mag.data(), (int16_t *)ulsch_llr.data(), nb_re, symbol);

    for (auto i = 0U; i < nb_re; i++) {
      EXPECT_EQ(ulsch_llr_ref[i], ulsch_llr[i])
          << "REF function error ref " << std::hex << ulsch_llr_ref[i] << " != dut " << ulsch_llr[i];
    }
  }
}


TEST(test_llr, test_8_res)
{
  AlignedVector512<c16_t> rxdataF_comp;
  AlignedVector512<uint16_t> ul_ch_mag;
  AlignedVector512<uint64_t> ulsch_llr;
  uint8_t symbol = 0;
  AlignedVector512<uint32_t> nb_res = {8};
  for (auto i = 0U; i < nb_res.size(); i++) {
    uint32_t nb_re = nb_res[i];
    rxdataF_comp.resize(nb_re);
    std::fill(rxdataF_comp.begin(), rxdataF_comp.end(), (c16_t){(int16_t)rand(), (int16_t)rand()});
    ul_ch_mag.resize(nb_re * 2);
    std::fill(ul_ch_mag.begin(), ul_ch_mag.end(), (int16_t)rand());

    AlignedVector512<uint64_t> ulsch_llr_ref;
    ulsch_llr_ref.resize(nb_re);
    std::fill(ulsch_llr_ref.begin(), ulsch_llr_ref.end(), 0);
    nr_ulsch_16qam_llr_ref((c16_t *)rxdataF_comp.data(),
                           (int32_t *)ul_ch_mag.data(),
                           (int16_t *)ulsch_llr_ref.data(),
                           nb_re,
                           symbol);

    ulsch_llr.resize(nb_re);
    std::fill(ulsch_llr.begin(), ulsch_llr.end(), 0);
    nr_ulsch_16qam_llr((int32_t *)rxdataF_comp.data(), (int32_t *)ul_ch_mag.data(), (int16_t *)ulsch_llr.data(), nb_re, symbol);

    for (auto i = 0U; i < nb_re; i++) {
      EXPECT_EQ(ulsch_llr_ref[i], ulsch_llr[i])
          << "REF function error ref " << std::hex << ulsch_llr_ref[i] << " != dut " << ulsch_llr[i];
    }
  }
}

// This is a "normal" segfault because the function assumed extra buffer for reading non-existent REs
TEST(test_llr, no_segmentation_fault_at_12_res)
{
  AlignedVector512<c16_t> rxdataF_comp;
  AlignedVector512<uint16_t> ul_ch_mag;
  AlignedVector512<uint64_t> ulsch_llr;
  uint8_t symbol = 0;
  AlignedVector512<uint32_t> nb_res = {12};
  for (auto i = 0U; i < nb_res.size(); i++) {
    uint32_t nb_re = nb_res[i];
    rxdataF_comp.resize(nb_re);
    std::fill(rxdataF_comp.begin(), rxdataF_comp.end(), (c16_t){(int16_t)rand(), (int16_t)rand()});
    ul_ch_mag.resize(nb_re * 2);
    std::fill(ul_ch_mag.begin(), ul_ch_mag.end(), (int16_t)rand());

    AlignedVector512<uint64_t> ulsch_llr_ref;
    ulsch_llr_ref.resize(nb_re);
    std::fill(ulsch_llr_ref.begin(), ulsch_llr_ref.end(), 0);
    nr_ulsch_16qam_llr_ref((c16_t *)rxdataF_comp.data(),
                           (int32_t *)ul_ch_mag.data(),
                           (int16_t *)ulsch_llr_ref.data(),
                           nb_re,
                           symbol);

    ulsch_llr.resize(nb_re);
    std::fill(ulsch_llr.begin(), ulsch_llr.end(), 0);
    nr_ulsch_16qam_llr((int32_t *)rxdataF_comp.data(), (int32_t *)ul_ch_mag.data(), (int16_t *)ulsch_llr.data(), nb_re, symbol);

    for (auto i = 0U; i < nb_re; i++) {
      EXPECT_EQ(ulsch_llr_ref[i], ulsch_llr[i])
          << "REF function error ref " << std::hex << ulsch_llr_ref[i] << " != dut " << ulsch_llr[i];
    }
  }
}

// This is a "normal" segfault because the function assumed extra buffer for reading non-existent REs
TEST(test_llr, no_segmentation_fault_at_36_res)
{
  AlignedVector512<c16_t> rxdataF_comp;
  AlignedVector512<uint16_t> ul_ch_mag;
  AlignedVector512<uint64_t> ulsch_llr;
  uint8_t symbol = 0;
  AlignedVector512<uint32_t> nb_res = {36};
  for (auto i = 0U; i < nb_res.size(); i++) {
    uint32_t nb_re = nb_res[i];
    rxdataF_comp.resize(nb_re);
    std::fill(rxdataF_comp.begin(), rxdataF_comp.end(), (c16_t){(int16_t)rand(), (int16_t)rand()});
    ul_ch_mag.resize(nb_re * 2);
    std::fill(ul_ch_mag.begin(), ul_ch_mag.end(), (int16_t)rand());

    AlignedVector512<uint64_t> ulsch_llr_ref;
    ulsch_llr_ref.resize(nb_re);
    std::fill(ulsch_llr_ref.begin(), ulsch_llr_ref.end(), 0);
    nr_ulsch_16qam_llr_ref((c16_t *)rxdataF_comp.data(),
                           (int32_t *)ul_ch_mag.data(),
                           (int16_t *)ulsch_llr_ref.data(),
                           nb_re,
                           symbol);

    ulsch_llr.resize(nb_re);
    std::fill(ulsch_llr.begin(), ulsch_llr.end(), 0);
    nr_ulsch_16qam_llr((int32_t *)rxdataF_comp.data(), (int32_t *)ul_ch_mag.data(), (int16_t *)ulsch_llr.data(), nb_re, symbol);

    for (auto i = 0U; i < nb_re; i++) {
      EXPECT_EQ(ulsch_llr_ref[i], ulsch_llr[i])
          << "REF function error ref " << std::hex << ulsch_llr_ref[i] << " != dut " << ulsch_llr[i];
    }
  }
}

// any number of REs should work
TEST(test_llr, no_segfault_any_number_of_re)
{
  AlignedVector512<c16_t> rxdataF_comp;
  AlignedVector512<uint16_t> ul_ch_mag;
  AlignedVector512<uint64_t> ulsch_llr;
  uint8_t symbol = 0;
  AlignedVector512<uint32_t> nb_res;
  nb_res.resize(1000);
  uint32_t i = 1;
  std::fill(nb_res.begin(), nb_res.end(), i++);
  for (auto i = 0U; i < nb_res.size(); i++) {
    uint32_t nb_re = nb_res[i];
    rxdataF_comp.resize(nb_re);
    std::fill(rxdataF_comp.begin(), rxdataF_comp.end(), (c16_t){(int16_t)rand(), (int16_t)rand()});
    ul_ch_mag.resize(nb_re * 2);
    std::fill(ul_ch_mag.begin(), ul_ch_mag.end(), (int16_t)rand());

    AlignedVector512<uint64_t> ulsch_llr_ref;
    ulsch_llr_ref.resize(nb_re);
    std::fill(ulsch_llr_ref.begin(), ulsch_llr_ref.end(), 0);
    nr_ulsch_16qam_llr_ref((c16_t *)rxdataF_comp.data(),
                           (int32_t *)ul_ch_mag.data(),
                           (int16_t *)ulsch_llr_ref.data(),
                           nb_re,
                           symbol);

    ulsch_llr.resize(nb_re);
    std::fill(ulsch_llr.begin(), ulsch_llr.end(), 0);
    nr_ulsch_16qam_llr((int32_t *)rxdataF_comp.data(), (int32_t *)ul_ch_mag.data(), (int16_t *)ulsch_llr.data(), nb_re, symbol);

    for (auto i = 0U; i < nb_re; i++) {
      EXPECT_EQ(ulsch_llr_ref[i], ulsch_llr[i])
          << "REF function error ref " << std::hex << ulsch_llr_ref[i] << " != dut " << ulsch_llr[i];
    }
  }
}

int main(int argc, char **argv)
{
  logInit();
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
