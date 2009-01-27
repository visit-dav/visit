#include "hdf5_d.h"

H5D::H5D(){
        valid = false;
        status = 1;
}

H5D::~H5D(){
        close();
}

bool H5D::close(){
        bool test = false;
        if(valid){
                status = H5Dclose(classID);
                if(status>=0){
                        valid = false;
                        test = true;
                }
        }
        return test;
}

bool H5D::open(hid_t filename, const char *groupname){
        //if the file is valid, close the old and open a new
        if(valid){
                status = H5Dclose(classID);
                if(status>=0)valid = false;
        }
        H5E_BEGIN_TRY{
                classID = H5Dopen(filename,groupname);
        }H5E_END_TRY;
        if(classID<0){
//                printf("H5D::open: File not found\n");
                return false;
        }
//        printf("%d\n",classID);
        else if((status>=0)&&(classID>=0)){
                valid = true;        
                return true;
        }
        else return false;
}

bool H5D::create(hid_t filename, const char *groupname,hid_t memtype,hid_t attribute_space){
        //if currently valid, close and open a new
        if(valid){
                status = H5Dclose(classID);
                if(status>=0)valid = false;
        }
        classID = H5Dcreate(filename,groupname,memtype,attribute_space,H5P_DEFAULT);
//        printf("%d\n",classID);
        if((status>=0)&&(classID>=0)){
                valid = true;
                return true;
        }
        else return false;
}

bool H5D::create_type(hid_t filename, const char *groupname,BaseFileInterface::DataType type,hid_t attribute_space){
        //if currently valid, close and open a new
        if(valid){
                status = H5Dclose(classID);
                if(status>=0)valid = false;
        }
        switch(type){
                case BaseFileInterface::DataType(0):
                        classID = H5Dcreate(filename,groupname,H5T_NATIVE_FLOAT,attribute_space,H5P_DEFAULT);
                break;
                case BaseFileInterface::DataType(1):
                        classID = H5Dcreate(filename,groupname,H5T_NATIVE_DOUBLE,attribute_space,H5P_DEFAULT);
                break;
                case BaseFileInterface::DataType(2):
                        classID = H5Dcreate(filename,groupname,H5T_NATIVE_INT32,attribute_space,H5P_DEFAULT);
                break;
                case BaseFileInterface::DataType(3):
                        classID = H5Dcreate(filename,groupname,H5T_NATIVE_INT64,attribute_space,H5P_DEFAULT);
                break;
                case BaseFileInterface::DataType(4):
                        classID = H5Dcreate(filename,groupname,H5T_NATIVE_CHAR,attribute_space,H5P_DEFAULT);
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

bool H5D::write_type(BaseFileInterface::DataType type,void* buffer){
        switch(type){
                case BaseFileInterface::DataType(0):
                        status = H5Dwrite(classID,H5T_NATIVE_FLOAT,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                case BaseFileInterface::DataType(1):
                        status = H5Dwrite(classID,H5T_NATIVE_DOUBLE,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                case BaseFileInterface::DataType(2):
                        status = H5Dwrite(classID,H5T_NATIVE_INT32,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                case BaseFileInterface::DataType(3):
                        status = H5Dwrite(classID,H5T_NATIVE_INT64,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                case BaseFileInterface::DataType(4):
                        status = H5Dwrite(classID,H5T_NATIVE_CHAR,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                default:
                break;
        }
        if(status >= 0){
                return true;
        }
        else return false;
}

bool H5D::write(hid_t memtype,void* buffer){
        status = H5Dwrite(classID,memtype,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
        if(status>=0)return true;
        else return false;
}

bool H5D::slabWrite(hid_t memtype, hid_t memspace, hid_t dataspace,void * buffer){
        status = H5Dwrite(classID,memtype,memspace, dataspace, H5P_DEFAULT,buffer);
        if(status>=0)return true;
        else return false;
}

bool H5D::read(hid_t memtype,void* buffer ){
        status = H5Dread(classID,memtype,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
        if(status>=0)return true;
        else return false;
}

bool H5D::read_type(BaseFileInterface::DataType type,void* buffer){
        switch(type){
                case BaseFileInterface::DataType(0):
                        status = H5Dread(classID,H5T_NATIVE_FLOAT,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                case BaseFileInterface::DataType(1):
                        status = H5Dread(classID,H5T_NATIVE_DOUBLE,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                case BaseFileInterface::DataType(2):
                        status = H5Dread(classID,H5T_NATIVE_INT32,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                case BaseFileInterface::DataType(3):
                        status = H5Dread(classID,H5T_NATIVE_INT64,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                case BaseFileInterface::DataType(4):
                        status = H5Dread(classID,H5T_NATIVE_CHAR,H5S_ALL, H5S_ALL, H5P_DEFAULT,buffer);
                break;
                default:
                break;
        }
        if(status >= 0){
                return true;
        }
        else return false;
}

void H5D::getSize(uint64_t *length){
        *length = (uint64_t)H5Dget_storage_size(classID);
}

bool H5D::slabRead(hid_t memtype, hid_t memspace, hid_t dataspace,void * buffer){
        status = H5Dread(classID, memtype, memspace, dataspace,H5P_DEFAULT, buffer);
        if(status>=0)return true;
        else return false;
}


BaseFileInterface::DataType H5D::getDataType(){
        hid_t data_type = H5Dget_type(classID);
        hid_t specific_type = H5Tget_native_type(data_type,H5T_DIR_ASCEND);
        BaseFileInterface::DataType answer;
        
        if(H5Tequal(specific_type, H5T_NATIVE_INT32)){
                answer = BaseFileInterface::DataType(2);
        }
        else if(H5Tequal(specific_type, H5T_NATIVE_FLOAT)){
                answer = BaseFileInterface::DataType(0);
        }
        else if(H5Tequal(specific_type,H5T_NATIVE_CHAR)){
                answer = BaseFileInterface::DataType(4);
        }
        else if(H5Tequal(specific_type,H5T_NATIVE_DOUBLE)){
                answer = BaseFileInterface::DataType(1);
        }
        else if(H5Tequal(specific_type,H5T_NATIVE_INT64)){
                answer = BaseFileInterface::DataType(3);
        }
        H5Tclose(specific_type);
        return answer;
} 
