#include "hdf5_g.h"

H5G::H5G(){
        valid = false;
        status = 1;
}

H5G::~H5G(){
        close();
}

bool H5G::close(){
        bool test = false;
        if(valid){
                status = H5Gclose(classID);
                if(status>=0){
                        valid = false;
                        test = true;
                }
        }
        return test;
}

bool H5G::open(hid_t filename, const char *groupname){
        //if the file is valid, close the old and open a new
        if(valid){
                status = H5Gclose(classID);
                if(status>=0)valid = false;
        }
        classID = H5Gopen(filename,groupname);
//        printf("%d\n",classID);
        if((status>=0)&&(classID>=0)){
                valid = true;
                return true;
        }
        else return false;
}

bool H5G::create(hid_t filename, const char *groupname){
        //if currently valid, close and open a new
        if(valid){
                status = H5Gclose(classID);
                if(status>=0)valid = false;
        }
        classID = H5Gcreate(filename,groupname,0);
//        printf("%d\n",classID);

        if((status>=0)&&(classID>=0)){
                valid = true;
                return true;
        }
        else return false;
}

hsize_t H5G::getGroupCount(){
        hsize_t i;
        H5Gget_num_objs(classID,&i);
        return i;
}
