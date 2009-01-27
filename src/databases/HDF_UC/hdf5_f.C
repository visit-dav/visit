#include "hdf5_f.h" 

H5F::H5F(){
        valid = false;
        status = 1;
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

bool H5F::open(const char *filename){

        //if previously opened, close and open a new one
        if(valid){
                H5Fclose(classID);
        }

        //make sure the file is a valid HDF5 file and that is exists...
        htri_t        temp;

        H5E_BEGIN_TRY{
                temp = H5Fis_hdf5(filename);
        }H5E_END_TRY;

        if(temp > 0){        
                classID = H5Fopen(filename,H5F_ACC_RDWR,H5P_DEFAULT);
                if(classID >=0)        valid = true;
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
