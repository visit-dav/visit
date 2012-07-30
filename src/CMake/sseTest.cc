#include <emmintrin.h>

static __m128 float_sse_var = _mm_set_ps1(1.f);
static __m128i int_sse_var  = _mm_set1_epi32(2);

#ifdef MANTA_TEST_GCC
static __m128i gcc_guy = _mm_set1_epi64x( (long long)1 );
#endif

#ifdef MANTA_TEST_CAST
static __m128i cast_float_to_int = _mm_castps_si128(float_sse_var);
static __m128  cast_int_to_float = _mm_castsi128_ps(cast_float_to_int);
static __m128d cast_float_to_double = _mm_castps_pd(float_sse_var);
static __m128  cast_double_to_float = _mm_castpd_ps(cast_float_to_double);
#endif

int main()
{
  return 0;
}


