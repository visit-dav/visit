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

#ifndef AbstractIO_included
#define AbstractIO_included


/*! @brief Base class for all input/output streams.
 *
 * Holds information about binary/ascii format, and 32b/64b information.
 *
 *   - bin_ indicates whether the stream is binary or ASCII
 *   - is_64b_ incidates whether the stream is in 64b or 32b
 *   - avoid_conversion_ is used for MPI Comm buffer, where we do not want any int/long conversion.
 *
 * See method must_convert() and file arch.h.in for more explanations on 32/64b.
 *
 * Note that the .sauv files for example are always written in 32b.
 *
 * @sa Entree, Sortie, and specifically the comments in operator_template<>() method.
 */
class AbstractIO
{
public:
  bool is_64b() const                   { return is_64b_; }
  virtual void set_64b(bool is_64b)     { is_64b_ = is_64b;  }
  virtual void set_bin(bool bin)        { bin_ = bin; }
  bool is_bin()                         { return bin_; }
  void set_avoid_conversion(bool avoid) { avoid_conversion_ = avoid; }
  bool avoid_conversion()               { return avoid_conversion_; }

protected:
  AbstractIO();
  virtual ~AbstractIO() { }

  template<typename _TYPE_> bool must_convert() const;

  bool bin_ = false;              ///< Is this a binary flux?
  bool is_64b_;           ///< Will we be reading/writing in 64b? (Init in ctor to avoid including arch.h probably)

  /*! If true, no hacking on int/long is performed in operator_template() methods of Entree/Sortie
   * This is useful for CommBuffer classes (=MPI exchanges) where we always want int to be sent as int, and long to be
   * sent as long.
   */
  bool avoid_conversion_ = false;
};

#endif /* AbstractIO_included */
