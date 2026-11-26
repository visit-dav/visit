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
#include "ReaderFORT21.h"

/* Description: Reads the .lata database in the given file indicating than the
 *  associated data files will be found in directory "prefix".
 *  If not empty, "prefix" must finish with a '/'.
 *  For "prefix" and "filename", if they do not begin with '/', are relative to pwd.
 * Exceptions:
 *  BAD_HEADER  means that the header found in this stream is not LATA_V2
 *  READ_ERROR  means that an error has been found in the file (premature eof,
 *              io error, bad keyword, ...)
 *  FILE_NOT_FOUND means that, well, the lata file could not be opened
 */


void clear_map_mesh(std::map<std::string,ReaderFORT21::BasicMesh*>& map_basicmeshses_)
{
  for (auto& a : map_basicmeshses_)
    {
      delete a.second;
      a.second=NULL;
    }
  map_basicmeshses_.clear();
}

class MapBasicMesh
{
  friend LataDB;
public:
  ~MapBasicMesh() { /*clear_map_mesh(map_basicmeshses_);i*/ }
private:
  mutable std::map<std::string,ReaderFORT21::BasicMesh>  map_basicmeshses_;
};

void LataDB::read_master_file_fort21(const char *prefix, const char *filename)
{

  Journal(1) << "read_master_file_fort21 " <<prefix<<" " <<filename << endl;
  ReaderFORT21 parser;
  parser.setFile(filename);
  parser.parse();
  parser.fix_bad_times();

  // Defaults for lataV1
  default_type_int_.msb_ = LataDBDataType::MSB_LITTLE_ENDIAN;
  default_type_int_.type_ = LataDBDataType::INT32;
  default_type_int_.array_index_ = LataDBDataType::C_INDEXING;
  default_type_int_.data_ordering_ = LataDBDataType::C_ORDERING;
  default_type_int_.fortran_bloc_markers_ = LataDBDataType::NO_BLOC_MARKER;
  default_type_int_.bloc_marker_type_ = LataDBDataType::INT32;
  default_float_type_ = LataDBDataType::REAL32;

  // Create timestep 0 (global domain and fields)
  timesteps_.add(LataDBTimestep());

  // on ajoute les geom
  // on verra apres pour les champs elem et som
  std::vector<std::string> geoms= parser.getElementNames();
  if (geoms.size()==0)
    throw LataDBError(LataDBError::FILE_NOT_FOUND);
  basicmeshses_ = new MapBasicMesh();
  std::map<std::string,ReaderFORT21::BasicMesh>& map_basicmeshses = basicmeshses_->map_basicmeshses_;
  //int first=1;
  std::string onedomain;
  for (unsigned int i=0; i<geoms.size(); i++)
    {
      LataDBGeometry dom;
      dom.timestep_ = timesteps_.size()-1;
      dom.name_=geoms[i];
      ReaderFORT21::BasicMesh mesh= parser.getMeshStack(geoms[i]);

      if (Nom(filename).finit_par(".21"))
        if (!Nom(filename).finit_par("FORT.21"))
          {
            if (!Nom(filename).finit_par(Nom(geoms[i])+Nom(".21")))
              {
                std::cerr<<" we dont load " << geoms[i]<< std::endl;
                continue;
              }
            else
              {
                onedomain=geoms[i];
              }
          }
      if (mesh.type_mesh_!=ReaderFORT21::MESH_Polygone)
        if (mesh.type_mesh_!=ReaderFORT21::MESH_Polyedre)
          if (mesh.type_mesh_!=ReaderFORT21::MESH_Hexa)
            continue;
      Journal(2) << "Adding geom "<<geoms[i]<<std::endl;
      map_basicmeshses[geoms[i]]= mesh;
      //new  ReaderFORT21::BasicMesh; *(map_basicmeshses[geoms[i]])= mesh;
      int ncells=mesh.nb_elems_,nnodes=mesh.nbnodes_, nbcomp=mesh.nodes_per_elem_;
      int spacedim=mesh.space_dim_;

      dom.elem_type_="Polygone";
      if (mesh.type_mesh_==ReaderFORT21::MESH_Polyedre)
        dom.elem_type_="Polyedre";
      if (mesh.type_mesh_==ReaderFORT21::MESH_Hexa)
        dom.elem_type_="Hexaedre";
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

        elem.nb_comp_ = nbcomp;

        add(timesteps_.size() - 1, dom);
        add(timesteps_.size() - 1, som);
        add(timesteps_.size() - 1, elem);
      }
    }
  std::vector<double> times =parser.getTimes();
  //auto  times =(onedomain==std::string()?parser.getTimes():parser.getTimesStack(onedomain)) ;

  for (auto t: times)
    {

      LataDBTimestep& tt =
        timesteps_.add(LataDBTimestep());
      tt.time_ = t;
    }
  for (auto a: map_basicmeshses)
    {
      const std::string& geomname = a.first;
      std::vector<std::string> fields;
      fields= parser.getVarFieldNames(geomname);
      //auto index_t =parser.getTimesIndexStack(geomname);
      auto timesf=parser.getTimesStack(geomname);
      if (times.size()!=timesf.size()) std::cout<<geomname << " soucis " << times.size()<< "  " << timesf.size()<< std::endl;
      const ReaderFORT21::BasicMesh& mesh = (a.second);

      for (unsigned int i=0; i<fields.size(); i++)
        {
          LataDBField som;

          som.name_ = fields[i];
          //if (som.name_.finit_par("%")) continue;

          som.filename_ = filename;


          som.nature_ = LataDBField::SCALAR;


          Nom meshname = geomname;
          som.geometry_ = meshname;

          ReaderFORT21::FieldInfo fieldinfo = parser.getVarFieldInfo(meshname.getString(),fields[i]);
          if (fieldinfo.type_of_field()== ReaderFORT21::T_REAL)
            {
              som.datatype_ = default_type_float(); // ??
            }
          else if (fieldinfo.type_of_field()== ReaderFORT21::T_INT)
            {
              som.datatype_ = default_type_int_; // ??
            }
          else
            continue;
          //if ((fieldinfo.size()*fieldinfo.nb_parts()==mesh.nb_elems_)||(fieldinfo.size()*fieldinfo.nb_parts()==mesh.nb_elems_+2))
          if (fieldinfo.localisation_of_field()== ReaderFORT21::L_ELEM)
            {
              som.size_=mesh.nb_elems_;
              if (fieldinfo.nb_parts()>1) som.size_=fieldinfo.size();
              som.localisation_="ELEM";
              if (fieldinfo.nb_parts()>1) som.nature_ = LataDBField::WALLSCALAR;

            }
          else
            continue;
          som.nb_comp_=1;


          int indexf=0;
          for (unsigned int  index=0; index<times.size(); index++)
            {
              file_pos_t fp;
	      if (indexf>=int(timesf.size()))
	      {
		indexf--;
                  Journal(0) << geomname<< " "<< fields[i]<<" We use time "<< timesf[indexf] <<" for " << times[index]<<std::endl;
	      }
              fp  = parser.getOffsetVarField(geomname,fields[i],indexf) ;
              som.datatype_.file_offset_ = fp;
              add(index+1,som);
	      
              if (times[index]<timesf[indexf])
                {
                  Journal(0) <<" We use time "<< timesf[indexf] <<" for " << times[index]<<std::endl;
                }
              else
                indexf++;
            }
        }
    }
}


