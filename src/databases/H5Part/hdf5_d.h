#ifndef _HDD_H
#define _HDD_H

#include "hdf5.h"
#include "base_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

class H5D{
public:
        //constructor and destructor
        H5D();
        ~H5D();

        //base functions

        //open a dataset
        bool open(hid_t,const char*);
        //close a dataset
        bool close();
        //create a data set
        bool create(hid_t filename, const char *groupname,hid_t memtype,hid_t attribute_space);
        bool create_type(hid_t filename, const char *groupname,BaseFileInterface::DataType type,hid_t attribute_space);
        //read a data set
        bool read(hid_t memtype, void *buffer);
        bool read_type(BaseFileInterface::DataType type, void *buffer);
        bool slabRead(hid_t memtype, hid_t memspace, hid_t dataspace,void * buffer);
        //write the dataset
        bool write(hid_t memtype,void* buffer);
        bool write_type(BaseFileInterface::DataType type,void* buffer);
        bool slabWrite(hid_t memtype, hid_t memspace, hid_t dataspace,void * buffer);
        //get the id of the H5G group
        hid_t getID(){return classID;}
        //return the number of subgroups in this group
        hsize_t getGroupCount();
        //get the DataType for the given dataset
        BaseFileInterface::DataType getDataType();
        //identifier for a copy of the dataspace for a dataset
        hid_t getDataSpace(){return H5Dget_space(classID);}
        hsize_t getSize(){return H5Dget_storage_size(classID);}
        void getSize(uint64_t *length);
        

private:
        //is the classID valid...i.e. is it tied to a file?
        bool valid;
        hid_t classID;  
        herr_t status;
};      
#endif
