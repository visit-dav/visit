/****************************************************************************
* Copyright (c) 2022, CEA
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

#ifndef UserFields_H
#define UserFields_H

#include <LataTools.h>
#include <LataStructures.h>
#include <TRUSTArray.h>

template<class F> class Field;
typedef Field<BigFloatTab> FieldType;
class Field_Id;
class LataFilter;
struct LataFieldMetaData;
class DomainUnstructured;
class DomainIJK;

// Description: classe outil pour acceder a une geometrie dans LataFilter.
//  La geometrie est chargee en memoire quand cet objet est cree,
//  elle est dechargee quand il est detruit.
// Exemple d'utilisation dans UserFields::interpoler_elem_vers_som
class Geometry_handle
{
public:
  Geometry_handle();
  Geometry_handle(Geometry_handle&);
  Geometry_handle& operator=(Geometry_handle&);
  ~Geometry_handle();
  void set(LataFilter &filter, const Domain_Id&);
  const DomainUnstructured& geom();
  const DomainIJK& geom_ijk();
  bool test_ijk();
protected:
  void reset();
  LataRef<LataFilter> lata_filter_;
  LataRef<const Domain> geom_;
};

class UserFields_options
{
public:
  UserFields_options();
  bool parse_option(const Nom&);
  void print_help_option() const;

  // Exemple de parametre en option (commentaires bienvenus !)
  // demie-largeur du filtre_boite en mailles
  int demie_largeur_filtre_boite_;
};

class UserFields
{
public:
  void set_options(const UserFields_options &opt) { opt_ = opt; }
  void new_fields_metadata(LataFilter &filter, LataVector<LataFieldMetaData> &fields_data);
  FieldType get_field(const Field_Id &id);
  Size_t compute_memory_size() { return 0; }

protected:
  // Declaration de methodes outils
  FieldType get_champ_source(const Field_Id &id);
  FieldType get_champ(const Nom &nom, const Field_Id &id);
  FieldType get_champ_loc(const Nom &nom, LataField_base::Elem_som loc, const Field_Id &id);
  void get_geometry(const Domain_Id &id, Geometry_handle&);

  FieldType filtre_boite(const Field_Id &id);
  FieldType calculer_normale(const Field_Id &id);
  FieldType calculer_angle(const Field_Id &id);
  FieldType interpoler_elem_vers_som(const Field_Id &id);
  FieldType interpoler_faces_vdf_vers_elem(const Field_Id &id);

  // Reference a la classe LataFilter (pour recuperer les champs sources)
  LataRef<LataFilter> lata_filter_;

  UserFields_options opt_;
};

#endif /* UserFields_H */
