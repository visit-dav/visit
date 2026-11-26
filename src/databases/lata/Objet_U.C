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

#include <Objet_U.h>
#include <Nom.h>

#ifndef LATATOOLS
#include <Memoire.h>
#include <Interprete_bloc.h>
#include <Lire.h>

int Objet_U::dimension=0;
int Objet_U::format_precision_geom=11;
int Objet_U::axi=0;
int Objet_U::bidim_axi=0;
int Objet_U::static_obj_counter_=0;
int Objet_U::DEACTIVATE_SIGINT_CATCH=0;
Interprete* Objet_U::l_interprete=0;

bool Objet_U::disable_TU=false; ///< Flag to disable or not the writing of the .TU files
bool Objet_U::stat_per_proc_perf_log=false; ///< Flag to enable the writing of the statistics detailed per processor in _csv.TU file
Type_info Objet_U::info_obj("Objet_U");

// Allows debugging by tracing back to the creation of a given object:
static int object_id_to_track = -1;

// Utility function to intercept the creation of an object.
// NOTE: since gcc3, the compiler generates multiple routines for
//  each constructor: at least Objet_U::Objet_U() and Objet_U::Objet_U$base().
//  This makes it difficult to use breakpoints in those routines.
//  Place the breakpoint here:
static void object_trap(int obj_id)
{
  Cerr << "Objet_U::Objet_U() : object_id_=" << obj_id << finl;
}

/*! @brief Default constructor: assigns a unique identifier to the object (object_id_) and registers the object in "memory" by giving it a _num_obj_ number.
 *
 *    object_id_ is very useful for debugging.
 *
 */
Objet_U::Objet_U() :  Process(),
  object_id_(static_obj_counter_++)
{
  int obj_id = object_id_;
  if (obj_id == object_id_to_track)
    {
      object_trap(obj_id);
    }
  Memoire& memoire = Memoire::Instance();
  _num_obj_ = memoire.add(this);
}

/*! @brief Copy constructor.
 *
 * Creates a new object number (does not copy the number from the other object!).
 *
 */
Objet_U::Objet_U(const Objet_U&) : Process(),
  object_id_(static_obj_counter_++)
{
  int obj_id = object_id_;
  if (obj_id == object_id_to_track)
    {
      object_trap(obj_id);
    }
  Memoire& memoire = Memoire::Instance();
  _num_obj_ = memoire.add(this);
}

/*! @brief Assignment operator: does nothing (the number and identifier are preserved).
 *
 */
const Objet_U& Objet_U::operator=(const Objet_U&)
{
  return *this;
}

/*! @brief Returns the unique identifier of the object (object_id_).
 *
 * @return The unique object identifier.
 */
int Objet_U::get_object_id() const
{
  return object_id_;
}

/*! @brief Returns the string identifying the class.
 *
 * @return Name identifying the class of the object.
 */
const Nom& Objet_U::que_suis_je() const
{
  return get_info()->name();
}

/*! @brief Reads non-simple-type parameters of an Objet_U from an input stream.
 *
 * @param motlu The name of the keyword to read.
 * @param is An input stream.
 * @return Negative value if the keyword is not understood, otherwise positive value.
 */
int Objet_U::lire_motcle_non_standard(const Motcle& motlu, Entree& is)
{
  Cerr << "The method " << __func__ << " must be overloaded in " << que_suis_je() << " !!!!" << finl;
  Process::exit();
  return -1;
}

/*! @brief Returns type information for the Objet_U.
 *
 * @return Structure containing type information for the Objet_U.
 */
const Type_info*  Objet_U::get_info() const
{
  return &info_obj;
}


/*! @brief Returns type information for the Objet_U (static version).
 *
 * @return Structure containing type information for the Objet_U.
 */
const Type_info*  Objet_U::info()
{
  return &info_obj;
}

/*! @brief Returns a constant reference to the case name. This method is static.
 *
 * @return Constant reference to the case name.
 */
const Nom& Objet_U::nom_du_cas()
{
  return get_set_nom_du_cas();
}

/*! @brief Returns a non-constant reference to the case name (to allow modification). This method is static.
 *
 * @return Non-constant reference to the case name.
 */
Nom& Objet_U::get_set_nom_du_cas()
{
  // This static object is constructed on the first call to the function.
  static Nom nom_du_cas_;
  return nom_du_cas_;
}

/*! @brief Method added for casting in Python.
 *
 * @param obj The object to cast.
 * @return Constant reference to the object.
 */
const Objet_U& Objet_U::self_cast(const Objet_U& obj)
{
  return ref_cast_non_const(Objet_U,obj);
}
Objet_U& Objet_U::self_cast(Objet_U& obj)
{
  return ref_cast_non_const(Objet_U,obj);
}

