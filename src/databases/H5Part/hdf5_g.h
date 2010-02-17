#ifndef _HDG_H
#define _HDG_H

#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

class H5G{
public:
        //constructor and destructor
        H5G();
        ~H5G();

        //base functions

        //open a dataset
        bool open(hid_t,const char*);
        //close the dataset
        bool close();
        bool create(hid_t,const char*);
        //get the id of the H5G group
        hid_t getID(){return classID;}
        //return the number of subgroups in this group
        hsize_t getGroupCount();
        

private:
        //is the classID valid...i.e. is it tied to a file?
        bool valid;
        hid_t classID;  
        herr_t status;
};      
#endif
