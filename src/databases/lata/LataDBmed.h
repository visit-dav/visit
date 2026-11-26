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

//#define WITH_MEDLOADER
#ifndef WITH_MEDLOADER
void LataDB::read_master_file_med(const char *prefix, const char *filename)
{
  Journal() << "MED PLUGIN not compiled!" << endl;
  throw;
}

template <class C_Tab>
void LataDB::read_data2_med_(const LataDBField& fld, C_Tab * const data, // const pointer to non const data !
                             Size_t debut, Size_t n) const
{
  Journal() << "MED PLUGIN not compiled!" << endl;
  throw;
}
#else

#include <medcoupling++.h>
#include <MEDLoader.hxx>
#include <MEDCouplingMemArray.hxx>
#include <MEDCouplingUMesh.hxx>
#include <MEDCouplingFieldDouble.hxx>
#include <CellModel.hxx>
#include <MEDFileField.hxx>

using std::vector;
using std::pair;
using std::string;


Nom latadb_name_from_type_geo(const med_geometry_type& type_geo)
{
  Nom type_elem;
  switch(type_geo)
    {
    case MED_QUAD4:
      type_elem="Rectangle";
      break;
    case MED_HEXA8:
      type_elem="Hexaedre";
      break;
    case MED_TRIA3:
      type_elem="Triangle";
      break;
    case MED_TETRA4:
      type_elem="Tetraedre";
      break;
    case MED_PENTA6:
      type_elem="Prisme";
      break;
    case MED_POLYHEDRON:
      type_elem="Polyedre";
      break;
    case MED_POLYGON:
      type_elem="Polygone";
      break;
    case MED_SEG2:
      type_elem="Segment";
      break;
    case MED_POINT1:
      type_elem="Point";
      break;
    default:
      Cerr<<"type_geo " << (int)type_geo <<" is not a supported element."<<finl;
      throw;
      break;
    }
  return type_elem;
}


// passage de la connectivite trio a MED si sens=1
// de MED a trio si sens=-1
ArrOfInt renum_conn(const LataDB::Element& type)
{
  //  cerr<<"type elem "<<type_elem<<endl;
  ArrOfInt filter;
  if (type==LataDB::quadri )
    {
      filter.resize_array(4) ;

      filter[0] = 0 ;
      filter[1] = 2 ;
      filter[2] = 3 ;
      filter[3] = 1 ;
    }

  if (type== LataDB::hexa  )
    {
      filter.resize_array(8) ;

      filter[0] = 0 ;
      filter[1] = 2 ;
      filter[2] = 3 ;
      filter[3] = 1 ;
      filter[4] = 4 ;
      filter[5] = 6 ;
      filter[6] = 7 ;
      filter[7] = 5 ;
    }
  return filter;
}

extern med_geometry_type typmai3[MED_N_CELL_FIXED_GEO];

