#include "indexfileC.h"
#include "indexfile.h"
#include <string.h>
#include <stdlib.h>
// #include <stdio.h>

#define H5UCptr(x) ((H5UCFile)((void*)(x)))

#define H5_Indexptr(x) ((H5_Index*)((void*)(x)))

H5UCFile H5UC_Open(char *filename){
  H5_Index *file = new H5_Index();
  file->openHDF5File(filename);
  return H5UCptr(file);
}

void H5UC_Close(H5UCFile f){
  H5_Index *file = H5_Indexptr(f);
  if(file) delete file;
}

int H5UC_getNumVariables(H5UCFile f){
  H5_Index *file=H5_Indexptr(f);
  if(!file) return false;
  std::vector< std::string > names;
  file->getVariableNames(names);
  return names.size();
}

void H5UC_getVariableNames(H5UCFile f,char **namelist,int maxlen){
  H5_Index *file=H5_Indexptr(f);
  if(!file) return;
  // if(maxlen<=0) maxlen=32; // nuts!
  std::vector< std::string > names;
  file->getVariableNames(names);
  int i;
  for(i=0;i<(int)names.size();i++){
    if(!namelist[i]){
      if((int)names[i].size()>maxlen)
        maxlen=names[i].size()+1;
      namelist[i]=(char*)malloc(names[i].size()+1);
    }
    register char *name=namelist[i];
    strncpy(name,names[i].c_str(),maxlen);
  }
}

char **H5UC_getVariableNamesAlloc(H5UCFile f,int *num){ /* allocate names (user must deallocate) */
  char **namelist;
  H5_Index *file=H5_Indexptr(f);
  if(!file) return 0;
  std::vector< std::string > names;
  file->getVariableNames(names);
  namelist = (char**)malloc(sizeof(char*)*names.size());
  int i;
  for(i=0;i<(int)names.size();i++){
    char *name = namelist[i]=(char*)malloc(names[i].size()+1);
    strcpy(name,names[i].c_str());
  }
  if(num) *num=names.size();
  return namelist;
}

int64_t H5UC_getNumSteps(H5UCFile f){
  H5_Index *file=H5_Indexptr(f);
  if(!file) return 0;
  return file->getNumSteps();
}

double H5UC_getMinStep(H5UCFile f){
  H5_Index *file=H5_Indexptr(f);
  if(!file) return 0.0;
  return file->getMinStep();
}

double H5UC_getMaxStep(H5UCFile f){
  H5_Index *file=H5_Indexptr(f);
  if(!file) return 0.0;
  return file->getMaxStep();
}

static BaseFileInterface::DataType TypeConvert(int datatype){
  switch(datatype){
  case H5_FLOAT32:
    return BaseFileInterface::H5_Float;
    break;
  case H5_FLOAT64:
    return BaseFileInterface::H5_Double;
    break;
  case H5_INT32:
    return BaseFileInterface::H5_Int32;
    break;
  case H5_INT64:
    return BaseFileInterface::H5_Int64;
    break;
  case H5_BYTE:
    return BaseFileInterface::H5_Byte;
    break;
  default:
    return BaseFileInterface::H5_Error;
  }
  return BaseFileInterface::H5_Error;
}

int64_t H5UC_getDatasetSize(H5UCFile f,int rank,int *dims,int datatype){ // generic size calculator
  H5_Index *file=H5_Indexptr(f);
  if(!file) return 0;   
  std::vector<int64_t> dimvec;
  dimvec.resize(rank);
  BaseFileInterface::DataType t=TypeConvert(datatype);
  for(int i=0;i<rank;i++) dimvec[i]=dims[i];
  // very lame switch statement!!! (argh)
  return file->getDatasetSize(dimvec,t);
}

int64_t H5UC_getVariableInfo(H5UCFile f, /* <in> */
                        char *name, /* <in> */
                        int64_t timestep, /* <in> */
                        int *rank, /* <out> */
                        int64_t *dims, /* <out> */
                        int *datatype){ /* <out> (ignore if null) */
  H5_Index *file=H5_Indexptr(f);
  if(!file) return 0;
  std::vector<int64_t> dimvec;
  std::string str(name);
  BaseFileInterface::DataType t;
  int64_t rval = file->getVariableInfo((const std::string)str,timestep,dimvec,&t);
  if(rank) *rank=dimvec.size();
  if(dims) for(int i=0;i<(int)dimvec.size();i++) dims[i]=dimvec[i];
  if(datatype) *datatype=(int)t;
  return rval;
}


void H5UC_getData(H5UCFile f,char *varname,int64_t time,void *data){
  H5_Index *file=H5_Indexptr(f);
  if(!file) return;
  std::string str(varname);
  file->getData(str,time,data);
}
