// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France


#ifndef __LOVE_REAL_H
#define __LOVE_REAL_H
/*-----------------------------------*
 *                LOVE               *
 *-----------------------------------*/
/*!
\brief  (component Interface.Outils)
\author Thierry Carrard
\date 4 Decembre 2007

Modifications history :
*/

// par defaut, on est en double
#ifndef REAL_PRECISION
#define REAL_PRECISION 64
#endif

// float = precision la plus basse
#if ( REAL_PRECISION <= 32 )

#define REAL  float
#define REAL2 float2
#define REAL3 float3
#define REAL4 float4

#define make_REAL1 make_float1
#define make_REAL2 make_float2
#define make_REAL3 make_float3
#define make_REAL4 make_float4

#define SQRT sqrtf
#define FABS fabsf
#define REAL_CONST(x) ((float)(x)) //( x##f )


// long double = precision la plus haute
#elif ( REAL_PRECISION > 64 )

#define REAL  long double
#define REAL2 ldouble2
#define REAL3 ldouble3
#define REAL4 ldouble4

#define make_REAL1 make_ldouble1
#define make_REAL2 make_ldouble2
#define make_REAL3 make_ldouble3
#define make_REAL4 make_ldouble4

#define SQRT  sqrtl
#define FABS  fabsl

#define REAL_CONST(x) ((long double)(x)) //( x##l )


// double = precision par defaut
#else

#define REAL  double
#define REAL2 double2
#define REAL3 double3
#define REAL4 double4

#define make_REAL1 make_double1
#define make_REAL2 make_double2
#define make_REAL3 make_double3
#define make_REAL4 make_double4

#define SQRT  sqrt
#define FABS  fabs

#define REAL_CONST(x) x

#endif

#endif /* __LOVE_REAL_H */

