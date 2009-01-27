#ifndef _HDF_H
#define _HDF_H

#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

/*
Low level template for the HDF5 interface.  H5G, H5D, and H5S classes are all derived form this
base class.
*/

class H5F{
public:
        //constructor and destructor
        H5F();
        ~H5F();
        
        //base functions

        //if the file exists, the file is opened and "true" is returned,
        //otherwise it is created and "false" is returned. 
        bool open(const char *);
        //close the file
        bool close();
        //test first of a given file exists and return true or false
        bool testFileExist(const char *);
        hid_t getID(){return classID;}        

    // Return the name of the HDF5 file.
    std::string getName() {
        char buf[1024];
        ssize_t ierr = H5Fget_name(classID, buf, 1024);
        if (ierr <= 0 || ierr >= 1024)
            buf[0] = 0;
        return buf;
    }
        
private:
        //is the classID valid...i.e. is it tied to a file?
        bool valid;
        hid_t classID;
        herr_t status;
};

#endif 