void latadb_get_info_mesh_med(const char* filename,const char* meshname,med_geometry_type& type_geo,trustIdType& ncells,
                              trustIdType& nnodes,int& spacedim, int& nbcomp,bool& is_structured, std::vector<int>& NIJK)
{
  using namespace MEDCoupling;

  is_structured=false;
  int meshDim;
  try
    {
      // In 32b, MEDCoupling is 32b, but if we are in LATATOOLS mode (lata_tools and VisIt plugin, trustIdType is 64b ...)
      mcIdType tmp_nod;
      auto res = GetUMeshGlobalInfo(filename, meshname, meshDim, spacedim, tmp_nod);
      nnodes = tmp_nod;

      // on prend que la dimension la plus grande et on verifie que l'on a qu'un type elt
      if (res.size()>1)
        {
          cerr<<"warning multi dimension in "<<meshname<<endl;
          //throw;
        }
      if (res[0].size()>1)
        {
          cerr<<"error multi elements in "<<meshname<<endl;
          throw;
        }
      type_geo=typmai3[res[0][0].first];

      if ((type_geo==MED_POLYGON)||(type_geo==MED_POLYHEDRON))
        {
          //on est force de lire le maillage pour avoir le bon nombre de cellules
          MCAuto<MEDCouplingUMesh> mesh(ReadUMeshFromFile(filename,meshname));
          ncells = mesh->getNumberOfCells();
          const mcIdType *idx = mesh->getNodalConnectivityIndex()->getConstPointer();
          nbcomp = 0;
          for (trustIdType i = 0; i < ncells; i++)
            {
              int c = (int)(idx[i + 1] - idx[i] - 1);
              if (nbcomp < c)
                nbcomp = c;
            }
        }
      else
        ncells=res[0][0].second;
    }
  catch (...)
    {
      // No UMesh try CMesh
      MEDCouplingMesh* mesh = ReadMeshFromFile(filename, meshname);
      /*
        type_geo,int& ncells,int& nnodes,int& spacedim, int &nbcomp
       */
      MCAuto<MEDCouplingCMesh> cmesh (dynamic_cast<MEDCouplingCMesh*>(mesh));
      if (cmesh == nullptr)
        {
          cerr << "Error: can not read CMesh ... " << meshname << endl;
          throw;
        }
      spacedim=cmesh-> getSpaceDimension() ;

      // See comment of DomainIJK:
      std::vector<mcIdType> nijk_64 = cmesh->getNodeGridStructure();
      for (int i=0; i<nijk_64.size(); i++)
        NIJK[i] = (int)nijk_64[i]; // downcast

      ncells=mesh->getNumberOfCells();
      nnodes=mesh->getNumberOfNodes();

      if (spacedim==3)
        type_geo =MED_HEXA8;
      else if (spacedim==2)
        type_geo =MED_QUAD4;
      else
        abort();
      is_structured=true;
      //abort();
      return;
    }
}


