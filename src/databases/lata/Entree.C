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

#include <Entree.h>
#include <Objet_U.h>
#include <Nom.h>
#include <errno.h>
#include <fstream>

using std::ifstream;

Entree::Entree() : AbstractIO(),
  error_action_(ERROR_CONTINUE),
  diffuse_(true),
  istream_(nullptr)
{
}

Entree::Entree(istream& is) : Entree()
{
  istream_ = new istream(is.rdbuf());
}

Entree::Entree(const Entree& is) : Entree()
{
  istream_ = new istream(is.get_istream().rdbuf());
}

istream& Entree::get_istream()
{
  return *istream_;
}

const istream& Entree::get_istream() const
{
  return *istream_;
}
void Entree::set_istream(istream* is)
{
  istream_ = is;
}

Entree& Entree::operator >>(Entree& (*f)(Entree&))
{
  (*f)(*this);
  return *this;
}
Entree& Entree::operator >>(istream& (*f)(istream&))
{
  (*f)(*istream_);
  return *this;
}
Entree& Entree::operator >>(ios& (*f)(ios&))
{
  (*f)(*istream_);
  return *this;
}

// Assignment operators
Entree& Entree::operator=(istream& is)
{
  if(istream_)
    delete istream_;
  istream_ = new istream(is.rdbuf());
  return *this;
}

Entree& Entree::operator=(Entree& is)
{
  if(istream_)
    delete istream_;
  istream_ = new istream(is.get_istream().rdbuf());
  return *this;
}

/*! @brief Reads a string from ostream_. bufsize is the size of the buffer allocated for ob (including
 *
 *   the final null character).
 *   The string always contains a null character even on failure.
 *   The method returns 1 if reading is successful, 0 otherwise.
 *   If the buffer is too small, we currently call exit(), but
 *   in the future we could test: if strlen(ob)==bufsize-1, then
 *   call lire() again until the end is reached. If the next lire()
 *   returns a string of length zero, it means the string length was
 *   exactly bufsize-1.
 *   Warning: the behaviour differs between binary and ASCII mode.
 *    In binary mode, the string is read until the next '\0'.
 *    In ASCII mode, the string is read until the next separator (space, tab, newline).
 *
 */
int Entree::get(char* ob, std::streamsize bufsize)
{
  assert(istream_!=0);
  assert(bufsize > 0);
  ob[bufsize-1] = 1;
  if(bin_)
    {
      // In binary mode, read until the next null character
      // (including spaces, newlines, etc.)
      std::streamsize i;
      for (i = 0; i < bufsize-1; i++)
        {
          (*istream_).read(ob+i, sizeof(char));
          if (!error_handle(istream_->fail()))
            ob[i] = 0;
          if (ob[i] == 0)
            break;
        }
      ob[i] = 0;
    }
  else
    {
      // C++20 solution: use std::string then copy
      std::string temp;
      (*istream_) >> temp;
      if (!error_handle(istream_->fail()))
        {
          ob[0] = 0;
        }
      else
        {
          std::streamsize len = std::min(static_cast<std::streamsize>(temp.size()), bufsize - 1);
          std::memcpy(ob, temp.c_str(), len);
          ob[len] = '\0';
        }
    }
  if (ob[bufsize-1] == 0)
    {
      // Note Benoit Mathieu:
      // If the buffer was filled to the end, it is probably too small.
      // Continuing to read is dangerous because in ASCII mode we cannot know
      // whether the string was read exactly (which would be fine) or whether
      // it was not fully read as an int. Thorough testing of the STL would be
      // needed, and the result likely depends on the implementation. Therefore,
      // if the buffer is full, we abort the code.
      Cerr << "Error in Entree::lire(char* ob, int bufsize) : buffer too small" << finl;
      Process::exit();
    }
  return (!istream_->fail());
}

void error_convert(const char * s, const char * type)
{
  Cerr << "Error converting a string to type " <<  type << " : string = " << s << finl;
  Process::exit();
}

/*! @brief Conversion method.
 *
 */
void convert_to(const char *s, int& ob)
{
  errno = 0;
  char * errorptr = 0;
  ob = (int)strtol(s, &errorptr, 0 /* base 10 by default */);
  if (errno || *errorptr != 0) error_convert(s,"int");
}

void convert_to(const char *s, long& ob)
{
  errno = 0;
  char * errorptr = 0;
  ob = strtol(s, &errorptr, 0 /* base 10 by default */);
  if (errno || *errorptr != 0)  error_convert(s,"long");
}

void convert_to(const char *s, long long& ob)
{
  errno = 0;
  char * errorptr = 0;
#ifdef HPPA_11 /* NO_PROCESS */
  ob = strtol(s, &errorptr, 0 /* base 10 by default */);
#else /* NO_PROCESS */
#ifdef MICROSOFT
  ob = _strtoi64(s, &errorptr, 0 /* base 10 by default */);
#else
  ob = strtoll(s, &errorptr, 0 /* base 10 by default */);
#endif
#endif /* NO_PROCESS */
  if (errno || *errorptr != 0)  error_convert(s,"long long");
}

