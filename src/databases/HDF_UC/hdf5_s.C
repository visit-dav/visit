#include "hdf5_s.h"
#include <visit-hdf5.h>

H5S::H5S(){
        valid = false;
        status = 1;
}

H5S::~H5S(){
        close();
}

bool H5S::close(){
        bool test = false;
        if(valid){
                status = H5Sclose(classID);
                if(status>=0){
                        valid = false;
                        test = true;
                }
        }
        return test;
}

bool H5S::create(int rank, const hsize_t *dims ){
        //if currently valid, close and open a new
        if(valid){
                status = H5Sclose(classID);
                if(status>=0)valid = false;
        }
        classID = H5Screate_simple(rank,dims,NULL);
//        printf("%d\n",classID);
        if((status>=0)&&(classID>=0)){
                valid = true;
                return true;
        }
        else return false;
}

bool H5S::assignSpace(hid_t dataset_id){

        if(valid){
                status = H5Sclose(classID);
                if(status>=0)valid = false;
        }
        classID = dataset_id;
//        printf("%d\n",classID);
        if((status>=0)&&(classID>=0)){
                valid=true;
                return true;
        }
        else return false;
}

bool H5S::selectNone(){
        status = H5Sselect_none(classID);
        if(status>=0)return true;
        else return false;}

/// Defines a selection with nsel points.
/// The incoming array indices specifies the coordinates of the elements to
/// be selected.  This function does the necessary transformation to match
/// the requirement of HDF5 H5Sselect_elements.  For version 1.6 and
/// earlier, it simply need to copy the indices into an array of hsize_t
/// type.  For version 1.8 and later, it has to generate a second level
/// pointers to satisfy the new requirement of HDF5 API.
///
/// TODO: change indices to std::vector<hsize_t> to avoid the need of
/// copying to coord.  After changing indices to std::vector<int32_t>,
/// coord == indices->begin().
bool H5S::selectElements(hsize_t nsel, hsize_t num_dimms,
                         std::vector<int32_t> *indices) {
  // copy the indices into array coord
  hsize_t *coord;
  coord = (hsize_t*) calloc ((nsel*num_dimms),sizeof(hsize_t));
  for (size_t j = 0; j < nsel * num_dimms; ++ j)
    coord[j] = indices->operator[](j);
  
#ifdef H5_USE_16_API
  status = H5Sselect_elements(classID, H5S_SELECT_SET, nsel,
                              const_cast<const hsize_t*>(coord));
#else
  // for HDF5 version 1.8 and later, need a new level of indirection
  const hsize_t **cptr =
    (const hsize_t**) calloc(nsel * sizeof(const hsize_t*));
  for (size_t j2 = 0; j2 < nsel; ++ j2)
    cptr[j2] = coord + j2 * num_dimms;
  status = H5Sselect_elements(classID, H5S_SELECT_SET, nsel, cptr);
  free(cptr);
#endif
  
  free (coord);
  if(status>=0)return true;
  else { 
    H5Eprint2(H5E_DEFAULT, stderr);
    return false;
  }

}

bool H5S::selectHSlab(hsize_t offset[], hsize_t count[]){
        
        status = H5Sselect_hyperslab(classID,H5S_SELECT_SET,offset,NULL,count,NULL);
        if(status>=0)return true;
        else return false;
}

