/****************************************************************************
* Copyright (c) 2024, CEA
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

#ifndef TRUSTArray_tools_TPP_included
#define TRUSTArray_tools_TPP_included

/*! @brief Renvoie 1 si les tableaux "v" et "a" sont de la meme taille et contiennent les memes valeurs au sens strict, sinon renvoie 0.
 *
 * Le test est !(v[i]!=a[i])
 *
 */
template <typename _TYPE_, typename _SIZE_>
bool operator==(const TRUSTArray<_TYPE_,_SIZE_>& v, const TRUSTArray<_TYPE_,_SIZE_>& a)
{
  const _SIZE_ n = v.size_array();
  const _SIZE_ na = a.size_array();

  if (n != na) return false;

  v.ensureDataOnHost();
  a.ensureDataOnHost();
  const _TYPE_ * vv = v.addr();
  const _TYPE_ * av = a.addr();
  for (_SIZE_ i = 0; i < n; i++)
    if (av[i] != vv[i])
      return false;
  return true;
}

/*! @brief Retourne l'indice du min ou -1 si le tableau est vide
 *
 */
template <typename _TYPE_, typename _SIZE_> _SIZE_ imin_array(const TRUSTArray<_TYPE_,_SIZE_>& dx)
{
  _SIZE_ indice_min = -1;
  const _SIZE_ size = dx.size_array();
  if (size > 0)
    {
      indice_min = 0;
      _TYPE_ valeur_min = dx[0];
      for (_SIZE_ i = 1; i < size; i++)
        {
          const _TYPE_ val = dx[i];
          if(val < valeur_min)
            {
              indice_min = i;
              valeur_min = val;
            }
        }
    }
  return indice_min;
}

/*! @brief Retourne l'indice du max ou -1 si le tableau est vide
 *
 */
template <typename _TYPE_, typename _SIZE_> _SIZE_ imax_array(const TRUSTArray<_TYPE_,_SIZE_>& dx)
{
  _SIZE_ indice_max = -1;
  const _SIZE_ size = dx.size_array();
  if (size > 0)
    {
      indice_max = 0;
      _TYPE_ valeur_max = dx[0];
      for (_SIZE_ i = 1; i < size; i++)
        {
          const _TYPE_ val = dx[i];
          if(val > valeur_max)
            {
              indice_max = i;
              valeur_max = val;
            }
        }
    }
  return indice_max;
}

/*! @brief Retourne la valeur minimale
 *
 */
template <typename _TYPE_, typename _SIZE_>
_TYPE_ min_array(const TRUSTArray<_TYPE_,_SIZE_>& dx)
{
  const _SIZE_ size = dx.size_array();
  assert(size > 0);
  _TYPE_ valeur_min = dx[0];
  for (_SIZE_ i = 1; i < size; i++)
    {
      const _TYPE_ val = dx[i];
      if (val < valeur_min) valeur_min = val;
    }
  return valeur_min;
}

/*! @brief Retourne la valeur maximale
 *
 */
template <typename _TYPE_, typename _SIZE_>
_TYPE_ max_array(const TRUSTArray<_TYPE_,_SIZE_>& dx)
{
  const _SIZE_ size = dx.size_array();
  assert(size > 0);
  _TYPE_ valeur_max = dx[0];
  for (_SIZE_ i = 1; i < size; i++)
    {
      const _TYPE_ val = dx[i];
      if (val > valeur_max) valeur_max = val;
    }
  return valeur_max;
}

// Absolute value for a scalar ....
namespace
{
template <typename _TYPE_>
inline _TYPE_ scalar_abs(_TYPE_ x)
{
  constexpr bool IS_INT = std::is_integral<_TYPE_>::value;
  return IS_INT ? std::abs(x) : std::fabs(x);
}
}

/*! @brief Retourne le max des abs(i)
 */
template <typename _TYPE_, typename _SIZE_>
_TYPE_ max_abs_array(const TRUSTArray<_TYPE_,_SIZE_>& dx)
{
  const _SIZE_ size = dx.size_array();
  assert(size > 0);
  _TYPE_ valeur_max = ::scalar_abs(dx[0]);
  for(_SIZE_ i = 1; i < size; i++)
    {
      const _TYPE_ val = ::scalar_abs(dx[i]);
      if (val > valeur_max) valeur_max = val;
    }
  return valeur_max;
}

/*! @brief Retourne le min des abs(i)
 *
 */
template <typename _TYPE_, typename _SIZE_>
_TYPE_ min_abs_array(const TRUSTArray<_TYPE_,_SIZE_>& dx)
{
  const _SIZE_ size = dx.size_array();
  assert(size > 0);
  _TYPE_ v = ::scalar_abs(dx[0]);
  for(_SIZE_ i = 1; i < size; i++)
    {
      const _TYPE_ val = ::scalar_abs(dx[i]);
      if (val < v) v = val;
    }
  return v;
}

/*! @brief Produit scalaire de deux "array"  (dotproduct_array remplace operator* car le nom indique clairement  que l'on fait un produit scalaire non distribue)
 *
 */
template <typename _TYPE_, typename _SIZE_>
_TYPE_ dotproduct_array(const TRUSTArray<_TYPE_,_SIZE_>& dx, const TRUSTArray<_TYPE_,_SIZE_>& dy)
{
  const _SIZE_ size = dx.size_array();
  assert(size == dy.size_array());
  _TYPE_ resultat = 0.;
  if (size > 0)
    {
      integer n = size;
      integer incx = 1;
      dx.ensureDataOnHost();
      dy.ensureDataOnHost();
      resultat = F77NAME(DDOT)(&n, dx.addr(), &incx, dy.addr(), &incx);
    }
  return resultat;
}


/*! @brief Calcul de la norme du vecteur dx (fonction blas DNRM2, soit racine carree(somme des dx[i]*dx[i]).
 *
 */
template <typename _TYPE_, typename _SIZE_>
_TYPE_ norme_array(const TRUSTArray<_TYPE_,_SIZE_>& dx)
{
  integer n = dx.size_array();
  _TYPE_ resultat = 0.;
  if (n > 0)
    {
      integer incx = 1;
      dx.ensureDataOnHost();
      resultat = F77NAME(DNRM2)(&n, &dx[0], &incx);
    }
  return resultat;
}

#endif /* TRUSTArray_tools_TPP_included */
