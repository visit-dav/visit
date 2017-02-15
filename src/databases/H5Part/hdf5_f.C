#include "hdf5_f.h"
#include <visit-hdf5.h>

#define H5PART_READ             0x01
#define H5PART_WRITE            0x02
#define H5PART_APPEND           0x04

H5F::H5F(){
        valid = false;
        status = 1;
        H5Eset_auto(H5E_DEFAULT , 0, 0);
}

H5F::~H5F(){
        close();
}

bool H5F::close(){
        bool test = false;
        if(valid){
                status = H5Fclose(classID);
                if(status>=0){
                        valid = false;
                        test = true;
                }
        }
        return test;
}

bool H5F::open(const char *filename, const char mode){

        //if previously opened, close and open a new one
        if(valid){
                H5Fclose(classID);
        }

        //make sure the file is a valid HDF5 file and that is exists...
        htri_t  temp;

        H5E_BEGIN_TRY{
                temp = H5Fis_hdf5(filename);
        }H5E_END_TRY;

        if(temp > 0){
            if( mode == H5PART_WRITE || mode == H5PART_APPEND )
                classID = H5Fopen(filename,H5F_ACC_RDWR,H5P_DEFAULT);
            else if( mode == H5PART_READ )
                classID = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);

            if(classID >=0) valid = true;
        }

        else{
                classID = H5Fcreate(filename,H5F_ACC_TRUNC,H5P_DEFAULT,H5P_DEFAULT);
        }
        return valid;
}

bool H5F::testFileExist(const char *filename){
        bool answer;
        //make sure the file is a valid HDF5 file and that is exists...
        std::ifstream filestr;
        filestr.open(filename, std::ifstream::in);
        if(filestr.is_open() && (H5Fis_hdf5(filename) > 0)){
                answer = true;
        }
        else{
                answer = false;
        }
        filestr.close();
        return answer;
}
