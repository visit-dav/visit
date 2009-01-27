#include "hdf5_s.h"

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

bool H5S::selectElements(hsize_t max,hsize_t num_dimms,std::vector<int32_t> *indices){
        int32_t counter=0,c1;
        //transfer all of the users data into the coord object
        hsize_t *coord;
        coord = (hsize_t*) calloc ((max*num_dimms),sizeof(hsize_t));
        counter = indices->size();
        counter=0;
        for(int y=0;y<(int)max;y++){
                for(int y1=0;y1<(int)num_dimms;y1++){
                        c1 = indices->operator[] (counter);
                        coord[y*num_dimms + y1]=c1;
                        counter++;
                }                
        }
        status = H5Sselect_elements(classID,H5S_SELECT_SET,max,(const hsize_t **)coord);

        free (coord);
        if(status>=0)return true;
        else return false;
}

bool H5S::selectHSlab(hsize_t offset[], hsize_t count[]){
        
        status = H5Sselect_hyperslab(classID,H5S_SELECT_SET,offset,NULL,count,NULL);
        if(status>=0)return true;
        else return false;
}

