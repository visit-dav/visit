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

#include <Nom.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include <algorithm>
#include <cmath>

Implemente_instanciable_sans_constructeur_ni_destructeur(Nom,"Nom",Objet_U);
// XD nom objet_u nom NO_BRACE Class to name the TRUST objects.
// XD attr mot chaine mot OPT Chain of characters.

/*! @brief Overrides Objet_U::printOn(Sortie&). Writes a Nom to an output stream.
 *
 * @param (Sortie& s) the output stream to use
 * @return (Sortie&) the modified output stream
 */
Sortie& Nom::printOn(Sortie& s) const
{
  const char* nom=getChar();
  if(nom)
    return s << nom;
  else
    return s;
}
#define BUFLEN 100000

/*! @brief Reads a name. On failure, the name is set to "??".
 *
 */
Entree& Nom::readOn(Entree& s)
{
  char buffer[BUFLEN];
  const int ok = s.get(buffer, BUFLEN);
  if (ok)
    operator=(buffer);
  else
    operator=("??");
  return s;
}

/*! @brief Default constructor. Creates the string "??".
 *
 */
Nom::Nom()
{
  nom_ = "??";
}

/*! @brief Constructs a name from a single character.
 *
 * @param (char c) the character of the name
 */
Nom::Nom(char c)
{
  nom_=c;
}


/*! @brief Constructs a name from an integer. The string created is the decimal representation of the integer.
 *
 *     Example: Nom(128) creates the string "128".
 *
 * @param (int i) the integer to use
 */
Nom::Nom(int i)
{
  nom_ = "";
  // 22 characters are sufficient to store any integer
  char chaine[22];
  snprintf(chaine, 22, "%d", i);
  operator=(chaine);
}

Nom::Nom(long i)
{
  nom_ = "";
  // 22 characters are sufficient to store any integer
  char chaine[22];
  snprintf(chaine, 22, "%ld", i);
  operator=(chaine);
}

Nom::Nom(long long i)
{
  nom_ = "";
  // 22 characters are sufficient to store any long long integer
  char chaine[22];
  snprintf(chaine, 22, "%lld", i);
  operator=(chaine);
}

/*! @brief Constructs a name from a character string. The string is copied.
 *
 * @param (const char* nom) the character string to use
 */
Nom::Nom(const char* nom) : nom_(nom)
{
}

Nom::Nom(const std::string& nom) : nom_(nom)
{
}


/*! @brief Copy constructor for a name.
 *
 * @param (const Nom& nom) the name to use
 */
Nom::Nom(const Nom& nom) : Objet_U(nom), nom_(nom.nom_)
{
}

/*! @brief Constructs a name from a floating-point number. The string created is the decimal representation of the real number (snprintf).
 *
 * @param (double le_reel) the real number to use
 */
Nom::Nom(double le_reel)
{
  nom_ = "";
  char la_chaine[80];
  snprintf(la_chaine,80,"%f",le_reel);
  operator=(la_chaine);
}

/*! @brief Constructs a name from a floating-point number with a custom format. The string created is the decimal representation of the real number (snprintf).
 *
 *     The format of the number in the string is given by format.
 *
 * @param (double le_reel) the real number to use
 */
Nom::Nom(double le_reel, const char* format)
{
  char la_chaine[80];
  snprintf(la_chaine,80,format,le_reel);
#ifdef MICROSOFT
  // under windows, numbers are written as 1.0000e+000 (with 3 digits for exponents)
  // remove the first zero
  unsigned int length=strlen(la_chaine);
  if (la_chaine[length-5]=='e')
    {
      if (la_chaine[length-3]!='0') Process::exit();
      for (unsigned int i=length-3; i<=length; i++)
        la_chaine[i]=la_chaine[i+1];
    }
#endif
  nom_ =  la_chaine;
  //delete[] la_chaine;
}


/*! @brief Destructor.
 *
 */
Nom::~Nom()
{
}

/*! @brief Converts the name to uppercase. Only letters 'a'-'z' are modified.
 *
 */
Nom& Nom::majuscule()
{
  std::transform(nom_.begin(), nom_.end(), nom_.begin(), ::toupper);
  return *this;
}

/*! @brief Returns the number of characters in the Nom string, including the null terminator.
 *
 *     Example: Nom("hello").longueur() == 6.
 *
 */
int Nom::longueur() const
{
  return (int)nom_.size()+1;
}

/*! @brief Copies the string nom.
 *
 * BM modification so that nom can point to a sub-part of nom_.
 *
 */