// Description: Reads the .lata database in the given file indicating than the
//  associated data files will be found in directory "prefix".
//  If not empty, "prefix" must finish with a '/'.
//  For "prefix" and "filename", if they do not begin with '/', are relative to pwd.
// Exceptions:
//  BAD_HEADER  means that the header found in this stream is not LATA_V2
//  READ_ERROR  means that an error has been found in the file (premature eof,
//              io error, bad keyword, ...)
//  FILE_NOT_FOUND means that, well, the lata file could not be opened
void LataDB::read_master_file_med(const char *prefix, const char *filename)
{
  Journal() << "MED " << MED_NUM_MAJEUR << "." << MED_MINOR_NUM << "." << MED_RELEASE_NUM << " PLUGIN ! " << endl;

  // Defaults for lataV1
  default_type_int_.msb_ = LataDBDataType::ASCII;
  default_type_int_.type_ = LataDBDataType::INT32;
  default_type_int_.array_index_ = LataDBDataType::F_INDEXING;
  default_type_int_.data_ordering_ = LataDBDataType::C_ORDERING;
  default_type_int_.fortran_bloc_markers_ = LataDBDataType::BLOC_MARKERS_SINGLE_WRITE;
  default_type_int_.bloc_marker_type_ = LataDBDataType::INT32;
  default_float_type_ = LataDBDataType::REAL32;

  // Create timestep 0 (global domain and fields)
  timesteps_.add(LataDBTimestep());

  // on ajoute les geom
  // on verra apres pour les champs elem et som
  vector<string> geoms= MEDCoupling::GetMeshNames(filename);

  vector<double> times;
  LataDBTimestep  table;
  bool first=true;

  for (int i=0; i<geoms.size(); i++)
    {
      LataDBGeometry dom;
      dom.timestep_ = timesteps_.size()-1;
      dom.name_=geoms[i];
      med_geometry_type type_geo;
      trustIdType ncells,nnodes;
      int nbcomp, spacedim;
      bool is_structured;
      std::vector<int> NIJK;
      latadb_get_info_mesh_med(filename,geoms[i].c_str(),type_geo,ncells,nnodes,spacedim,nbcomp,is_structured,NIJK);

      dom.elem_type_=latadb_name_from_type_geo(type_geo);

      if (!is_structured)
        {
          LataDBField som;
          som.name_ = "SOMMETS";
          som.geometry_ = dom.name_;
          som.filename_ = filename;
          som.size_=nnodes;
          som.datatype_ = default_type_float(); // ??
          som.nb_comp_=spacedim;

          LataDBField elem;
          elem.name_ = "ELEMENTS";
          elem.geometry_ = dom.name_;
          elem.filename_ = filename;
          elem.size_=ncells;
          elem.datatype_ = default_type_float(); // ??

          int dim,ff,ef;
          get_element_data(dom.elem_type_, dim, elem.nb_comp_, ff, ef);
          if (elem.nb_comp_ == -1) elem.nb_comp_ = nbcomp;

          add(timesteps_.size() - 1, dom);
          add(timesteps_.size() - 1, som);
          add(timesteps_.size() - 1, elem);
        }
      else
        {
          add(timesteps_.size() - 1, dom);
          {
            LataDBField som;
            som.name_ = "SOMMETS_IJK_I";
            som.geometry_ = dom.name_;
            som.filename_ = filename;
            som.size_=NIJK[0];
            som.datatype_ = default_type_float(); // ??
            som.nb_comp_=1;
            add(timesteps_.size() - 1, som);
          }
          {
            LataDBField som;
            som.name_ = "SOMMETS_IJK_J";
            som.geometry_ = dom.name_;
            som.filename_ = filename;
            som.size_=NIJK[1];
            som.datatype_ = default_type_float(); // ??
            som.nb_comp_=1;
            add(timesteps_.size() - 1, som);
          }
          {
            LataDBField som;
            som.name_ = "SOMMETS_IJK_K";
            som.geometry_ = dom.name_;
            som.filename_ = filename;
            som.size_=NIJK[2];
            som.datatype_ = default_type_float(); // ??
            som.nb_comp_=1;
            add(timesteps_.size() - 1, som);
          }

        }

      vector<string> fields;
      fields= MEDCoupling::GetAllFieldNamesOnMesh(filename,dom.name_.getString());

      for (int i=0; i<fields.size(); i++)
        {
          LataDBField som;
          som.name_ = fields[i];
          som.filename_ = filename;
          som.datatype_ = default_type_float(); // ??
          som.nature_ = LataDBField::SCALAR;

          const Nom& meshname = dom.name_;
          som.geometry_ = meshname;
          Nom newname(fields[i].c_str());
          Nom ajout("_");

          // cerr<<"field " <<fields[i]<< " "<< meshname<<" ";
          vector< MEDCoupling::TypeOfField > ltypes=MEDCoupling::GetTypesOfField(filename,meshname.getString(),fields[i].c_str());
          //if (ltypes.size()!=1) throw;
          for (int t=0; t < (int)ltypes.size(); t++)
            {
              switch (ltypes[t])
                {
                case MEDCoupling::ON_CELLS :
                  //cerr<<"elem"<<endl;
                  som.size_=ncells;
                  som.localisation_="ELEM";
                  break;
                case  MEDCoupling::ON_NODES :
                  //cerr<<"som"<<endl;
                  som.size_=nnodes;
                  som.localisation_="SOM";
                  break;
                default:
                  cerr<<"type inconnu "<<endl;
                  throw;
                }

              som.nb_comp_=1;
              // pour recupere nb_comp !!!

              som.nb_comp_ = (int)MEDCoupling::GetComponentsNamesOfField(filename,fields[i].c_str()).size();

              if (spacedim==som.nb_comp_)
                som.nature_ = LataDBField::VECTOR;
              ajout+=som.localisation_;
              ajout+="_";
              ajout+=meshname;
              newname=newname.prefix(ajout);

              som.name_=newname;

              //som.uname_= Field_UName(meshname, newname, som.localisation_);

              table.fields_.add(som);

              if (ltypes.size()>1)
                {
                  auto iters= MEDCoupling::GetFieldIterations(ltypes[t],filename,meshname.getString(),fields[i].c_str());
                  for (int iter=0; iter< (int)iters.size(); iter++)
                    {
                      double t= MEDCoupling::GetTimeAttachedOnFieldIteration(filename,fields[i].c_str(),iters[iter].first,iters[iter].second);
                      if (first)
                        times.push_back(t);
                      else
                        {
                          if (times[iter]!=t)
                            cerr<<"field " <<fields[i]<<" M time "<< t << " diff "<<times[iter] << endl;
                        }
                    }

                }
              else
                {
                  auto vtimes=MEDCoupling::GetAllFieldIterations(filename,/*meshname,*/fields[i].c_str());
                  for (int it=0; it<vtimes.size(); it++)
                    {
                      double t=vtimes[it].second;
                      if (first)
                        times.push_back(t);
                      else
                        {
                          if (times[it]!=t)
                            cerr<<"field " <<fields[i]<<" time "<< t << " diff "<<times[it] << endl;
                        }
                    }
                }
              first=false;
            }
        }
    }

  if (times.size()>0)
    for (int i=0; i<(int)times.size(); i++)
      {
        //LataDBTimestep & t = timesteps_.add(table);
        LataDBTimestep& t = timesteps_.add(LataDBTimestep());
        t.time_=times[i];
        for (int f=0; f<(int)table.fields_.size(); f++)
          add(i+1,table.fields_[f]);
      }

  for (int i=0; i<(int)times.size()*0; i++)
    cerr<<" time "<<times[i]<<endl;
}