/*! @brief Changes the internal number of the Objet_U.
 *
 * @param new_ones Array of new numbers, indexed by old numbers.
 * @return The new number assigned to the Objet_U.
 */
int Objet_U::change_num(const int* const new_ones)
{
  return _num_obj_=new_ones[_num_obj_];
}


/*! @brief Returns the type name of the Objet_U.
 *
 * @return Character string representing the type of the Objet_U.
 */
const char* Objet_U::le_type() const
{
  return get_info()->name();
}

/*! @brief Associates the Objet_U with another Objet_U. Virtual method to override.
 *
 * @param obj The Objet_U to associate with.
 * @return Return code.
 */
int Objet_U::associer_(Objet_U& obj)
{
  return 0;
}

static Interprete& interprete_bidon()
{
  static Lire bidon;
  return bidon;
}
// BM: to be removed eventually (use Interprete::objet instead)
const Interprete& Objet_U::interprete() const
{
  return interprete_bidon();
}

// BM: to be removed eventually (use Interprete::objet instead)
Interprete& Objet_U::interprete()
{
  return interprete_bidon();
}

/*! @brief Returns x.est_egal_a(y).
 *
 * @param x First Objet_U for comparison.
 * @param y Second Objet_U for comparison.
 * @return 1 if the two Objet_U are equal, 0 otherwise.
 */
int operator==(const Objet_U& x, const Objet_U& y)
{
  return x.est_egal_a(y);
}

/*! @brief Returns 1 - x.est_egal_a(y).
 *
 * @param x First Objet_U for comparison.
 * @param y Second Objet_U for comparison.
 * @return 1 if the two Objet_U are different, 0 if they are equal.
 */
int operator!=(const Objet_U& x, const Objet_U& y)
{
  return (1-(x.est_egal_a(y)));
}

#endif   // LATATOOLS

double Objet_U::precision_geom = 1e-10;

/*! @brief Destructor. Removes the object from the list of objects registered in "memory".
 *
 */
Objet_U::~Objet_U()
{
#ifndef LATATOOLS
  Memoire& memoire = Memoire::Instance();
  memoire.suppr(_num_obj_);
  _num_obj_ = -2; // Paranoia
#endif
}

/*! @brief Returns the index of the object in Memoire::data.
 *
 * @return The object's index in memory.
 */
int Objet_U::numero() const
{
#ifndef LATATOOLS
  return _num_obj_;
#else
  return 0;
#endif
}

/*! @brief Writes the object to an output stream. Virtual method to override.
 *
 * @param s Output stream.
 * @return Modified output stream.
 */
Sortie& Objet_U::printOn(Sortie& s) const
{
  return s;
}


/*! @brief Reads an Objet_U from an input stream. Virtual method to override.
 *
 * @param s Input stream.
 * @return Modified input stream.
 */
Entree& Objet_U::readOn(Entree& s)
{
  return s;
}

/*! @brief Returns 1 if x and *this are the same instance (same memory address).
 *
 * @param x The object to compare with.
 * @return 1 if same instance, 0 otherwise.
 */
int Objet_U::est_egal_a(const Objet_U& x) const
{
#ifndef LATATOOLS
  int resultat;
  if (&x==this)
    resultat = 1;
  else
    resultat = 0;
  return resultat;
#else
  return 0;
#endif
}

/*! @brief Returns the name of the Objet_U. Virtual method to override: returns "neant" in this implementation.
 *
 * @return The name of the Objet_U.
 */
const Nom& Objet_U::le_nom() const
{
  static Nom inconnu="neant";
  return inconnu;
}

/*! @brief Assigns a name to the Objet_U. Virtual method to override.
 *
 * @param nom The name to assign to the Objet_U.
 */
void Objet_U::nommer(const Nom& nom)
{
}

/*! @brief Restores an Objet_U from an input stream. Virtual method to override.
 *
 * @param is Input stream to use for restoration.
 * @return Return code.
 */
int Objet_U::reprendre(Entree& is)
{
#ifndef LATATOOLS
  Cerr << "The method " << __func__ << " must be overloaded in " << que_suis_je() << " !!!!" << finl;
  Process::exit();
#endif
  return 1;
}

/*! @brief Saves an Objet_U to an output stream. Virtual method to override.
 *
 * @param os Output stream to use for saving.
 * @return Return code.
 */
int Objet_U::sauvegarder(Sortie& os) const
{
#ifndef LATATOOLS
  Cerr << "The method " << __func__ << " must be overloaded in " << que_suis_je() << " !!!!" << finl;
  Process::exit();
#endif
  return 0;
}