void convert_to(const char *s, float& ob)
{
  errno = 0;
  char * errorptr = 0;
  ob = strtof(s, &errorptr);
  if (errno || *errorptr != 0)  error_convert(s,"float");
}

void convert_to(const char *s, double& ob)
{
  errno = 0;
  char * errorptr = 0;
  ob = strtod(s, &errorptr);
  if (errno || *errorptr != 0)  error_convert(s,"double");
}

// Virtual method to read an int or a real number. In this base class, reading is done from istream using read() (if is_bin() == 1)
// or using operator>>() (if is_bin() == 0). If the check_types flag is set, convert_to() is called to verify the types of the objects read.
// In that case, for ints, the formats 123 (decimal), 0xa345 (hexadecimal) and others are accepted.
// If an error occurs, error_handle_() is called.
// Note for developers of derived classes: the implementation of this method must always go through error_handle_().
Entree& Entree::operator>>(double& ob) { return operator_template<double>(ob); }

// Virtual method to read an array of ints or reals (the array must have the correct size: no verification is possible)
int Entree::get(double * ob, std::streamsize n) { return get_template<double>(ob,n); }

Entree& Entree::operator>>(int& ob) { return operator_template<int>(ob); }
int Entree::get(int * ob, std::streamsize n) { return get_template<int>(ob,n); }

Entree& Entree::operator>>(float& ob) { return operator_template<float>(ob); }
int Entree::get(float * ob, std::streamsize n) { return get_template<float>(ob,n); }

Entree& Entree::operator>>(long& ob) { return operator_template<long>(ob); }
int Entree::get(long * ob, std::streamsize n) { return get_template<long>(ob,n); }

Entree& Entree::operator>>(long long& ob) { return operator_template<long long>(ob); }
int Entree::get(long long * ob, std::streamsize n) { return get_template<long long>(ob,n); }

// Yes this is awful. We will get rid of this along with Nom.
Entree& Entree::operator>>(std::string& ob) { Nom tmp; *this >> tmp; ob = tmp.getString(); return *this;}

Entree& Entree::operator >>(Objet_U& ob) { return ob.readOn(*this); }

int Entree::jumpOfLines()
{
  if(istream_)
    {
      int jol = 0;
      char tmp=(char)istream_->peek();
      while(isspace(tmp))  //tmp=='\n')
        {
          if(tmp=='\n')
            jol++;
          istream_->get(tmp);
          tmp=(char)istream_->peek();
        }
      return jol;
    }
  return -1;
}

int Entree::eof()
{
  if(istream_)
    return (istream_->eof());
  else
    return -1;
}
int Entree::fail()
{
  if(istream_)
    return (istream_->fail());
  else
    return -1;
}
int Entree::good()
{
  if(istream_)
    return (istream_->good());
  else
    return -1;
}
Entree::~Entree()
{
#ifndef TRUST_USE_UVM // ToDo bug ?
  if(istream_)
    delete istream_;
#endif
  istream_=nullptr;
}

/*! @brief Changes the file write mode.
 *
 * This method can be called at any time.
 *
 */
void Entree::set_bin(bool bin)
{
  bin_ = bin;
  if (istream_)
    {
      Cerr<<"Error you cant change binary format after open "<<finl;
      assert(0);
      Process::exit();
    }
}

/*! @brief Indicates whether the stream should verify the types of read objects (ints and floating-point numbers).
 *
 * Example: the input contains 123.456 123.456
 *   int i;
 *   check_types(0);
 *   is >> i;   // i contains 123
 *   check_types(1);
 *   is >> i;   // Error: reads the string 123.456 and tries to convert it to int
 *   See operator>>(int &)
 *
 */
void Entree::set_check_types(bool flag)
{
  check_types_ = flag;
}


/*! @brief This function is called by operator>>, get, get_nom, ouvrir, fermer, lire, etc.
 *
 * .. on failure (when fail() is set).
 *   It returns 0 if an error occurred (go through error_handle() which
 *   handles inline the case where there is no error), and 1 if there is no error.
 *   (for coding convenience, one writes "return error_handle(fail());"
 *   It can be configured to:
 *    - return "0" on error and continue code execution
 *      (case of legacy code that does not handle exceptions but periodically
 *       tests the fail() flag)
 *      In this case, operator>> methods continue execution even on failure,
 *      and the content of the read variables is undefined!
 *    - call Process::exit() (case of a code section where no error handling
 *      is desired and everything is assumed to succeed)
 *    - throw an exception (allows rigorous error handling and optimal
 *      user feedback depending on context)
 *
 *   @sa set_error_action()
 *
 */