template <class C_Tab>
int LataDB::read_data2_fort21_( LataDataFile& f,
                                const LataDBField& fld,
                                C_Tab * const data, // const pointer to non const data !
                                Size_t debut, Size_t n) const
{
  Journal(6)<< "read_data2_fort21_  " << fld.name_<<" on " <<fld.geometry_<<" "<<fld.datatype_.file_offset_<<endl;

  if (fld.name_=="SOMMETS")
    {
      assert(debut==0);
      if (!basicmeshses_)
        basicmeshses_ = new MapBasicMesh();
      std::map<std::string,ReaderFORT21::BasicMesh>& map_basicmeshses = basicmeshses_->map_basicmeshses_;
      if (map_basicmeshses.count(fld.geometry_.getString())<1)
        {
          ReaderFORT21 parser;
          parser.setFile(fld.filename_.getString());
          parser.parse();
          map_basicmeshses[fld.geometry_.getString()] = parser.getMeshStack(fld.geometry_.getString());
        }
      const ReaderFORT21::BasicMesh& mesh = (map_basicmeshses[fld.geometry_.getString()]);

      //    cerr<<"load sommets "<<endl;
      data->resize(int(fld.size_),fld.nb_comp_);
      BigFloatTab* data2= dynamic_cast<BigFloatTab*>(data);
      if (data2 == nullptr)
        {
          std::cerr << "Unexpected error!" << std::endl;
          throw;
        }
      for (int i=0; i<fld.size_; i++)
        for (int j=0; j<fld.nb_comp_; j++)
          (*data2)(i,j)=float(mesh.coords_[fld.nb_comp_*i+j]);

      return 1;
    }
  else if (fld.name_=="ELEMENTS")
    {
      assert(debut==0);
      std::map<std::string,ReaderFORT21::BasicMesh>& map_basicmeshses = basicmeshses_->map_basicmeshses_;
      const ReaderFORT21::BasicMesh& mesh = map_basicmeshses[fld.geometry_.getString()];

      data->resize(int(fld.size_),fld.nb_comp_);
      BigTIDTab* data2= dynamic_cast<BigTIDTab*>(data);
      if (data2 == nullptr)
        {
          std::cerr << "Unexpected error!" << std::endl;
          throw;
        }
      for (int i=0; i<fld.size_; i++)
        for (int j=0; j<fld.nb_comp_; j++)
          (*data2)(i,j)=mesh.connectivity_[fld.nb_comp_*i+j];
      // hexa connectivity of lata is special...
      if (mesh.type_mesh_==ReaderFORT21::MESH_Hexa)
        for (int i=0; i<fld.size_; i++)
          {
            trustIdType sa = (*data2)(i,2);
            (*data2)(i,2)= (*data2)(i,3);
            (*data2)(i,3)= sa;
            {
              trustIdType sa2 = (*data2)(i,6);
              (*data2)(i,6)= (*data2)(i,7);
              (*data2)(i,7)= sa2;
            }
          }

      return 1;
    }
  else
    {
      if (fld.nature_ == LataDBField::WALLSCALAR)
        {
          std::map<std::string,ReaderFORT21::BasicMesh>& map_basicmeshses = basicmeshses_->map_basicmeshses_;
          const ReaderFORT21::BasicMesh& mesh = (map_basicmeshses[fld.geometry_.getString()]);
          // cas particulier.... a coder
          //
          // case LataDBField::SCALAR:
          int size=int(fld.size_);
          int nb_parts= mesh.nb_elems_/size;
          //std::cout<<"Special "<< nb_parts<<" "<<fld.name_<<" "<<fld.geometry_<< " "<<fld.size_<< " "<<mesh.nb_elems_<<std::endl;
          // int aNameSize =  16;
          data->resize(size*nb_parts,fld.nb_comp_);

          LataDBField fld2(fld);
          fld2.nature_=LataDBField::SCALAR;
          C_Tab tmp(int(size),1);
          int aNameSize=16+8;
          for (int p=0; p<nb_parts; p++)
            {

              read_data2_(f,fld2,&tmp,debut,int(fld2.size_));
              fld2.datatype_.file_offset_+=aNameSize +fld2.size_*sizeof(tmp(0,0));
              for (trustIdType i = 0; i < fld2.size_; i++)
                (*data)(int(i+p*fld2.size_),0) = tmp(i,0);
            }
          return 1;
        }
      return 0;
    }
}

