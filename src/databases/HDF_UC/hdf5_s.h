#ifndef _HDS_H
#define _HDS_H

#define H5_USE_16_API
#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

class H5S{
public:
        //constructor and destructor
        H5S();
        ~H5S();

        //base functions

        //create a data set
        bool create(int rank, const hsize_t *dims);
/*inline void create(int rank, uint64_t *dims) {
           hsize_t tdims[6];
           create(rank,(const hsize_t*)tdims);
           for(int i=0;i<rank;i++) dims[i]=(uint64_t)tdims[i];
        }*/
        //close the dataset
        bool close();
        //get the id of the H5G group
        hid_t getID(){return classID;}
        //get the ndim
        int64_t getNDim(hid_t dataset_id){return H5Sget_simple_extent_ndims(dataset_id);}
        int64_t getDim(hid_t dataset_id,
                        hsize_t stddims[],
                        hsize_t maxdims[]){return H5Sget_simple_extent_dims(dataset_id,stddims,maxdims);}
        bool assignSpace(hid_t dataset_id);
        bool selectHSlab(hsize_t offset[], hsize_t count[]);
        bool selectElements(hsize_t max,hsize_t num_dimm,std::vector<int32_t> *indices);
        bool selectNone();
        

private:
        //is the classID valid...i.e. is it tied to a file?
        bool valid;
        hid_t classID;        
        herr_t status;
};        
#endif
