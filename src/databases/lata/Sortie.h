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

#ifndef Sortie_included
#define Sortie_included

#include <AbstractIO.h>
#include <iostream>
#include <assert.h>
#include <arch.h>
#include <string>
#include <memory>

using std::ostream;
using std::flush;
using std::endl;
using std::cerr;
using std::ends;
using std::cout;
using std::ios;
using std::cin;

template <typename T> class TRUST_Ref;
class TRUST_Ref_Objet_U;
class Separateur;
class Objet_U;

/*! @brief Base class for output streams.
 *
 * Knows how to write simple types (integers, floats) and Objet_U objects (via printOn of the Objet_U).
 *    Note: some derived classes are parallel; in that case, syncfile() must be called periodically
 *    on all processors. See class EcrFicPartage.
 *    Note: to write correctly to a stream in both ASCII and BINARY mode,
 *    a Separateur (finl or space) must be used to separate written objects.
 *
 * @sa Entree
 */

class Sortie: public AbstractIO
{
public:
  Sortie();
  Sortie(ostream& os);
  Sortie(const Sortie& os);
  virtual ~Sortie() {}

  Sortie& operator=(ostream& os);
  Sortie& operator=(Sortie& os);

  void set_bin(bool bin) override;

  inline ostream& get_ostream() { return *ostream_; }
  inline const ostream& get_ostream() const { return *ostream_; }
  inline void set_col_width(int w) { col_width_ = w; }

  Sortie& operator <<(ostream& (*f)(ostream&));
  Sortie& operator <<(Sortie& (*f)(Sortie&));
  Sortie& operator <<(ios& (*f)(ios&));

  virtual Sortie& flush();
  virtual Sortie& lockfile();
  virtual Sortie& unlockfile();
  virtual Sortie& syncfile();
  virtual void setf(IOS_FORMAT);
  virtual void precision(int);

  template <typename T>
  Sortie& operator<<(const TRUST_Ref<T>& ) { std::cerr << __func__ << " :: SHOULD NOT BE CALLED ! Use -> !! " << std::endl ; throw; }

  Sortie& operator<<(const TRUST_Ref_Objet_U& ) { std::cerr << __func__ << " :: SHOULD NOT BE CALLED ! Use -> !! " << std::endl ; throw; }

  virtual Sortie& operator<<(const Separateur& );
  virtual Sortie& operator<<(const Objet_U&    ob);
  virtual Sortie& operator<<(const int    ob);
  virtual Sortie& operator<<(const unsigned ob);
  virtual Sortie& operator<<(const long      ob);
  virtual Sortie& operator<<(const long long      ob);
  virtual Sortie& operator<<(const unsigned long      ob);
  virtual Sortie& operator<<(const float     ob);
  virtual Sortie& operator<<(const double    ob);
  virtual Sortie& operator<<(const char      * ob);
  virtual Sortie& operator<<(const std::string& str);

  virtual int add_col(const double ob);
  virtual int add_col(const char * ob);

  // The put methods can potentially write long stream of data (std::streamsize == ptrdiff_t == long)
  // but in many derived classes only used after the Scatter (e.g. LecFicDiffuse) we downcast to int inside impl.
  virtual int put(const unsigned* ob, std::streamsize n, std::streamsize nb_colonnes=1);
  virtual int put(const int* ob, std::streamsize n, std::streamsize nb_colonnes=1);
  virtual int put(const float * ob, std::streamsize n, std::streamsize nb_colonnes=1);
  virtual int put(const double* ob, std::streamsize n, std::streamsize nb_colonnes=1);
  virtual int put(const long  * ob, std::streamsize n, std::streamsize nb_colonnes=1);
  virtual int put(const long long * ob, std::streamsize n, std::streamsize nb_colonnes=1);

  inline bool has_ostream() const { return ostream_ != nullptr; }

protected:
  int col_width_;

  /*! A smart pointer to a std::ostream object, or any of its derived class.
   * Explicit construction might be done in derived classes of Sortie.
   */
  std::unique_ptr<ostream> ostream_;

private:
  template <typename _TYPE_>
  int put_template(const _TYPE_* ob, std::streamsize n, std::streamsize nb_col);

  template <typename _TYPE_>
  Sortie& operator_template(const _TYPE_& );
};

#endif /* Sortie_included */