Nom& Nom::operator=(const char* const nom)
{
  nom_=nom;
  return *this;
}

/*! @brief Copies the Nom nom.
 *
 * @param (const Nom& nom) the name to copy
 * @return (Nom&) reference to this, representing the string of Nom nom
 */
Nom& Nom::operator=(const Nom& nom)
{
  nom_ = nom.nom_;
  return *this;
}

/*! @brief Concatenation with a Nom.
 *
 * @param (const Nom& x) the name to concatenate
 * @return (Nom&) reference to this
 */
Nom& Nom::operator +=(const Nom& x)
{
  nom_ += x.nom_;
  return *this;
}

Nom& Nom::operator+=(const char *x)
{
  nom_ += x;
  return *this;
}

/*! @brief String concatenation.
 *
 */
Nom& Nom::operator +=(char x)
{
  char n[2];
  n[0] = x;
  n[1] = 0;
  operator+=(n);
  return *this;
}

Nom& Nom::operator +=(unsigned char x)
{
  char n[2];
  n[0] = (char)x;
  n[1] = 0;
  operator+=(n);
  return *this;
}


Nom& Nom::operator +=(int x)
{
  nom_ += Nom(x);
  return *this;
}

/*! @brief Suffix extraction: Nom x("azerty");
 *
 *      x.suffix("aze") leaves x containing "rty".
 *
 * @param (const char* const ch) character string to use as prefix to strip
 * @return (Nom&) reference to this
 */
Nom& Nom::suffix(const char* const s)
{
  if (debute_par(s))
    {
      int n2 = (int)strlen(s);
      nom_.erase(0,n2);
    }
  return *this;
}

const Nom Nom::getSuffix(const char* const s) const
{
  if (debute_par(s))
    {
      const int n1 = (int)strlen(s);
      const int n2 = (int)nom_.size();
      const std::string str1 = nom_.substr(n1,n2);
      return Nom(str1);

    }
  return *this;
}

int Nom::debute_par(const std::string& ch) const
{
  return (nom_.rfind(ch, 0) == 0);
}

int Nom::finit_par(const std::string& s) const
{
  auto l = nom_.size(), e = s.size();
  if (l >= e)
    return (0 == nom_.compare(l - e, e, s));
  else
    return 0;
}

int Nom::find(const std::string& n) const
{
  std::size_t x = nom_.find(n);
  return (x != std::string::npos) ? (int)x : -1;
}

int Nom::find(const char* const n ) const
{
  return find(std::string(n));
}

int Nom::debute_par(const char* const n) const
{
  return debute_par(std::string(n));
}

int Nom::finit_par(const char* const n) const
{
  return finit_par(std::string(n));
}

Nom& Nom::prefix(const char* const s)
{
  if (finit_par(s))
    {
      int n = (int)nom_.size();
      int n2 = (int)strlen(s);
      nom_.erase(n-n2,n2);
    }
  return *this;
}

const Nom Nom::getPrefix(const char* const s) const
{
  if (finit_par(s))
    {
      const int n1 = (int)nom_.size();
      const int n2 = (int)strlen(s);
      const std::string str1 = nom_.substr(0,n1-n2);
      return Nom(str1);

    }
  return *this;
}

/*! @brief Concatenation with a Nom.
 *
 * @param (const Nom& x) the name to concatenate
 * @return (Nom) the new Nom created by concatenating this and x
 */
Nom Nom::operator +(const Nom& x) const
{
  Nom nouveau(*this);
  nouveau += x;
  return nouveau;
}

/*! @brief Comparison with an Objet_U. The Objet_U is cast to Nom for the comparison.
 *
 * @param (const Objet_U& x) the Objet_U to use for comparison
 * @return (int) 1 if equal
 */
int Nom::est_egal_a(const Objet_U& x) const
{
#ifndef LATATOOLS
  if (!(sub_type(Nom, x))) return 0;
  return (*this == ref_cast( Nom, x));
#else
  const Nom& n2 = dynamic_cast<const Nom&>(x);
  return (*this == n2);
#endif
}

/*! @brief Inserts _prefix000n (n=me() or nproc()) into a file name (e.g. toto.titi) to produce toto_prefix000n.titi.
 *
 * @param (without_padding) flag indicating that leading zeros should not be added before n
 */
