#include "hdf5_s.h"

H5S::H5S() {
    valid = false;
    status = 1;
}

H5S::~H5S(){
    close();
}

bool H5S::close() {
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

bool H5S::create(int rank, const hsize_t *dims ) {
    //if currently valid, close and open a new
    if(valid){
        status = H5Sclose(classID);
        if(status>=0)valid = false;
    }
    classID = H5Screate_simple(rank,dims,NULL);
    //  printf("%d\n",classID);
    if((status>=0)&&(classID>=0)){
        valid = true;
        return true;
    }
    else {
        return false;
    }
}

bool H5S::assignSpace(hid_t dataset_id) {
    if (valid) {
        status = H5Sclose(classID);
        if(status>=0)valid = false;
    }
    classID = dataset_id;
    //  printf("%d\n",classID);
    if ((status>=0)&&(classID>=0)){
        valid=true;
        return true;
    }
    else {
        return false;
    }
}

bool H5S::selectNone(){
    status = H5Sselect_none(classID);
    if(status>=0)return true;
    else return false;
}

/// Defines a selection with nsel points.
/// @note This implementation relies on the fact that std::vector stores
/// its members consecutively in memory!
bool H5S::selectElements(hsize_t nsel, hsize_t num_dimms,
        const std::vector<hsize_t>& indices) {
    if (nsel*num_dimms <= indices.size()) {
#ifndef H5_USE_16_API
        herr_t ierr =
            H5Sselect_elements(classID, H5S_SELECT_SET, nsel, &(indices[0]));
#else
        // need a new array to use use the older interface
        std::vector<const hsize_t*> pptr(nsel);
        for (hsize_t j = 0; j < nsel; ++ j)
            pptr[j] = &(indices[j*num_dimms]);
        herr_t ierr =
            H5Sselect_elements(classID, H5S_SELECT_SET, nsel, &(pptr[0]));
#endif
        return(0 <= ierr);
    }
    else {
        return false;
    }
}

bool H5S::selectHSlab(hsize_t offset[], hsize_t count[]){
    status = H5Sselect_hyperslab(classID, H5S_SELECT_SET, offset, NULL,
                                 count, NULL);
    if (status>=0) return true;
    else return false;
}
