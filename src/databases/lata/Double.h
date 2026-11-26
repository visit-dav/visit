/****************************************************************************
* Copyright (c) 2025, CEA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#ifndef DOUBLE_H
#define DOUBLE_H

#ifndef LATATOOLS
#include <blas1.h>
#endif
#include <Objet_U.h>
#include <cmath>

# ifndef M_E
#  define M_E                2.7182818284590452354
# endif
# ifndef M_LOG2E
#  define M_LOG2E        1.4426950408889634074
# endif
# ifndef M_LOG10E
#  define M_LOG10E        0.43429448190325182765
# endif
# ifndef M_LN2
#  define M_LN2                0.69314718055994530942
# endif
# ifndef M_LN10
#  define M_LN10        2.30258509299404568402
# endif
# ifndef M_PI
#  define M_PI                3.14159265358979323846
# endif
# ifndef M_PI_2
#  define M_PI_2        1.57079632679489661923
# endif
# ifndef M_PI_4
#  define M_PI_4        0.78539816339744830962
# endif
# ifndef M_1_PI
#  define M_1_PI        0.31830988618379067154
# endif
# ifndef M_2_PI
#  define M_2_PI        0.63661977236758134308
# endif
# ifndef M_2_SQRTPI
#  define M_2_SQRTPI        1.12837916709551257390
# endif
# ifndef M_SQRT2
#  define M_SQRT2        1.41421356237309504880
# endif
# ifndef M_SQRT1_2
#  define M_SQRT1_2        0.70710678118654752440
# endif
# ifndef DMAXFLOAT
#  define DMAXFLOAT        1e+38
# endif
# ifndef DMINFLOAT
#  define DMINFLOAT        1e-30
# endif

/*! @brief : useful functions on doubles
 *
 */
#ifdef LATATOOLS
#define KOKKOS_INLINE_FUNCTION inline
#endif

KOKKOS_INLINE_FUNCTION double carre(double x) { return x*x; }

inline double* prodvect(const double* a, const double* b, double* resu)
{
  resu[0]=a[1]*b[2]-a[2]*b[1];
  resu[1]=a[2]*b[0]-a[0]*b[2];
  resu[2]=a[0]*b[1]-a[1]*b[0];
  return(resu);
}

inline double determinant(double a[2], double b[2]) { return (a[0]*b[1]-a[1]*b[0]); }
#if 0
inline double norme(int n, const double* const a)
{
  const integer un=1;
  const integer m=n ;
  return F77NAME(DNRM2)(&m, a, &un);
}
#endif

KOKKOS_INLINE_FUNCTION
int est_egal(double x1, double x2, double eps)
{
  double somme = std::fabs(x1)+std::fabs(x2);
  return ( (somme < eps) || (std::fabs(x1-x2) <= eps * somme) );
}

inline int est_egal(double x1, double x2) { return est_egal(x1,x2,Objet_U::precision_geom); }

inline int est_different(double x1, double x2, double eps) { return !est_egal(x1,x2,eps); }

inline int est_different(double x1, double x2) { return !est_egal(x1,x2); }

inline int inf_ou_egal(double x1, double x2, double eps)
{
  double somme=std::fabs(x1)+std::fabs(x2);
  return ( (somme < eps) || (x1 <= x2 + eps * somme));
}

inline int inf_ou_egal(double x1, double x2) { return inf_ou_egal(x1,x2,Objet_U::precision_geom); }

inline int inf_strict(double x1, double x2, double eps)
{
  double somme=std::fabs(x1)+std::fabs(x2);
  return ( (somme > eps) && (x1 < x2 - eps * somme) );
}

inline int inf_strict(double x1, double x2) { return inf_strict(x1,x2,Objet_U::precision_geom); }

inline int sup_strict(double x1, double x2, double eps) { return !inf_ou_egal(x1,x2,eps); }

inline int sup_strict(double x1, double x2) { return sup_strict(x1,x2,Objet_U::precision_geom); }

inline int sup_ou_egal(double x1, double x2, double eps) { return !inf_strict(x1,x2,eps); }

inline int sup_ou_egal(double x1, double x2) { return sup_ou_egal(x1,x2,Objet_U::precision_geom); }

#endif /* DOUBLE_H */
