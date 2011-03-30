/*****************************************************************************
*
* Copyright (c) 2011, CEA
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of CEA, nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <UserFields.h>
#include <LataFilter.h>
#include <stdlib.h>
// ********************************************************************************
// METHODES OUTILS DE GESTION
//  (normalement, on n'a pas besoin de les modifier mais on peut en ajouter...)
// ********************************************************************************

// Implementation de la classe Geometry_handle
// (utiliser cette classe pour eviter d'avoir a faire des dynamic_cast
//  compliques et pour ne pas avoir a gerer get_geometry et release_geometry a la main)
// Exemple d'utilisation: voir interpoler_elem_vers_som()
Geometry_handle::Geometry_handle()
{
}
void Geometry_handle::set(LataFilter & lata_filter, const Domain_Id & id)
{
  lata_filter_ = lata_filter;
  geom_ = lata_filter.get_geometry(id);
}
Geometry_handle::Geometry_handle(Geometry_handle & handle)
{
  operator=(handle);
}
Geometry_handle & Geometry_handle::operator=(Geometry_handle & handle)
{
  reset();
  lata_filter_ = handle.lata_filter_;
  // Get another reference from the lata filter (to increment ref counter in the cache)
  geom_ = lata_filter_.valeur().get_geometry(handle.geom_.valeur().id_);
  return *this;
}
Geometry_handle::~Geometry_handle()
{
  reset();
}
void Geometry_handle::reset()
{
  if (geom_.non_nul())
    lata_filter_.valeur().release_geometry(geom_.valeur());
  geom_.reset();
  lata_filter_.reset();
}
const DomainUnstructured & Geometry_handle::geom()
{
  if (!geom_.non_nul()) {
    Journal() << "Internal error in Geometry_handle::geom() : nul pointer" << endl;
    throw;
  }
  const DomainUnstructured* ptr = dynamic_cast<const DomainUnstructured *>(&geom_.valeur());
  if (!ptr) {
    Journal() << "Error in Geometry_handle::geom() : domain " 
              << geom_.valeur().id_.name_ << " is not unstructured" << endl;
    throw;
  }
  return *ptr;
}
const DomainIJK & Geometry_handle::geom_ijk()
{
  if (!geom_.non_nul()) {
    Journal() << "Internal error in Geometry_handle::geom() : nul pointer" << endl;
    throw;
  }
  const DomainIJK* ptr = dynamic_cast<const DomainIJK *>(&geom_.valeur());
  if (!ptr) {
    Journal() << "Error in Geometry_handle::geom() : domain " 
              << geom_.valeur().id_.name_ << " is not IJK" << endl;
    throw;
  }
  return *ptr;
}
entier Geometry_handle::test_ijk()
{
  if (!geom_.non_nul()) {
    Journal() << "Internal error in Geometry_handle::geom() : nul pointer" << endl;
    throw;
  }
  const DomainIJK* ptr = dynamic_cast<const DomainIJK *>(&geom_.valeur());
  if (ptr)
    return 1;
  else
    return 0;
}

// Petite fonction outil qui construit l'objet LataFieldMetaData en changeant uniquement 
//  le nom du champ (dimension, localisation, geometrie, nombre de composantes sont identiques)
//  Le champ "source" est rempli avec une reference a source, on pourra donc appeler
//  get_champ_source() (voir interpoler_elem_vers_som pour un exemple)
// Voir new_fields_metadata() pour un exemple d'utilisation.
static LataFieldMetaData declare_new_name(const LataFieldMetaData & source, 
                                          const char * name)
{
  LataFieldMetaData dest = source;
  // Lorsqu'on demandera ce champ, on saura que c'est UserFields qui devra le calculer
  dest.source_ = "user_fields";
  // On change le nom du champ:
  dest.name_ = name;
  dest.uname_.set_field_name(name);
  // On remplit le champ source
  dest.source_field_ = source.uname_;
  
  return dest;
}

// Fonction identique a declare_new_name, mais pour declarer un champ avec une localisation
//  differente.
// Voir new_fields_metadata() pour un exemple d'utilisation.
static LataFieldMetaData declare_new_name_localisation(const LataFieldMetaData & source, 
                                                       const char * name,
                                                       LataField_base::Elem_som loc)
{
  LataFieldMetaData dest = source;
  // Lorsqu'on demandera ce champ, on saura que c'est UserFields qui devra le calculer
  dest.source_ = "user_fields";
  // On change le nom du champ et la localisation:
  dest.name_ = name;
  dest.uname_ = Field_UName(source.uname_.get_geometry(),
                            name,
                            LataField_base::localisation_to_string(loc));
  // On remplit le champ source
  dest.source_field_ = source.uname_;

  // En plus: je change la localisation:
  dest.localisation_ = loc;

  return dest;
}

// Fonction identique a declare_new_name, mais pour declarer un champ avec une localisation
//  differente de type vectoriel.
// Voir new_fields_metadata() pour un exemple d'utilisation.
static LataFieldMetaData declare_new_vector_field(const LataFieldMetaData & source, 
                                                  const char * name,
                                                  LataField_base::Elem_som loc,
                                                  const entier dim)
{
  LataFieldMetaData dest = source;
  // Lorsqu'on demandera ce champ, on saura que c'est UserFields qui devra le calculer
  dest.source_ = "user_fields";
  // On change le nom du champ et la localisation:
  dest.name_ = name;
  dest.uname_ = Field_UName(source.uname_.get_geometry(),
                            name,
                            LataField_base::localisation_to_string(loc));
  // On remplit le champ source
  dest.source_field_ = source.uname_;

  // En plus: je change la localisation:
  dest.localisation_ = loc;

  // et le type (vecteur)
  dest.component_names_.reset();
  dest.nb_components_ = dim;
  dest.is_vector_ = 1;

  return dest;
}

// Description: demande a la classe LataFilter le champ source du champ "id"
//  qui a ete declare quand on a appele declare_new_name() au debut
// Voir filtre_boite() pour un exemple d'utilisation.
FieldType UserFields::get_champ_source(const Field_Id & id)
{
  // Cherche la structure LataFieldMetaData du champ "id":
  const LataFieldMetaData & data = lata_filter_.valeur().get_field_metadata(id.uname_);
  Field_Id id2(data.source_field_, id.timestep_, id.block_);
  FieldType tmp;
  const LataField_base & field = lata_filter_.valeur().get_field(id2);
  const FieldType* ptr = dynamic_cast<const FieldType *>(&field);
  if (!ptr) {
    Journal() << "Error in UserFields::get_champ_source : field " << id.uname_ 
              << " is not a floattab" << endl;
    throw;
  }
  // Copie le contenu du champ dans un tableau temporaire:
  tmp = *ptr;
  // Libere le champ d'origine
  lata_filter_.valeur().release_field(field);
  return tmp;
}

// Description: demande a la classe LataFilter le champ de nom "nom" et dont
//  la geometrie, le pas de temps et la localisation sont celles de "id".
FieldType UserFields::get_champ(const Nom & nom, const Field_Id & id)
{
  FieldType tmp;
  // Construit un Field_Id identique, seul le nom du champ chamge:
  Field_Id id2(id);
  id2.uname_.set_field_name(nom);
  const LataField_base & field = lata_filter_.valeur().get_field(id2);
  const FieldType* ptr = dynamic_cast<const FieldType *>(&field);
  if (!ptr) {
    Journal() << "Error in UserFields::get_champ : field " << id.uname_ 
              << " is not a floattab" << endl;
    throw;
  }
  // Copie le contenu du champ dans un tableau temporaire:
  tmp = *ptr;
  // Libere le champ d'origine
  lata_filter_.valeur().release_field(field);

  return tmp;
}

// Description: idem, mais cherche un champ avec une localisation differente de id
FieldType UserFields::get_champ_loc(const Nom & nom, LataField_base::Elem_som loc, const Field_Id & id)
{
  FieldType tmp;
  // Construit un Field_Id identique, seul le nom du champ chamge:
  Field_Id id2(id);
  id2.uname_ = Field_UName(id.uname_.get_geometry(), nom, LataField_base::localisation_to_string(loc));
  
  const LataField_base & field = lata_filter_.valeur().get_field(id2);
  const FieldType* ptr = dynamic_cast<const FieldType *>(&field);
  if (!ptr) {
    Journal() << "Error in UserFields::get_champ : field " << id.uname_ 
              << " is not a floattab" << endl;
    throw;
  }
  // Copie le contenu du champ dans un tableau temporaire:
  tmp = *ptr;
  // Libere le champ d'origine
  lata_filter_.valeur().release_field(field);

  return tmp;
}

// Description: renvoie un objet Geometry_handle qui pointe sur le domaine
//  support du champ "id".
// Voir interpoler_elem_vers_som() pour un exemple d'utilisation
void UserFields::get_geometry(const Domain_Id & id, Geometry_handle & h)
{
  h.set(lata_filter_.valeur(), id);
}

// ********************************************************************************
//  METHODES OUTILS DE CALCUL
//  Ces methodes sont des fonctions qui calculent un champ en fonction d'un autre champ.
//  On peut les modifier comme on veut, en ajouter, etc... 
//  Lachez-vous...
// ********************************************************************************

// Description:
//  Fonction d'interpolation qui transforme un champ aux "elements"
//  en un champ aux "sommets".
//  Dans cet exemple, on a deux algorithmes selon que le champ est sur
//  un maillage ijk ou non.
//  La valeur aux sommets est la moyenne des valeurs sur les elements adjacents.
FieldType UserFields::interpoler_elem_vers_som(const Field_Id & id)
{
  // Recupere le champ a filtrer (champ aux elements)
  FieldType source = get_champ_source(id);

  FieldType resu;
  // Remplissage des meta-data du champ:
  resu.id_ = id;
  resu.component_names_ = source.component_names_;
  resu.localisation_ = LataField_base::SOM;
  resu.nature_ = source.nature_;
  
  // Recupere la geometrie (domaine ijk ou non structure) sur laquelle est definie
  //  le champ source:
  Geometry_handle geom;
  get_geometry(id, geom);
  ArrOfFloat poids;

  if (geom.test_ijk()) {
    const DomainIJK & dom = geom.geom_ijk();
    // Le code suivant marche en 1D, 2D et 3D:
    const entier nbsom = dom.nb_nodes();
    const entier nbcompo = source.data_.dimension(1);
    resu.data_.resize(nbsom, nbcompo);
    const entier nsom_x = dom.nb_som_dir(0);
    const entier nsom_y = dom.nb_som_dir(1);
    const entier nelem_x = dom.nb_elem_dir(0);
    const entier nelem_y = dom.nb_elem_dir(1);
    const entier nelem_z = dom.nb_elem_dir(2);
    poids.resize_array(nbsom);
    const entier ni = 2;
    const entier nj = (dom.dimension() > 1) ? 2 : 1;
    const entier nk = (dom.dimension() > 2) ? 2 : 1;

    // Avec les boucles imbriquees comme ceci, on parcourt tous les
    //  elements dans l'ordre croissant:
    // (l'indice de l'element (i,j,k) est :
    //    elem = (k * nelem_y + j) * nelem_x + i
    entier elem = 0;
    for (entier k = 0; k < nelem_z; k++) {
      for (entier j = 0; j < nelem_y; j++) {
        for (entier i = 0; i < nelem_x; i++) {
          if (dom.invalid_connections_.size_array() == 0 || dom.invalid_connections_[elem] == 0) {
            // Element valide:
            // Boucle sur les sommets de l'element
            const entier som0 = (k * nsom_y + j) * nsom_x + i;
            for (entier kk = 0; kk < nk; kk++) {
              for (entier jj = 0; jj < nj; jj++) {
                for (entier ii = 0; ii < ni; ii++) {
                  entier som = som0 + (kk * nsom_y + jj) * nsom_x + ii;
                  for (entier compo = 0; compo < nbcompo; compo++)
                    resu.data_(som, compo) += source.data_(elem, compo);
                  poids[som] += 1.;
                }
              }
            }
          }
          elem++;
        }
      }
    }
  } else {
    const DomainUnstructured & dom = geom.geom();

    const entier nbsom = dom.nb_nodes();
    const entier nbcompo = source.data_.dimension(1);
    resu.data_.resize(nbsom, nbcompo);
    poids.resize_array(nbsom);
    const IntTab & les_elem = dom.elements_;
    const entier n = les_elem.dimension(0);
    const entier m = les_elem.dimension(1);
    int i, j, k;
    for ( i = 0; i < n; i++) {
      for (j = 0; j < m; j++) {
        entier som = les_elem(i,j);
        for (k = 0; k < nbcompo; k++) {
          float x = source.data_(i, k);
          resu.data_(som, k) += x;
        }
        poids[som] += 1.;
      }
    }
  }
  const entier nbsom = poids.size_array();
  const entier nbcompo = resu.data_.dimension(1);
  for (entier i = 0; i < nbsom; i++)
    for (entier k = 0; k < nbcompo; k++)
      resu.data_(i, k) /= poids[i];

  return resu;
}





//  Attention: le constructeur par defaut n'initialise pas le vecteur !
class Vecteur3
{
public:
  Vecteur3() {};
  Vecteur3(const Vecteur3 & w) {
    v[0] = w.v[0]; v[1] = w.v[1]; v[2] = w.v[2];
  }
  Vecteur3(double x, double y, double z) {
    v[0] = x; v[1] = y; v[2] = z;
  }
  void  set(double x, double y, double z) {
    v[0] = x; v[1] = y; v[2] = z;
  }
  double length() const { return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); };
  Vecteur3(const DoubleTab & tab, entier i) {
    //assert(tab.line_size() == 3);
    assert(i >= 0 && i < tab.dimension_tot(0));
    const double *ptr = tab.addr() + i * 3;
    v[0] = ptr[0];
    v[1] = ptr[1];
    v[2] = ptr[2];
  }
  Vecteur3 & operator=(double x) {
    v[0] = x; v[1] = x; v[2] = x;
    return *this;
  }
  Vecteur3 & operator*=(double x) {
    v[0] *= x; v[1] *= x; v[2] *= x;
    return *this;
  }

  Vecteur3 & operator=(const Vecteur3 & w) {
    v[0] = w.v[0]; v[1] = w.v[1]; v[2] = w.v[2];
    return *this;
  }
  double               operator[](entier i) const { assert(i>=0 && i<3); return v[i]; }
  double &             operator[](entier i)       { assert(i>=0 && i<3); return v[i]; }
  inline        double norme_Linfini();
  static inline void   produit_vectoriel(const Vecteur3 & x, const Vecteur3 & y, Vecteur3 & resu);
  static inline double produit_scalaire(const Vecteur3 & x, const Vecteur3 & y);
  friend Vecteur3 operator-(const Vecteur3 &, const Vecteur3 &);
protected:
  double v[3];
};


inline void Vecteur3::produit_vectoriel(const Vecteur3 & x, const Vecteur3 & y, Vecteur3 & z)
{
  z.v[0] = x.v[1] * y.v[2] - x.v[2] * y.v[1];
  z.v[1] = x.v[2] * y.v[0] - x.v[0] * y.v[2];
  z.v[2] = x.v[0] * y.v[1] - x.v[1] * y.v[0];
}

inline double Vecteur3::produit_scalaire(const Vecteur3 & x, const Vecteur3 & y)
{
  double r = x.v[0] * y.v[0] + x.v[1] * y.v[1] + x.v[2] * y.v[2];
  return r;
}

// Description: norme L_infini, c'est le max des abs(v[i])
inline double Vecteur3::norme_Linfini()
{
  double x = fabs(v[0]);
  double y = fabs(v[1]);
  double z = fabs(v[2]);
  double resu = ((x > y) ? x : y);
  resu = ((resu > z) ? resu : z);
  return resu;  
}

inline Vecteur3 operator-(const Vecteur3 & x, const Vecteur3 & y)
{
  Vecteur3 z;
  z.v[0] = x.v[0] - y.v[0];
  z.v[1] = x.v[1] - y.v[1];
  z.v[2] = x.v[2] - y.v[2];
  return z;
}

double largest_angle_2(const DoubleTab& coords)
        
{
  if (((coords.dimension(0)!=4)&&(coords.dimension(0)!=3))||(coords.dimension(1)!=3))
    {
      Cerr<<" cas nn prevu"<<endl; 
      Process::exit();
    }
  int nb_face=coords.dimension(0);
  Vecteur3 normals[4];
  Vecteur3 edge[2],opp;
  edge[1].set(0,0,1);
  for (int n=0;n<nb_face;n++)
    {
     
      int prem=0;
      if (n==0) prem=1;
      int compteur=0;
      for (int s=0;s<nb_face;s++)
        {

          if ((s!=n) && (s!=prem))
            {
              edge[compteur].set(coords(s,0)-coords(prem,0),
                                 coords(s,1)-coords(prem,1),
                                 coords(s,2)-coords(prem,2));
              compteur++;
            }
        }
      if (compteur!=nb_face-2) abort();
      opp.set(coords(n,0)-coords(prem,0),
              coords(n,1)-coords(prem,1),
              coords(n,2)-coords(prem,2));
      Vecteur3::produit_vectoriel(edge[0],edge[1],normals[n]);
      //normals[n]=edge[0]*edge[1];
      normals[n]*=1./normals[n].length();
      if (Vecteur3::produit_scalaire(normals[n],opp)<0)
        normals[n]*=-1;
    }
  // on a les 4 normals orientés vers l'intérieur
  double max_pscal=-100;
  for (int n1=0;n1<nb_face;n1++)
    for (int n2=n1+1;n2<nb_face;n2++)
      {
        double pscal=Vecteur3::produit_scalaire(normals[n1],normals[n2]);
        //min_pscal=pscal;
        if (pscal>max_pscal)
          max_pscal=pscal;
      }
  double tet=acos(max_pscal)/acos(-1.)*180; // PL: acos(-1) ne compile pas sur de multiples plateformes
  
  tet=180-tet;
  return tet;
}



FieldType UserFields::calculer_angle(const Field_Id & id)
{
  // Recupere le champ a filtrer (champ aux elements)
  //  FieldType source = get_champ_source(id);
  
  FieldType resu;
  // Remplissage des meta-data du champ:
  resu.id_ = id;
  const LataFieldMetaData & data = lata_filter_.valeur().get_field_metadata(id.uname_);
  resu.component_names_ = data.component_names_;
  resu.localisation_ = data.localisation_;
  resu.nature_ = LataDBField::SCALAR;
  
  // Recupere la geometrie (domaine ijk ou non structure) sur laquelle est definie
  //  le champ source:
  Geometry_handle geom;
  get_geometry(id, geom);

  if (geom.test_ijk()) {
    Journal() <<    "non code" <<endl;
    throw;
  } else {
    const DomainUnstructured & dom = geom.geom();
    const entier nbcompo = dom.dimension();
    //poids.resize_array(nbsom);
    const IntTab & les_elem = dom.elements_;
    const entier n = les_elem.dimension(0);
 
    resu.data_.resize(n, nbcompo);
    
    const FloatTab& nodes_=dom.nodes_;
    int nb_som_elem=les_elem.dimension(1);
    DoubleTab coords(nb_som_elem,3);
    for ( int i = 0; i < n; i++) {
      for (int s=0;s<nb_som_elem;s++)
        for (int d=0;d<nodes_.dimension(1);d++)
          coords(s,d)=nodes_(les_elem(i,s),d);
      resu.data_(i, 0) = largest_angle_2(coords);
        
    }
  
  }
  return resu;
}

FieldType UserFields::calculer_normale(const Field_Id & id)
{

  // Recupere le champ a filtrer (champ aux elements)
  //  FieldType source = get_champ_source(id);

  FieldType resu;
  // Remplissage des meta-data du champ:
  resu.id_ = id;
  const LataFieldMetaData & data = lata_filter_.valeur().get_field_metadata(id.uname_);
  resu.component_names_ = data.component_names_;
  resu.localisation_ = data.localisation_;
  resu.nature_ = LataDBField::VECTOR;
  
  // Recupere la geometrie (domaine ijk ou non structure) sur laquelle est definie
  //  le champ source:
  Geometry_handle geom;
  get_geometry(id, geom);

  if (geom.test_ijk()) {
    Journal() <<    "non code" <<endl;
    throw;
  } else {
    const DomainUnstructured & dom = geom.geom();
    const entier nbcompo = dom.dimension();
    const IntTab & les_elem = dom.elements_;
    const entier n = les_elem.dimension(0);
 
    resu.data_.resize(n, nbcompo);
    
    const FloatTab& nodes_=dom.nodes_;
    
    ArrOfFloat v1( nbcompo),v2(nbcompo);
    ArrOfDouble nor(nbcompo);
    for ( int i = 0; i < n; i++) {
      // calcul de la normale
      entier som0 = les_elem(i,0);
      entier som1 = les_elem(i,1);
      for (int j=0;j<nbcompo;j++)
        v1[j]=nodes_(som1,j)-nodes_(som0,j);
      if (nbcompo==3)
        {
          entier som2 = les_elem(i,2);
          for (int j=0;j<nbcompo;j++)
            v2[j]=nodes_(som2,j)-nodes_(som0,j);
          
          nor[0]=v1[1]*v2[2]-v1[2]*v2[1];
          nor[1]=v1[2]*v2[0]-v1[0]*v2[2];
          nor[2]=v1[0]*v2[1]-v1[1]*v2[0];
          nor/=2.;
        }
      else
        {
          assert(nbcompo==2);
          nor[0]=v1[1];
          nor[1]=-v1[0];
          
        }
      for (int k = 0; k < nbcompo; k++) {
        resu.data_(i, k) = nor[k];
      }
       
    }
    
  }
  

  return resu;
}

// Description:
//  Fonction d'interpolation qui transforme un champ de vitesse VDF aux "faces"
//  en un champ aux "elements".
//  Ne fonctionne que sur les maillages ijk !
//  On attend un champ scalaire a une composante en entree (champ source)
//  et on fournit en sortie un champ vectoriel a "dimension" composantes.
FieldType UserFields::interpoler_faces_vdf_vers_elem(const Field_Id & id)
{
  // Recupere le champ a filtrer (champ aux elements)
  FieldType source = get_champ_source(id);
  
  if (source.localisation_ != LataField_base::FACES) {
    Journal() << "Error in UserFields::interpoler_faces_vdf_vers_elem: source field " << id.uname_.build_string()
              << " is not at faces !" << endl;
    throw;
  }

  // Get geometry:
  Geometry_handle geom;
  get_geometry(id, geom);
  if (!geom.test_ijk()) {
    Journal() << "Error in UserFields::interpoler_faces_vdf_vers_elem: geometry of field " << id.uname_.build_string()
              << " is not IJK" << endl;
    throw;
  }
  const DomainIJK & dom = geom.geom_ijk();

  const entier dim = dom.dimension();
  
  if (source.data_.dimension(1) != dim) {
    Journal() << "Error in UserFields::interpoler_faces_vdf_vers_elem: source field " << id.uname_.build_string()
              << " must have " << dim << " components !" << endl;
    throw;
  }

  FieldType resu;
  // Remplissage des meta-data du champ:
  resu.id_ = id;
  resu.component_names_.reset();
  resu.localisation_ = LataField_base::ELEM;
  resu.nature_ = LataDBField::VECTOR;

  // Le code suivant marche en 1D, 2D et 3D:
  const entier nbelem = dom.nb_elements();
  const entier nbcompo = dim;
  resu.data_.resize(nbelem, nbcompo);
  const entier nelem_x = dom.nb_elem_dir(0);
  const entier nelem_y = dom.nb_elem_dir(1);
  const entier nelem_z = dom.nb_elem_dir(2);
  const entier nfaces_x = dom.nb_som_dir(0);
  const entier nfaces_y = dom.nb_som_dir(1);
  // Avec les boucles imbriquees comme ceci, on parcourt tous les
  //  elements dans l'ordre croissant:
  // (l'indice de l'element (i,j,k) est :
  //    elem = (k * nelem_y + j) * nelem_x + i
  entier elem = 0;
  for (entier k = 0; k < nelem_z; k++) {
    for (entier j = 0; j < nelem_y; j++) {
      for (entier i = 0; i < nelem_x; i++) {
        if (dom.invalid_connections_.size_array() == 0 || dom.invalid_connections_[elem] == 0) {
          // Element valide:
          // Boucle sur les trois directions:
          for (entier dir = 0; dir < dim; dir++) {
            // indices des deux faces opposees de l'element dan la direction dir:
            const entier face1 = (k * nfaces_y + j) * nfaces_x + i;
            entier face2;
            if (dir == 0)
              face2 = face1 + 1;
            else if (dir == 1)
              face2 = face1 + nfaces_x;
            else
              face2 = face1 + nfaces_y * nfaces_x;
            // On fait la moyenne des vitesses sur les deux faces
            double v_moy = (source.data_(face1, dir) + source.data_(face2, dir)) * 0.5;
            resu.data_(elem, dir) = v_moy;
          }
        }
        elem++;
      }
    }
  }
  return resu;
}

// **********************************************************************************
// METHODES UTILISATEUR: ces methodes sont a mettre a jour en fonction des besoins
//  specifiques...
// **********************************************************************************

// Description: Constructeur de la classe.
//  Attention: penser a initialiser toutes les variables de la classe (options)
UserFields_options::UserFields_options()
{
  demie_largeur_filtre_boite_ = 1;
}

// Cette methode est appelee avec les options en ligne de commande ou sur la troisieme
//  ligne. Il faut renvoyer 0 si on ne comprend pas l'option, sinon 1.
entier UserFields_options::parse_option(const Nom & option)
{
  if (option.debute_par("demie_largeur_filtre_boite=")) {
    demie_largeur_filtre_boite_ = LataOptions::read_int_opt(option);
  } else {
    return 0;
  }
  return 1;
}

// Cette methode est appelee par lata2dx en ligne de commande pour afficher une aide.
// On peut decrire toutes les options...
void UserFields_options::print_help_option() const
{
  cerr << "Options provided by UserFields:" << endl;
  cerr << " demie_largeur_filtre_boite=N  (see filtre_boite implementation)" << endl;
}

// Description:
//  Cette methode est appelee par lata2dx au debut pour connaitre la liste
//  des champs que UserFields est capable de calculer.
//  fields_data contient en entree tous les champs deja fournis par lata2dx
//   (champs presents dans le fichier .lata, plus les champs resultant des operateurs
//    standards (regularize, dualmesh etc...)
//  On doit ajouter dans fields_data la description des champs supplementaires 
//   que UserFields peut calculer.
void UserFields::new_fields_metadata(LataFilter & filter,
                                     LataVector<LataFieldMetaData> & fields_data)
{
  lata_filter_ = filter;

  const Noms geoms = filter.get_exportable_geometry_names();
  
  const entier nb_geometries = geoms.size();
  
  for (int i = 0; i < nb_geometries; i++) {
    const LataGeometryMetaData data = filter.get_geometry_metadata(geoms[i]);
    
    // Si on a des faces, proposer la normale aux faces
    int topo_dim=data.dimension_;
    
    switch(data.element_type_) {
    case Domain::point:     topo_dim = 0; break;
    case Domain::line:      topo_dim = 1; break;
    case Domain::triangle:
    case Domain::quadri:    topo_dim = 2; break;
    case Domain::tetra:
    case Domain::prism6:
    case Domain::polyedre:
    case Domain::hexa:      topo_dim = 3; break;
    default:
      cerr << "avtlataFileFormat::PopulateDatabaseMetaData error: unknown element type" << endl;
      exit(-1);
    }
    if ((data.dimension_>1)&&(topo_dim!=data.dimension_)) {
      Journal(0)<<"Ajout de la normale"<<endl;
      LataFieldMetaData dest;
      dest.name_ = "NORMALE";
      dest.geometry_name_ = data.internal_name_;
      dest.component_names_.reset() ;
      
      dest.nb_components_ = data.dimension_;
      dest.is_vector_ = 1;
      dest.localisation_ = LataField_base::ELEM;
      dest.source_localisation_ = "ELEM";
      dest.source_ = "user_fields";
      // source_field_ inutile.
      
      dest.uname_ = Field_UName(dest.geometry_name_,
                                dest.name_,
                                LataField_base::localisation_to_string(dest.localisation_));
      fields_data.add(dest);
    }
    else
    if (data.element_type_==Domain::triangle||data.element_type_==Domain::tetra)
      
      {
        Journal(0)<<"Ajout de mesh_quality/LargestAngle"<<endl;
         LataFieldMetaData dest;
         dest.name_ = "mesh_quality/LargestAngle";
         dest.geometry_name_ = data.internal_name_;
         dest.component_names_.reset() ;
      
         dest.nb_components_ = 1;
         dest.is_vector_ = 0;
         dest.localisation_ = LataField_base::ELEM;
         dest.source_localisation_ = "ELEM";
         dest.source_ = "user_fields";
         // source_field_ inutile.
      
         dest.uname_ = Field_UName(dest.geometry_name_,
                                   dest.name_,
                                   LataField_base::localisation_to_string(dest.localisation_));
         fields_data.add(dest);
      }
  }
  // on laisse les lignes pour vérifier la compilation
  if ( 0) {
  const entier nb_fields_debut = fields_data.size();
  
  // On fait une boucle sur tous les champs disponibles dans le filtre
  //  (nb_fields_debut est le nombre de champs existant avant qu'on 
  //   commence a en ajouter dans le tableau fields_data)

  for (int i_in = 0; i_in < nb_fields_debut; i_in++) 
    {
      // On cherche si le champ de temperature aux elements existe
      //  sur une geometrie IJK (Motcle permet d'ignorer majuscule/minuscule)
      const LataFieldMetaData data = fields_data[i_in];

      // Les deux if suivants sont des EXEMPLES

      if (Motcle(data.name_) == "TEMPERATURE"
          && data.localisation_ == LataField_base::ELEM
          && Motcle(data.geometry_name_).finit_par("_IJK"))
        {
          // On declare un champ identique qui s'appelle MOYENNE_TEMPERATURE
          fields_data.add(declare_new_name(data, "MOYENNE_TEMPERATURE"));
        }

      // Si le champ est aux elements, on propose une interpolation aux sommets
      // On reconnaitra le champ parce que son nom finira par elem_vers_som (voir get_field())
      if (data.localisation_ == LataField_base::ELEM)
        {
          Nom nom = data.name_;
          nom += "_elem_vers_som";
          fields_data.add(declare_new_name_localisation(data, nom, LataField_base::SOM));
        }

      // Si le champ est aux faces et le maillage est ijk, on propose
      //  une interpolation aux elements
      if (data.localisation_ == LataField_base::FACES
          && Motcle(data.geometry_name_).finit_par("_IJK"))
        {
          Nom nom = data.name_;
          nom += "_faces_vers_elem";
          // Le champ aux faces a deja dimension composantes
          const entier dim = data.nb_components_;
          fields_data.add(declare_new_vector_field(data, nom, LataField_base::ELEM, dim));
        }
    }
  }
}

// Description:
//  Cette methode publique est appelee par lata2dx pour obtenir les champs declares dans
//  new_fields_metadata. Il faut tester "id" et calculer le champ demande.
//  On a le droit d'appeler get_champ() pour obtenir d'autres champs.
FieldType UserFields::get_field(const Field_Id & id)
{
  // Convertit le nom du champ en majuscules:
  Motcle nom(id.uname_.get_field_name());

  // Ces deux lignes sont des EXEMPLES (a remplacer par les champs qu'on veut
  //  effectivement calculer)
  if (nom == "moyenne_temperature")    return filtre_boite(id);
  else if (nom.finit_par("_elem_vers_som")) return interpoler_elem_vers_som(id);
  else if (nom.finit_par("_faces_vers_elem")) return interpoler_faces_vdf_vers_elem(id);
  else if (nom.debute_par("NORMALE")) return calculer_normale(id);
  else if(nom.debute_par("mesh_quality/LargestAngle")) return calculer_angle(id);
  // Ceci doit rester:
  else {
    Journal() << "Error in UserFields::get_field: unknown field " << nom << endl;
    throw;
  }
}

class FiltreSpatial
{   
public:
  FiltreSpatial(LataFilter & lata, const Domain_Id & id, entier demi_pas) :
    demi_pas_(-1), pbDim_(-1), nx_(-1), ny_(-1), nz_(-1), dx_(-1.), dy_(-1.), dz_(-1.)
  {
    init(lata, id, demi_pas); 
  }
  FieldType filtrer(const FieldType & f, const Field_Id & id) const;
  FieldType gradient(const FieldType & f, const Field_Id & id) const;
  float volume() const { return dx_ * dy_ * dz_; }
protected:
  void init(LataFilter & lata, const Domain_Id & id, entier demi_pas);
  FloatTab calculer_somme_dir(const FloatTab & src, const int dir) const;
  FloatTab annu_bord(const FloatTab & input, int epaisseur) const;
  int ijk_index(int i, int j, int k) const {
    if (i < 0)
      i = 0;
    else if (i >= nx_)
      i = nx_-1;
    if (j < 0)
      j = 0;
    else if (j >= ny_)
      j = ny_ - 1;
    if (k < 0)
      k = 0;
    else if (k >= nz_)
      k = nz_-1; 
    return k * ny_ * nx_ + j * nx_ + i;
  } 

  // Tableau: pour chaque element, 1 s'il est INVALIDE, 0 s'il est OK
  ArrOfBit invalid_connections_;

  entier demi_pas_;
  int pbDim_; // dimension
  int nx_;
  int ny_;
  int nz_;
  float dx_;
  float dy_;
  float dz_;
};

void FiltreSpatial::init(LataFilter & lata, const Domain_Id & id, entier demi_pas)
{
  const Domain & dom = lata.get_geometry(id);
  const DomainIJK * ptr = dynamic_cast<const DomainIJK *>(&dom);
  if (!ptr) {
    Journal() << "Error in FiltreSpatial::init : domain " << id.name_ << " is not IJK" << endl;
    throw;
  }
  demi_pas_ = demi_pas;
  pbDim_ = ptr->coord_.size();
  nx_ = ptr->coord_[0].size_array() - 1;
  ny_ = ptr->coord_[1].size_array() - 1;
  if (pbDim_ == 3)
    nz_ = ptr->coord_[2].size_array() - 1;
  else
    nz_ = 1;

  dx_ = ptr->coord_[0][1] - ptr->coord_[0][0];
  dy_ = ptr->coord_[1][1] - ptr->coord_[1][0];
  if (pbDim_ == 3)
    dz_ = ptr->coord_[2][1] - ptr->coord_[2][0];
  else
    dz_ = 1.;

  invalid_connections_ = ptr->invalid_connections_;

  if (invalid_connections_.size_array() == 0) {
    invalid_connections_.resize_array(ptr->nb_elements());
    invalid_connections_ = 0;
  }

  lata.release_geometry(dom);
}

FloatTab FiltreSpatial::calculer_somme_dir(const FloatTab & src, const int dir) const
{
  const int n = src.dimension(0);
  const int nb_compo = src.dimension(1);
  FloatTab tmp;
  tmp.resize(n, nb_compo);

  int index_resu = 0;
  for (int k = 0; k < nz_; k++) {
    for (int j = 0; j < ny_; j++) {
      for (int i = 0; i < nx_; i++) {
        for (int count = -demi_pas_; count <= demi_pas_; count++) {
          int index;
          switch(dir) {
          case 0: index = ijk_index(i+count, j, k); break;
          case 1: index = ijk_index(i, j+count, k); break;
          case 2: index = ijk_index(i, j, k+count); break;
          default:
            throw;
          }

          if (invalid_connections_[index] == 1 && dir == 0) {
            // element invalide !
          } else {
            // element ok !
            for (int compo = 0; compo < nb_compo; compo++)
              tmp(index_resu, compo) += src(index, compo);
          }
        }
        index_resu++;
      }
    }
  }
  return tmp;
}

FieldType FiltreSpatial::filtrer(const FieldType & source, const Field_Id & id) const
{
  // On copie tout pour avoir les noms des composantes, localisation etc...
  FieldType resu = source;
  resu.id_ = id;

  FloatTab somme_x = calculer_somme_dir(source.data_, 0);
  FloatTab somme_y = calculer_somme_dir(somme_x, 1);
  if (pbDim_ == 3)
    resu.data_ = calculer_somme_dir(somme_y, 2);
  else
    resu.data_ = somme_y;

  entier pas = demi_pas_ * 2 + 1;
  double fact = pas * pas;
  if (pbDim_ == 3)
    fact *= pas;
  resu.data_ *= (1. / fact);

  return resu;
}

FieldType UserFields::filtre_boite(const Field_Id & id)
{
  FieldType source = get_champ_source(id);

  FiltreSpatial filtre(lata_filter_.valeur(), id, opt_.demie_largeur_filtre_boite_);

  FieldType resu = filtre.filtrer(source, id);

  return resu;
}

