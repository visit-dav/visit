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

#ifndef Entree_included
#define Entree_included

#include <Process.h>
#include <AbstractIO.h>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <cstdio> // For EOF on GNU >= 4.4
#include <arch.h> // for LIBLATAFILTER int64
#include <cstdint>// For INT32_MAX on GNU >= 13

using std::istream;
using std::ios;

template <typename T> class TRUST_Ref;
class TRUST_Ref_Objet_U;
class Objet_U;
class Nom;

/*! @brief Class defining operators and methods for all reading operation in an input flow (file, keyboard
 * communication buffer, etc.)
 *
 * @sa Sortie
 */
class Entree: public AbstractIO
{
public:
  // Constructors
  Entree();
  Entree(istream& is);
  Entree(const Entree& is);
  virtual ~Entree();

  void set_bin(bool bin) override;

  // Assignment operators
  Entree& operator=(istream& is);
  Entree& operator=(Entree& is);

  virtual istream& get_istream();
  virtual const istream& get_istream() const;
  void set_istream(istream *is);

  Entree& operator >>(Entree& (*f)(Entree&));
  Entree& operator >>(istream& (*f)(istream&));
  Entree& operator >>(ios& (*f)(ios&));

  template <typename T>
  Entree& operator>>(const TRUST_Ref<T>& ) { std::cerr << __func__ << " :: SHOULD NOT BE CALLED ! Use -> !! " << std::endl ; throw; }

  Entree& operator>>(const TRUST_Ref_Objet_U& ) { std::cerr << __func__ << " :: SHOULD NOT BE CALLED ! Use -> !! " << std::endl ; throw; }

  virtual Entree& operator>>(int& ob);
  virtual Entree& operator>>(long& ob);
  virtual Entree& operator>>(long long& ob);
  virtual Entree& operator>>(float& ob);
  virtual Entree& operator>>(double& ob);
  virtual Entree& operator>>(std::string& ob);

  // final
  virtual Entree& operator>>(Objet_U& ob) final;

  virtual int get(int *ob, std::streamsize n);
  virtual int get(long *ob, std::streamsize n);
  virtual int get(long long *ob, std::streamsize n);
  virtual int get(float *ob, std::streamsize n);
  virtual int get(double *ob, std::streamsize n);

  virtual int get(char *buf, std::streamsize bufsize);

  virtual int eof();
  virtual int jumpOfLines();
  virtual int fail();
  virtual int good();
  virtual void set_check_types(bool flag);
  bool check_types() const { return check_types_; }
  enum Error_Action { ERROR_EXIT, ERROR_CONTINUE, ERROR_EXCEPTION };
  virtual void set_error_action(Error_Action);
  Error_Action get_error_action();

  inline operator istream& () { return get_istream(); }
  inline istream& putback(char ch) { return get_istream().putback(ch); }
  inline bool get_diffuse() { return diffuse_; }

  virtual void set_diffuse(bool diffuse);

protected:
  // Inline method to handle the trivial case quickly. Otherwise, calls the virtual method error_handle_().
  inline int error_handle(int fail_flag)
  {
    if (!fail_flag) return 1;
    else return error_handle_(fail_flag);
  }

  virtual int error_handle_(int fail_flag);
  bool check_types_ = false;
  Error_Action error_action_;
  bool diffuse_; // By default true, but some child classes (e.g. LecFicDiffuse) can set it temporarily to false to not diffuse to other processes

private:
  istream *istream_;

  template <typename _TYPE_>
  int get_template(_TYPE_ *ob, std::streamsize n);

  template <typename _TYPE_>
  Entree& operator_template(_TYPE_& ob);
};

int is_a_binary_file(Nom&);

void convert_to(const char *s, int& ob);
void convert_to(const char *s, long& ob);
void convert_to(const char *s, long long& ob);
void convert_to(const char *s, float& ob);
void convert_to(const char *s, double& ob);

// Class returned by Entree when an exception is raised on error
class Entree_Sortie_Error
{
};

#endif