Nom Nom::nom_me(int n, const char* prefixe, int without_padding) const
{
  int compteur=(int)nom_.size();
  const char* ptr=nom_.c_str()+compteur;
  while((*ptr!='.') && (*ptr!='/')&&(compteur>0))  // backward loop
    {
      ptr--;
      compteur--;
      if (*ptr=='/')
        {
          compteur=0;
        }
    }
  int pas_de_point=0;
  if(compteur==0)
    {
      compteur=(int)nom_.size();
      pas_de_point=1 ;
    }
  std::string newname=nom_.substr(0,compteur);

  //searching for the number of digits we want to write
  int digits=0,diviseur=0;
  if(without_padding)
    {
      digits = (n==0) ? 1 : (int)std::lrint(std::truncl(log10(n)+1.0));
      diviseur = (int)std::lrint(std::truncl(pow(10, digits-1)));
    }
  else
    {
      if (Process::nproc()<=10000)
        {
          //the underscore will be taken into account later
          //digits=5;
          digits=4;
          diviseur=1000;
        }
      else if (Process::nproc()<=100000)
        {
          //the underscore will be taken into account later
          //digits=6;
          digits=5;
          diviseur=10000;
        }
      else if (Process::nproc()<=1000000)
        {
          //the underscore will be taken into account later
          //digits=7;
          digits=6;
          diviseur=100000;
        }
      else
        {
          Cerr << "Error in Nom::nom_me. Contact TRUST support." << finl;
          Process::exit();
        }

    }

  int prefix_len = 1; //for the underscore
  if(prefixe) prefix_len+=(int)strlen(prefixe);

  char *c_numero=new char[prefix_len+digits+1];
  int resultat;
  c_numero[0]='_';
  if(prefixe) strcpy(c_numero+1, prefixe);
  for (int i=prefix_len; i<prefix_len+digits; i++)
    {
      resultat=n/diviseur;
      char c= (char)((int)'0' + resultat); // on old compilers, '+' is not for char, always int ...
      c_numero[i]=c;
      n-=resultat*diviseur;
      diviseur/=10;
    }
  c_numero[prefix_len+digits]='\0';
  newname+=c_numero;
  if (pas_de_point==0)
    newname+=ptr;
  Nom new_name(newname);
  delete[] c_numero;
  return new_name;
}

/*! @brief Returns a name using the usual substr command. NOTE: deb = 1 means the first character of the string.
 *
 */
Nom Nom::substr_old(const int deb, const int la_longueur) const
{

  assert(deb > 0);
  assert(deb - 1 + la_longueur <= (int) nom_.size());
  Nom nouveau(nom_.substr(deb-1,la_longueur));
  return nouveau;
}

/*! @brief Returns the filename part if the name is in the form /toto/titi/filename.
 *
 */
Nom Nom::basename() const
{
  Nom dirname("");
  Nom the_basename(nom_);
  int iLength = (int)nom_.size();
  for (int i=0; i<iLength; i++)
    {
      dirname+=nom_[i];
      if (nom_[i]=='/' || nom_[i]=='\\')    // slash or backslash
        {
          the_basename.suffix(dirname);
          dirname="";
        }
    }
  return the_basename;
}

/*! @brief Returns a pointer to the character string of the name.
 *
 * @return (char*) pointer to the character string of the name
 */
Nom::operator const char*() const
{
  return nom_.c_str();
}

/*! @brief Compares a name with a character string using strcmp.
 *
 * @param (const Nom& un_nom)
 * @param (const char* const un_autre)
 * @return (int) 1 if the names are equal, 0 otherwise
 */
int operator ==(const Nom& un_nom, const char* const un_autre)
{
  int res_actu=(un_nom.nom_.compare(un_autre)==0);
  return res_actu;
}
int operator ==(const Nom& un_nom, const Nom& un_autre)
{
  return (un_nom==un_autre.getChar());
}
int operator ==(const char* const un_autre, const Nom& un_nom)
{
  return (un_nom == un_autre);
}

/*! @brief Compares a name with a character string.
 *
 * @param (const Nom& un_nom)
 * @param (const char* const un_autre)
 * @return (int) 1 if the names are different, 0 otherwise
 */
int operator !=(const Nom& un_nom, const char* un_autre)
{
  return ! (un_nom == un_autre);
}

int operator !=(const Nom& un_nom, const Nom& un_autre)
{
  return ! (un_nom == un_autre);
}

int operator !=(const char* const un_autre, const Nom& un_nom)
{
  return ! (un_autre == un_nom);
}

bool operator <(const Nom& n1, const Nom& n2)
{
  return n1.nom_.compare(n2.nom_) < 0;
}


/*! @brief Returns *this.
 *
 * @return (const Nom&) reference to the Nom
 */
const Nom& Nom::le_nom() const
{
  return *this;
}