template <class C_Tab>
void LataDB::read_data2_med_(const LataDBField& fld, C_Tab * const data, // const pointer to non const data !
                             Size_t debut, Size_t n) const
{
  using namespace MEDCoupling;
  using value_t = typename C_Tab::Value_type_;    // type stored in TAB
  assert(debut==0);

  if (fld.name_=="SOMMETS")
    {
      // Type checking:
      assert(!(std::is_integral<value_t>::value));

      //    cerr<<"load sommets "<<endl;
      MCAuto<MEDCouplingUMesh> mesh = ReadUMeshFromFile(fld.filename_.getString(),fld.geometry_.getString());
      const DataArrayDouble* coords=mesh->getCoords();
      data->resize(fld.size_,fld.nb_comp_);
      for (trustIdType i=0; i<fld.size_; i++)
        for (int j=0; j<fld.nb_comp_; j++)
          (*data)(i,j)=(value_t)coords->getIJ(i,j); // see type checking above

    }
  else if (fld.name_=="ELEMENTS")
    {
      // Type checking:
      assert((std::is_same<value_t, mcIdType>::value));

      // cerr<<"load elements "<<endl;
      Nom type_elem=get_geometry(fld.timestep_,fld.geometry_).elem_type_;
      LataDB::Element type =LataDB::element_type_from_string(type_elem);
      ArrOfInt filter=renum_conn(type);
      MCAuto<MEDCouplingUMesh> mesh = ReadUMeshFromFile(fld.filename_.getString(),fld.geometry_.getString());
      const DataArrayIdType *elems = mesh->getNodalConnectivity(), *idx = mesh->getNodalConnectivityIndex();
      const mcIdType *ptr_elems=elems->getConstPointer(), *ptr_idx = idx->getConstPointer();
      data->resize(fld.size_,fld.nb_comp_);
      trustIdType compt=0;
      bool void_fild = (filter.size_array()<=0);
      for (trustIdType i=0; i<fld.size_; i++)
        {
          compt++;
          for (int j=0; j<fld.nb_comp_; j++)
            {
              bool reel = (trustIdType)j + ptr_idx[i] + 1 < ptr_idx[i + 1];
              (*data)(i, void_fild ? j : filter[j]) = reel ? (value_t)ptr_elems[compt] + 1 : 0; // see type checking above
              if (reel) compt++;
            }
        }
    }
  else if (fld.name_.debute_par("SOMMETS_IJK_"))
    {
      // Type checking:
      assert(!(std::is_integral<value_t>::value));

      MEDCouplingMesh * mesh = ReadMeshFromFile(fld.filename_.getString(),fld.geometry_.getString());
      data->resize(fld.size_,fld.nb_comp_);
      MCAuto<MEDCouplingCMesh> cmesh = dynamic_cast<MEDCouplingCMesh*>(mesh);
      if (cmesh == nullptr)
        {
          cerr << "Error: can not read CMesh ... " << endl;
          throw;
        }
      int dir;
      if (fld.name_=="SOMMETS_IJK_I")
        dir=0;
      else  if (fld.name_=="SOMMETS_IJK_J")
        dir=1;
      else if (fld.name_=="SOMMETS_IJK_K")
        dir=2;
      else
        abort();
      const DataArrayDouble* coords=cmesh->getCoordsAt(dir);
      for (trustIdType i=0; i<fld.size_; i++)
        for (int j=0; j<fld.nb_comp_; j++)
          (*data)(i,j)=(value_t)coords->getIJ(i,j); // see type checking above
    }

  else
    {
      // Type checking:
      assert(!(std::is_integral<value_t>::value));

      data->resize(fld.size_,fld.nb_comp_);

      //  if (fld.timestep_==1) iter.first=1;
      // double t;
      Nom fieldname=fld.name_;
      fieldname+="_";
      fieldname+=fld.localisation_;
      fieldname+="_";
      fieldname+=fld.geometry_;

      bool ok=false;

      vector<string> fields = GetAllFieldNamesOnMesh(fld.filename_.getString(),fld.geometry_.getString());

      for (int f=0; f<(int)fields.size(); f++)
        if (fieldname==fields[f].c_str())
          {
            ok=true;
            break;
          }
      if (!ok)
        fieldname=fld.name_;
      auto vtimes = GetAllFieldIterations(fld.filename_.getString(),fieldname.getString());

      int it=fld.timestep_-1;
      pair<int, int> iter(fld.timestep_-1,-1);
      if (fld.timestep_==1) it=0;
      //Cerr<<iter.first <<" 00 "<<vtimes.size()<<finl;

      iter.first=vtimes[it].first.first;
      iter.second=vtimes[it].first.second;
      // Cerr<<"iiii"<<iter.first<<" "<< it<<finl;
      double t = GetTimeAttachedOnFieldIteration(fld.filename_.getString(),fieldname.getString(),iter.first,iter.second);

      MEDCoupling::TypeOfField type;
      if (fld.localisation_=="ELEM")
        type=MEDCoupling::ON_CELLS;
      else
        {
          type=MEDCoupling::ON_NODES;
          assert(fld.localisation_=="SOM");
        }
      MCAuto<MEDFileField1TS> field = MEDFileField1TS::New(fld.filename_.getString(),fieldname.getString(),iter.first,iter.second);
      const DataArrayDouble *values=field->getUndergroundDataArray();

      if (field->getNumberOfComponents()!=fld.nb_comp_)
        {
          cerr<<field->getNumberOfComponents()<<" test "<< endl;
          Journal()<<fieldname<<" not loaded "<<endl;
        }
      else
        {
          assert(field->getNumberOfComponents()==fld.nb_comp_);
          const double* ptr=values->getConstPointer();
          for (trustIdType i=0; i<fld.size_; i++)
            for (int j=0; j<fld.nb_comp_; j++)
              (*data)(i,j)=(value_t)ptr[i*fld.nb_comp_+j]; // see type checking above
        }
    }
}
#endif
