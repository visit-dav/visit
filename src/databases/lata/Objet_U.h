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


#ifndef Objet_U_included
#define Objet_U_included

//     Some macros may #define min, max or throw — undo them here
#undef min
#undef max
#undef throw

#include <assert.h>
#include <DecBaseInst.h>

#ifdef LATATOOLS
#include <LataJournal.h>
#include <Sortie.h>
#include <Entree.h>
#define Cerr Journal()
#define finl std::endl
#define tspace " "

#else
#include <EntreeSortie.h>
#endif

#include <Process.h>

class Type_info;
class Interprete;
class Motcle;
class Param; // need this forward for pure virtual set_param

/*! @brief Base class for TRUST objects (Objet_U).
 *
 *      In classes derived from Objet_U, a Declare_instanciable or Declare_base macro is always added,
 *       which gives objects the following properties:
 *      An Objet_U can be read from an Entree or written to a Sortie
 *       (standard I/O, .data dataset, disk file, memory buffer, parallel communication buffer).
 *       The readOn and printOn methods must therefore always be implemented.
 *      An Objet_U of any type can be instantiated using a character string that identifies it
 *       (que_suis_je()), see OWN_PTR::typer.
 *      An Objet_U can be "saved" or "resumed" on disk (in the sense of checkpoint/restart).
 *       These operations differ from readOn/printOn because they optionally allow redistribution
 *       of parallel data or version changes.
 *      An Objet_U, if "Declare_instanciable", can be dynamically created and read from the TRUST
 *       dataset (via readOn). It then has the name (le_nom()) assigned in the dataset
 *       (see Interprete_bloc and Lire classes).
 *      An Objet_U is subject to special memory management by the kernel for debugging and optimization
 *       (specific operations at creation, destruction and copy).
 *
 * @sa Memoire Objet_U_ptr Process, Abstract class
 */
class Objet_U : public Process
{
public:
  friend class Sortie;
  friend class Entree;

  ~Objet_U() override;
  int        numero() const;
  virtual int    duplique()  const =0;
  virtual Sortie&   printOn(Sortie& ) const;
  virtual Entree&   readOn(Entree& ) ;
  virtual unsigned  taille_memoire() const =0;
  virtual int    est_egal_a(const Objet_U&) const;
  virtual const Nom& le_nom() const;
  static double precision_geom;

  virtual void       nommer(const Nom&);
  virtual int    reprendre(Entree& ) ;
  virtual int    sauvegarder(Sortie& ) const;


#ifndef LATATOOLS            // All the below is not needed in lata_tools:
  int        get_object_id() const;

  // Elie Saikali: add this to statically test whether class templates are REF/OWN_PTR or plain objects!
  static constexpr bool HAS_POINTER = false;

  static int dimension;
  static int format_precision_geom;
  static int axi;
  static int bidim_axi;
  static int DEACTIVATE_SIGINT_CATCH; // flag to not enter the overloaded function signal_callback_handler
  static const Nom& nom_du_cas();
  static Nom& get_set_nom_du_cas();

  static Type_info         info_obj;
  virtual const Type_info* get_info() const;
  static const Type_info*  info();

  const Nom&         que_suis_je() const;
  const char*        le_type() const;

  friend int     operator ==(const Objet_U&, const Objet_U&);
  friend int     operator !=(const Objet_U&, const Objet_U&);
  virtual int    change_num(const int* const );
  virtual int lire_motcle_non_standard(const Motcle& motlu, Entree& is);
  virtual int    associer_(Objet_U&) ;
  const Interprete& interprete() const;
  Interprete& interprete();
  /* method added for casting in Python */
  static const Objet_U& self_cast(const Objet_U&);
  static Objet_U& self_cast( Objet_U&);

  static bool disable_TU; // Flag to disable the writing of the .TU files
  static bool stat_per_proc_perf_log; // Flag to enable the writing of the statistics detailed per processor in _csv.TU file
protected:
  Objet_U();
  Objet_U(const Objet_U&);
  const Objet_U& operator=(const Objet_U&);

  // not pure virtual because that would need adapting every derived of Objet_U
  // and not needed since the macro Declare_xxx_with_param forces to define this anyway
  // eventually, should only be implemented in derived of Objet_U_With_Params
  virtual void set_param(Param&) const {}

private:
  // Object number (index of the object in Memoire::data).
  // This number may change between construction and destruction.
  int _num_obj_;
  // Unique identifier of the object (assigned by the constructor
  // and never modified afterwards).
  const int object_id_;

  // Counter of created objects (incremented by the constructor).
  static int static_obj_counter_;
  static Interprete* l_interprete;

#endif
};

#ifndef LATATOOLS
int operator==(const Objet_U& x, const Objet_U& y) ;
int operator!=(const Objet_U& x, const Objet_U& y) ;
#endif

#endif