int Entree::error_handle_(int fail_flag)
{
  if (!fail_flag)
    return 1;

  switch(error_action_)
    {
    case ERROR_CONTINUE:
      break;
    case ERROR_EXIT:
      Cerr << "Error while reading in Entree object. Exiting.\n";
      if (istream_)
        {
          // We do not use Entree::eof() because in the case of a Lec_Fic_Dif,
          // eof() is parallel and could block.
          if (get_istream().eof())
            Cerr << " End of file reached." << finl;
          else
            Cerr << " IO error (not an EOF error)." << finl;
        }
      Process::exit();
      break;
    case ERROR_EXCEPTION:
      Entree_Sortie_Error e;
      throw (e);
    }
  return 0;
}

/*! @brief Returns error_action_ for this input (allows modifying it and restoring the previous value afterwards).
 *
 */
Entree::Error_Action Entree::get_error_action()
{
  return error_action_;
}

/*! @brief Changes the error behaviour of the input; see error_handle_() and get_error_action().
 *
 */
void Entree::set_error_action(Entree::Error_Action action)
{
  error_action_ = action;
}

// Detects whether a file with name filename
// is of binary type
int is_a_binary_file(Nom& filename)
{
  // Scan the first 1000 bytes
  // Very imperfect detection, therefore limited
  // to TRUST geometry files
  int n=0;
  int c;
  std::ifstream fic(filename.getChar());
  // If a character with ASCII value > 127 is encountered,
  // the file is of binary type
  while((c = fic.get()) != EOF && n++<1000)
    if ((c>127)||(c<9))
      return 1;
  // GF on windows, binary characters are mostly <9
  //  else printf("ici %d %c \n",c,c);
  return 0;
}

/*! @brief Sets the diffuse flag for this input stream.
 *
 * This virtual method does nothing in the base class; see override in Lec_Diffuse_base.
 * @param diffuse Whether to diffuse data to other processes.
 */
void Entree::set_diffuse(bool diffuse)
{
  // virtual method does nothing ; cf override in Lec_Diffuse_base
  diffuse_ = true;
}


template<typename _TYPE_>
int Entree::get_template(_TYPE_ *ob, std::streamsize n)
{
  assert(istream_!=0);
  assert(n >= 0);
  if (bin_)
    {
      if (this->must_convert<_TYPE_>())
        {
          // Need to cast, use '>>' operator - see doc in operator_template<>
          for (int i = 0; i < n; i++) (*this) >> ob[i];
        }
      else
        {
          // In binary, optimized block reading:
          char *ptr = (char*) ob;
          std::streamsize sz = sizeof(_TYPE_);
          sz *= n;
          istream_->read(ptr, sz);
          error_handle(istream_->fail());
        }
    }
  else
    {
      // In ASCII mode: use operator>> to verify conversions
      // Warning: we call the one from this class, not from a derived class
      for (int i = 0; i < n; i++) Entree::operator>>(ob[i]);
    }
  return (!istream_->fail());
}

// Explicit instanciations:
template int Entree::get_template(int *ob, std::streamsize n);
template int Entree::get_template(long *ob, std::streamsize n);
template int Entree::get_template(long long *ob, std::streamsize n);
template int Entree::get_template(double *ob, std::streamsize n);
template int Entree::get_template(float *ob, std::streamsize n);


template <typename _TYPE_>
Entree& Entree::operator_template(_TYPE_& ob)
{
  assert(istream_!=0);
  if (bin_)
    {
      // Do we need to worry about 32b / 64b conversion?
      if (this->must_convert<_TYPE_>())
        {
          // Yes, then two cases:
          // Case 1: requested _TYPE_ is 32b and file is 64b -> only OK if read value is actually within the 32b range
          if (is_64b_)
            {
              std::int64_t pr;
              char *ptr = (char*) &pr;
              istream_->read(ptr, sizeof(std::int64_t));
              if (pr > std::numeric_limits<int>::max())
                {
                  Cerr << "Can't read this int64 binary file with an int32 binary: values too big, overflow!!" << finl;
                  throw;
                }
              // It's ok, we passed the check above, we can safely downcast:
              ob = static_cast<_TYPE_>(pr);
            }
          // Case 2: requested _TYPE_ is 64b and file is 32b -> this is always OK, just need int to make sure we really read a 32b value
          else
            {
              int pr;
              char * ptr = (char*) &pr;
              istream_->read(ptr, sizeof(int));
              ob=(_TYPE_)pr;
            }
        }
      else  // File has the same bit-ness as binary, or we are trying to read a non-problematic type - all OK.
        {
          char *ptr = (char*) &ob;
          istream_->read(ptr, sizeof(_TYPE_));
          error_handle(istream_->fail());
        }
    }
  else  // Not binary, ascii format
    {
      if (check_types_)
        {
          char buffer[100];
          int ok = Entree::get(buffer, 100); // Must call get() from this class, not a derived one
          if (ok)
            convert_to(buffer, ob);
        }
      else
        {
          (*istream_) >> ob;
          error_handle(istream_->fail());
        }
    }
  return *this;
}

// Explicit instanciations:
template Entree& Entree::operator_template(int& ob);
template Entree& Entree::operator_template(long& ob);
template Entree& Entree::operator_template(long long& ob);
template Entree& Entree::operator_template(double& ob);
template Entree& Entree::operator_template(float& ob);

