#include "hdf5_a.h"

H5A::H5A(){
        valid = false;
        status = 1;
}

H5A::~H5A(){
        close();
}

bool H5A::close(){
        bool test = false;
        if(valid){
                status = H5Aclose(classID);
                if(status>=0){
                        valid = false;
                        test = true;
                }
        }
        return test;
}

bool H5A::open(hid_t filename, const char *groupname){
        //if the file is valid, close the old and open a new
        if(valid){
                status = H5Aclose(classID);
                if(status>=0)valid = false;
        }
        classID = H5Aopen_name(filename,groupname);
//      printf("%d\n",classID);
        if((classID >= 0) && (status >= 0)){
                valid = true;
                return true;
        }
        else return false;
}

bool H5A::Delete(hid_t filename, const char *groupname){
        if(valid){
                status = H5Adelete(filename,groupname);
                if(status>=0)valid = false;
        }
        if(status >= 0)return true;
        else return false;
}

bool H5A::create(hid_t filename, const char *groupname,hid_t memtype,hid_t attribute_space){
        //if currently valid, close and open a new
        if(valid){
                status = H5Aclose(classID);
                if(status>=0)valid = false;
        }
        classID = H5Acreate(filename, groupname, memtype, attribute_space,
                            H5P_DEFAULT
#ifndef H5_USE_16_API
                            , H5P_DEFAULT
#endif
                            );
//      printf("%d\n",classID);
        if((classID >= 0) && (status >= 0)){
                valid = true;
                return true;
        }
        else return false;
}

bool H5A::create_type(hid_t filename, const char *groupname,
                      BaseFileInterface::DataType type, hid_t attribute_space) {

        //if currently valid, close and open a new
        if(valid){
                status = H5Aclose(classID);
                if(status>=0)valid = false;
        }
        switch(type){
                case BaseFileInterface::DataType(0):
                        classID = H5Acreate(filename, groupname,
                                            H5T_NATIVE_FLOAT, attribute_space,
                                            H5P_DEFAULT
#ifndef H5_USE_16_API
                                            , H5P_DEFAULT
#endif
                                            );
                break;
                case BaseFileInterface::DataType(1):
                        classID = H5Acreate(filename, groupname,
                                            H5T_NATIVE_DOUBLE, attribute_space,
                                            H5P_DEFAULT
#ifndef H5_USE_16_API
                                            , H5P_DEFAULT
#endif
                                            );
                break;
                case BaseFileInterface::DataType(2):
                        classID = H5Acreate(filename, groupname,
                                            H5T_NATIVE_INT32, attribute_space,
                                            H5P_DEFAULT
#ifndef H5_USE_16_API
                                            , H5P_DEFAULT
#endif
                                            );
                break;
                case BaseFileInterface::DataType(3):
                        classID = H5Acreate(filename, groupname,
                                            H5T_NATIVE_INT64, attribute_space,
                                            H5P_DEFAULT
#ifndef H5_USE_16_API
                                            , H5P_DEFAULT
#endif
                                            );
                break;
                case BaseFileInterface::DataType(4):
                        classID = H5Acreate(filename, groupname,
                                            H5T_NATIVE_CHAR, attribute_space,
                                            H5P_DEFAULT
#ifndef H5_USE_16_API
                                            , H5P_DEFAULT
#endif
                                            );
                break;
                default:
                break;
        }
        if((classID >= 0) && (status >= 0)){
                valid = true;
                return true;
        }
        else return false;
}

bool H5A::write(hid_t memtype,void* buffer){
        status = H5Awrite(classID,memtype,buffer);
        if(status >= 0) return true;
        else return false;
}

bool H5A::write_type(BaseFileInterface::DataType type,void* buffer){
        switch(type){
                case BaseFileInterface::DataType(0):
                        float *data_f;
                        data_f = (float *)buffer;
                        status = H5Awrite(classID,H5T_NATIVE_FLOAT,data_f);
                break;
                case BaseFileInterface::DataType(1):
                        double *data_d;
                        data_d = (double *)buffer;
                        status = H5Awrite(classID,H5T_NATIVE_DOUBLE,data_d);
                break;
                case BaseFileInterface::DataType(2):
                        int32_t *data_i;
                        data_i = (int32_t *)buffer;
                        status = H5Awrite(classID,H5T_NATIVE_INT32,data_i);
                break;
                case BaseFileInterface::DataType(3):
                        int64_t *data_l;
                        data_l = (int64_t *)buffer;
                        status = H5Awrite(classID,H5T_NATIVE_INT64,data_l);
                break;
                case BaseFileInterface::DataType(4):
                        char *data_b;
                        data_b = (char *)buffer;
                        status = H5Awrite(classID,H5T_NATIVE_CHAR,data_b);
                break;
                default:
                break;
        }
        if(status >= 0) return true;
        else return false;
}


bool H5A::read(hid_t memtype,void* buffer ){
        status = H5Aread(classID,memtype,buffer);
        if(status >= 0) return true;
        else return false;
}

bool H5A::read_type(BaseFileInterface::DataType type,void* buffer){
        switch(type){
                case BaseFileInterface::DataType(0):
                        status = H5Aread(classID,H5T_NATIVE_FLOAT,buffer);
                break;
                case BaseFileInterface::DataType(1):
                        status = H5Aread(classID,H5T_NATIVE_DOUBLE,buffer);
                break;
                case BaseFileInterface::DataType(2):
                        status = H5Aread(classID,H5T_NATIVE_INT32,buffer);
                break;
                case BaseFileInterface::DataType(3):
                        status = H5Aread(classID,H5T_NATIVE_INT64,buffer);
                break;
                case BaseFileInterface::DataType(4):
                        status = H5Aread(classID,H5T_NATIVE_CHAR,buffer);
                break;
                default:
                break;
        }
        if(status >= 0) return true;
        else return false;
}
