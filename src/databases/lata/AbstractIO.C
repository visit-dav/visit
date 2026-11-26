/****************************************************************************
* Copyright (c) 2026, CEA
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

#include <AbstractIO.h>
#include <arch.h>
#include <type_traits>

AbstractIO::AbstractIO() :
#ifdef INT_is_64_   // in 64b, we write 64b integers by default. But sometimes (save/restart files for example) we might want to still write 32b.
  is_64b_(true)
#else
  is_64b_(false)
#endif
{ }


/*! @brief Whether to convert an int into a long when reading/writing out data.
 *
 * The rules are:
 *   - for 32 files, everything is read/written as 'int', even if _TYPE_ 'long' (or trustIdType) was requested - overflow is checked
 *     in Entree/Sortie classes
 *   - for 64 files, everything is read/written as 'long', even if _TYPE_ 'int' (or trustIdType) was requested
 *     This is for historical reasons: in the former 64b version of TRUST all int were replaced by long, we need to
 *     keep backward compatibility. When we read a former .sauv file written prior to 1.9.5 for example, all integer values are 'long'.
 *     But we might need to load them in an 'int' variable (for example this is the case with the 'format_sauvegarde'
 *     value in Probleme_base).
 *
 * Note however that we can force the file to written in 32b even when running the 64b executable, thanks to the is_64b
 * flag. This is what we do in Problem_base, save/restart logic.
 */
template<typename _TYPE_>
bool AbstractIO::must_convert() const
{
  static constexpr bool IS_INTEG = std::is_integral<_TYPE_>::value;

  if (!IS_INTEG || avoid_conversion_)
    return false;
  else
    {
      const bool SAME_BITNESS = (std::is_same<_TYPE_, std::int64_t>::value && is_64b_)  // The file being read is 64b and _TYPE_ is 64b (long)
                                || (std::is_same<_TYPE_, std::int32_t>::value && !is_64b_);         // The file being read is 32b and _TYPE_ is 32b (int)
      return !SAME_BITNESS;
    }
}

template bool AbstractIO::must_convert<int>() const;
template bool AbstractIO::must_convert<unsigned>() const;
template bool AbstractIO::must_convert<unsigned long>() const;
template bool AbstractIO::must_convert<long>() const;
template bool AbstractIO::must_convert<long long>() const;
template bool AbstractIO::must_convert<float>() const;
template bool AbstractIO::must_convert<double>() const;



