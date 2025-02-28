#pragma once

#if defined(ZIMG_X86) or defined(ZIMG_ARM)

#ifndef ZIMG_X86_AVX2_UTIL_H_
#define ZIMG_X86_AVX2_UTIL_H_

#include "common/ccdep.h"
#include "x86util.h"

namespace zimg {

namespace _avx2 {

// Transpose two 8x8 matrices stored in the lower and upper 128-bit lanes of [row0]-[row7].
static inline FORCE_INLINE void mm256_transpose8_x2_epi16(__m256i &row0, __m256i &row1, __m256i &row2, __m256i &row3,
                                                          __m256i &row4, __m256i &row5, __m256i &row6, __m256i &row7)
{
	__m256i t0, t1, t2, t3, t4, t5, t6, t7;
	__m256i tt0, tt1, tt2, tt3, tt4, tt5, tt6, tt7;

	t0 = _mm256_unpacklo_epi16(row0, row1);
	t1 = _mm256_unpacklo_epi16(row2, row3);
	t2 = _mm256_unpacklo_epi16(row4, row5);
	t3 = _mm256_unpacklo_epi16(row6, row7);
	t4 = _mm256_unpackhi_epi16(row0, row1);
	t5 = _mm256_unpackhi_epi16(row2, row3);
	t6 = _mm256_unpackhi_epi16(row4, row5);
	t7 = _mm256_unpackhi_epi16(row6, row7);

	tt0 = _mm256_unpacklo_epi32(t0, t1);
	tt1 = _mm256_unpackhi_epi32(t0, t1);
	tt2 = _mm256_unpacklo_epi32(t2, t3);
	tt3 = _mm256_unpackhi_epi32(t2, t3);
	tt4 = _mm256_unpacklo_epi32(t4, t5);
	tt5 = _mm256_unpackhi_epi32(t4, t5);
	tt6 = _mm256_unpacklo_epi32(t6, t7);
	tt7 = _mm256_unpackhi_epi32(t6, t7);

	row0 = _mm256_unpacklo_epi64(tt0, tt2);
	row1 = _mm256_unpackhi_epi64(tt0, tt2);
	row2 = _mm256_unpacklo_epi64(tt1, tt3);
	row3 = _mm256_unpackhi_epi64(tt1, tt3);
	row4 = _mm256_unpacklo_epi64(tt4, tt6);
	row5 = _mm256_unpackhi_epi64(tt4, tt6);
	row6 = _mm256_unpacklo_epi64(tt5, tt7);
	row7 = _mm256_unpackhi_epi64(tt5, tt7);
}

// Exchange the upper 128-bit lane of [row0] with the lower 128-bit lane of [row1].
static inline FORCE_INLINE void mm256_exchange_lanes_si128(__m256i &row0, __m256i &row1)
{
	__m256i tmp0 = _mm256_permute2f128_si256(row0, row1, 0x20);
	__m256i tmp1 = _mm256_permute2f128_si256(row0, row1, 0x31);
	row0 = tmp0;
	row1 = tmp1;
}

} // namespace _avx2


// Store from [x] into [dst] the 8-bit elements with index less than [idx].
static inline FORCE_INLINE void mm_store_idxlo_epi8(__m128i *dst, __m128i x, unsigned idx)
{
	__m128i orig = _mm_load_si128(dst);
	__m128i mask = _mm_load_si128((const __m128i *)(&xmm_mask_table[idx]));

	x = _mm_blendv_epi8(orig, x, mask);
	_mm_store_si128(dst, x);
}

// Store from [x] into [dst] the 8-bit elements with index greater than or equal to [idx].
static inline FORCE_INLINE void mm_store_idxhi_epi8(__m128i *dst, __m128i x, unsigned idx)
{
	__m128i orig = _mm_load_si128(dst);
	__m128i mask = _mm_load_si128((const __m128i *)(&ymm_mask_table[idx]));

	x = _mm_blendv_epi8(x, orig, mask);
	_mm_store_si128(dst, x);
}

// Store from [x] into [dst] the 16-bit elements with index less than [idx].
static inline FORCE_INLINE void mm_store_idxlo_epi16(__m128i *dst, __m128i x, unsigned idx)
{
	mm_store_idxlo_epi8(dst, x, idx * 2);
}

// Store from [x] into [dst] the 16-bit elements with index greater than or equal to [idx].
static inline FORCE_INLINE void mm_store_idxhi_epi16(__m128i *dst, __m128i x, unsigned idx)
{
	mm_store_idxhi_epi8(dst, x, idx * 2);
}

// Store from [x] into [dst] the 32-bit elements with index less than [idx].
static inline FORCE_INLINE void mm_store_idxlo_ps(float *dst, __m128 x, unsigned idx)
{
	__m128i mask = _mm_load_si128((const __m128i *)(&xmm_mask_table[idx * 4]));
	_mm_maskstore_ps(dst, mask, x);
}

// Store from [x] into [dst] the 32-bit elements with index greater than or equal to [idx]
static inline FORCE_INLINE void mm_store_idxhi_ps(float *dst, __m128 x, unsigned idx)
{
	__m128i mask = _mm_load_si128((const __m128i *)(&xmm_mask_table[idx * 4]));
	mask = _mm_castps_si128(_mm_xor_ps(_mm_castsi128_ps(mask), _mm_castsi128_ps(_mm_set1_epi32(-1))));
	_mm_maskstore_ps(dst, mask, x);
}

// Stores the elements of [x] into [dst0]-[dst7].
static inline FORCE_INLINE void mm_scatter_epi16(uint16_t *dst0, uint16_t *dst1, uint16_t *dst2, uint16_t *dst3,
                                                 uint16_t *dst4, uint16_t *dst5, uint16_t *dst6, uint16_t *dst7, __m128i x)
{
	*dst0 = _mm_extract_epi16(x, 0);
	*dst1 = _mm_extract_epi16(x, 1);
	*dst2 = _mm_extract_epi16(x, 2);
	*dst3 = _mm_extract_epi16(x, 3);
	*dst4 = _mm_extract_epi16(x, 4);
	*dst5 = _mm_extract_epi16(x, 5);
	*dst6 = _mm_extract_epi16(x, 6);
	*dst7 = _mm_extract_epi16(x, 7);
}

// Stores the elements of [x] into [dst0]-[dst3].
static inline FORCE_INLINE void mm_scatter_ps(float *dst0, float *dst1, float *dst2, float *dst3, __m128 x)
{
	_mm_store_ss(dst0, x);
	*(uint32_t *)dst1 = _mm_extract_ps(x, 1);
	*(uint32_t *)dst2 = _mm_extract_ps(x, 2);
	*(uint32_t *)dst3 = _mm_extract_ps(x, 3);
}

// Store from [x] into [dst] the 8-bit elements with index less than [idx].
static inline FORCE_INLINE void mm256_store_idxlo_epi8(__m256i *dst, __m256i x, unsigned idx)
{
	__m256i orig = _mm256_load_si256(dst);
	__m256i mask = _mm256_load_si256((const __m256i *)(&ymm_mask_table[idx]));

	x = _mm256_blendv_epi8(orig, x, mask);
	_mm256_store_si256(dst, x);
}

// Store from [x] into [dst] the 8-bit elements with index greater than or equal to [idx].
static inline FORCE_INLINE void mm256_store_idxhi_epi8(__m256i *dst, __m256i x, unsigned idx)
{
	__m256i orig = _mm256_load_si256(dst);
	__m256i mask = _mm256_load_si256((const __m256i *)(&ymm_mask_table[idx]));

	x = _mm256_blendv_epi8(x, orig, mask);
	_mm256_store_si256(dst, x);
}

// Store from [x] into [dst] the 16-bit elements with index less than [idx].
static inline FORCE_INLINE void mm256_store_idxlo_epi16(__m256i *dst, __m256i x, unsigned idx)
{
	mm256_store_idxlo_epi8(dst, x, idx * 2);
}

// Store from [x] into [dst] the 16-bit elements with index greater than or equal to [idx].
static inline FORCE_INLINE void mm256_store_idxhi_epi16(__m256i *dst, __m256i x, unsigned idx)
{
	mm256_store_idxhi_epi8(dst, x, idx * 2);
}

// Store from [x] into [dst] the 32-bit elements with index less than [idx].
static inline FORCE_INLINE void mm256_store_idxlo_ps(float *dst, __m256 x, unsigned idx)
{
	__m256i mask = _mm256_load_si256((const __m256i *)(&ymm_mask_table[idx * 4]));
	_mm256_maskstore_ps(dst, mask, x);
}

// Store from [x] into [dst] the 32-bit elements with index greater than or equal to [idx].
static inline FORCE_INLINE void mm256_store_idxhi_ps(float *dst, __m256 x, unsigned idx)
{
	__m256i mask = _mm256_load_si256((const __m256i *)(&ymm_mask_table[idx * 4]));
	mask = _mm256_castps_si256(_mm256_xor_ps(_mm256_castsi256_ps(mask), _mm256_castsi256_ps(_mm256_set1_epi32(-1))));
	_mm256_maskstore_ps(dst, mask, x);
}

// Transpose in-place the 8x8 matrix stored in [row0]-[row7].
static inline FORCE_INLINE void mm_transpose8_epi16(__m128i &row0, __m128i &row1, __m128i &row2, __m128i &row3,
                                                    __m128i &row4, __m128i &row5, __m128i &row6, __m128i &row7)
{
	__m128i t0, t1, t2, t3, t4, t5, t6, t7;
	__m128i tt0, tt1, tt2, tt3, tt4, tt5, tt6, tt7;

	t0 = _mm_unpacklo_epi16(row0, row1);
	t1 = _mm_unpacklo_epi16(row2, row3);
	t2 = _mm_unpacklo_epi16(row4, row5);
	t3 = _mm_unpacklo_epi16(row6, row7);
	t4 = _mm_unpackhi_epi16(row0, row1);
	t5 = _mm_unpackhi_epi16(row2, row3);
	t6 = _mm_unpackhi_epi16(row4, row5);
	t7 = _mm_unpackhi_epi16(row6, row7);

	tt0 = _mm_unpacklo_epi32(t0, t1);
	tt1 = _mm_unpackhi_epi32(t0, t1);
	tt2 = _mm_unpacklo_epi32(t2, t3);
	tt3 = _mm_unpackhi_epi32(t2, t3);
	tt4 = _mm_unpacklo_epi32(t4, t5);
	tt5 = _mm_unpackhi_epi32(t4, t5);
	tt6 = _mm_unpacklo_epi32(t6, t7);
	tt7 = _mm_unpackhi_epi32(t6, t7);

	row0 = _mm_unpacklo_epi64(tt0, tt2);
	row1 = _mm_unpackhi_epi64(tt0, tt2);
	row2 = _mm_unpacklo_epi64(tt1, tt3);
	row3 = _mm_unpackhi_epi64(tt1, tt3);
	row4 = _mm_unpacklo_epi64(tt4, tt6);
	row5 = _mm_unpackhi_epi64(tt4, tt6);
	row6 = _mm_unpacklo_epi64(tt5, tt7);
	row7 = _mm_unpackhi_epi64(tt5, tt7);
}

// Transpose in-place the 16x16 matrix stored in [row0]-[row15].
static inline FORCE_INLINE void mm256_transpose16_epi16(__m256i &row0, __m256i &row1, __m256i &row2, __m256i &row3,
                                                        __m256i &row4, __m256i &row5, __m256i &row6, __m256i &row7,
                                                        __m256i &row8, __m256i &row9, __m256i &row10, __m256i &row11,
                                                        __m256i &row12, __m256i &row13, __m256i &row14, __m256i &row15)
{
	_avx2::mm256_transpose8_x2_epi16(row0, row1, row2, row3, row4, row5, row6, row7);
	_avx2::mm256_transpose8_x2_epi16(row8, row9, row10, row11, row12, row13, row14, row15);

	_avx2::mm256_exchange_lanes_si128(row0, row8);
	_avx2::mm256_exchange_lanes_si128(row1, row9);
	_avx2::mm256_exchange_lanes_si128(row2, row10);
	_avx2::mm256_exchange_lanes_si128(row3, row11);
	_avx2::mm256_exchange_lanes_si128(row4, row12);
	_avx2::mm256_exchange_lanes_si128(row5, row13);
	_avx2::mm256_exchange_lanes_si128(row6, row14);
	_avx2::mm256_exchange_lanes_si128(row7, row15);
}

// Transpose in-place the 8x8 matrix stored in [row0]-[row7].
static inline FORCE_INLINE void mm256_transpose8_ps(__m256 &row0, __m256 &row1, __m256 &row2, __m256 &row3, __m256 &row4, __m256 &row5, __m256 &row6, __m256 &row7)
{
	__m256 t0, t1, t2, t3, t4, t5, t6, t7;
	__m256 tt0, tt1, tt2, tt3, tt4, tt5, tt6, tt7;

	t0 = _mm256_unpacklo_ps(row0, row1);
	t1 = _mm256_unpackhi_ps(row0, row1);
	t2 = _mm256_unpacklo_ps(row2, row3);
	t3 = _mm256_unpackhi_ps(row2, row3);
	t4 = _mm256_unpacklo_ps(row4, row5);
	t5 = _mm256_unpackhi_ps(row4, row5);
	t6 = _mm256_unpacklo_ps(row6, row7);
	t7 = _mm256_unpackhi_ps(row6, row7);

	tt0 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(1, 0, 1, 0));
	tt1 = _mm256_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 2, 3, 2));
	tt2 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(1, 0, 1, 0));
	tt3 = _mm256_shuffle_ps(t1, t3, _MM_SHUFFLE(3, 2, 3, 2));
	tt4 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(1, 0, 1, 0));
	tt5 = _mm256_shuffle_ps(t4, t6, _MM_SHUFFLE(3, 2, 3, 2));
	tt6 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(1, 0, 1, 0));
	tt7 = _mm256_shuffle_ps(t5, t7, _MM_SHUFFLE(3, 2, 3, 2));

	row0 = _mm256_permute2f128_ps(tt0, tt4, 0x20);
	row1 = _mm256_permute2f128_ps(tt1, tt5, 0x20);
	row2 = _mm256_permute2f128_ps(tt2, tt6, 0x20);
	row3 = _mm256_permute2f128_ps(tt3, tt7, 0x20);
	row4 = _mm256_permute2f128_ps(tt0, tt4, 0x31);
	row5 = _mm256_permute2f128_ps(tt1, tt5, 0x31);
	row6 = _mm256_permute2f128_ps(tt2, tt6, 0x31);
	row7 = _mm256_permute2f128_ps(tt3, tt7, 0x31);
}

} // namespace zimg

#endif // ZIMG_X86_AVX2_UTIL_H_

#endif // ZIMG_X86
