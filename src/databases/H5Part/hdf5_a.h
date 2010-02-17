#ifndef _HDA_H
#define _HDA_H

#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "base_api.h"

class H5A{
public:
        //constructor and destructor
        H5A();
        ~H5A();

        //base functions

        //open a dataset
        bool open(hid_t,const char*);
        //close a dataset
        bool close();
        bool Delete(hid_t filename, const char *groupname);
        //create a data set
        bool create(hid_t filename, const char *groupname,hid_t memtype,hid_t attribute_space);
        bool create_type(hid_t filename, const char *groupname,BaseFileInterface::DataType type,hid_t attribute_space);
        //read a data set
        bool read(hid_t memtype, void *buffer);
        bool read_type(BaseFileInterface::DataType type,void* buffer);
        //write the dataset
        bool write(hid_t memtype,void* buffer);
        bool write_type(BaseFileInterface::DataType type,void* buffer);
        //get the id of the H5G group
        hid_t getID() const {return classID;}
        //return the number of subgroups in this group
        hsize_t getGroupCount();
        

private:
        //is the classID valid...i.e. is it tied to a file?
        bool valid;
        hid_t classID;  
        herr_t status;
};      
#endif
