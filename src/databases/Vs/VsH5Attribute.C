/*
 * VsH5Attribute.cpp
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#include "VsH5Attribute.h"
#include "VsLog.h"
#include <hdf5.h>
  
VsH5Attribute::VsH5Attribute(VsH5Object* parentObject, std::string attName, hid_t id):
  VsH5Object(parentObject->registry, parentObject, attName, id) {
  
  dataType = H5Aget_type(id);
}

VsH5Attribute::~VsH5Attribute() {
  hid_t id = getId();
  if (id != -1) {
    H5Aclose(id);
    id = -1;
  }
}

void VsH5Attribute::write() {
  std::string offset = "";
  for (int i = 0; i < getDepth(); ++i) {
    offset += "  ";
  }
  VsLog::debugLog() << offset<< "Attribute "<< getShortName() << std::endl;
  VsLog::debugLog() << offset<< "  rank = " << dims.size() << std::endl;
}

// Total length
size_t VsH5Attribute::getLength() {
  size_t len = 1;
  for (size_t i = 0; i < dims.size(); ++i) {
    len *= dims[i];
  }
  return len;
}

herr_t VsH5Attribute::getStringValue(std::string* val) {
  hid_t atype = H5Aget_type(getId());
  H5T_class_t type = H5Tget_class(atype);
  hid_t aspace = H5Aget_space(getId());
  size_t rank = H5Sget_simple_extent_ndims(aspace);
  
  if (type != H5T_STRING) {
    VsLog::warningLog() <<"VsH5Attribute::getStringValue() - Requested attribute " <<getShortName()
       <<" is not a string value." <<std::endl;
    val->clear();
    return -1;
  }
  
  //Yes, this is correct (rank != 0)
  if (rank != 0) {
    VsLog::warningLog() <<"VsH5Attribute::getStringValue() - Requested attribute " <<getShortName()
           <<" is not a string value." <<std::endl;
    val->clear();
    return -1;
  }
  
  size_t len = H5Aget_storage_size(getId());
  val->resize(len);
  char* v = new char[len];
  herr_t err = H5Aread(getId(), atype, v);
  // JRC: is this right?
  // err = H5Aread(id, H5T_NATIVE_CHAR, v);
  for (size_t i = 0; i < len; ++i) {
     if (v[i] == 0) {
       //This happens when a program (IDL, HdfView) declares a string of length 5
       //and uses the 5th character for the null terminator
       //In h5dump this shows up as a string "mesh" being declared as 5 characters
       //If dumped to a terminal, it prints like so: "mesh^@"
       VsLog::warningLog() <<"VsH5Attribute::getStringValue() - Found null char inside string attribute: " <<getFullName() <<std::endl;
       val->resize(i);
       break;
     }
     (*val)[i] = v[i];
  }
  delete [] v;
  return err;
}

// JRC: This fat interface may not scale?  What about
// scalar attributes?
herr_t VsH5Attribute::getIntVectorValue(std::vector<int>* ivals) {
  herr_t err;
  size_t npoints;
  hid_t atype = H5Aget_type(getId());
  H5T_class_t type = H5Tget_class(atype);
  hid_t aspace = H5Aget_space(getId());
  size_t rank = H5Sget_simple_extent_ndims(aspace);

  if (type != H5T_INTEGER) {
    VsLog::warningLog() <<"VsH5Attribute::getIntVectorValue() - Requested attribute " <<getShortName()
        <<" is not an integer vector." <<std::endl;
     ivals->resize(0);
     return -1;
  }
  
  if (rank == 0) {
    ivals->resize(1);
    int v;
    // err = H5Aread(id, atype, &v);
    err = H5Aread(getId(), H5T_NATIVE_INT, &v);
    (*ivals)[0] = v;
    return err;
  }
  
  // rank>0
  npoints = H5Sget_simple_extent_npoints(aspace);
  int* v = new int[npoints];
  err = H5Aread(getId(), H5T_NATIVE_INT, v);
  ivals->resize(npoints);
  for (size_t i = 0; i<npoints; ++i) {
    (*ivals)[i] = v[i];
  }
  delete v;
  
  return err;
}

// JRC: This fat interface may not scale?  What about
// scalar attributes?
herr_t VsH5Attribute::getFloatVectorValue(std::vector<float>* fvals) {

  herr_t err = 0;
  size_t npoints;
  hid_t atype = H5Aget_type(getId());
  H5T_class_t type = H5Tget_class(atype);
  hid_t aspace = H5Aget_space(getId());
  size_t rank = H5Sget_simple_extent_ndims(aspace);

  if (type != H5T_FLOAT) {
    VsLog::warningLog() <<"VsH5Attribute::getFloatVectorValue() - Requested attribute " <<getShortName()
         <<" is not a floating point vector." <<std::endl;
    fvals->resize(0);
    return -1;
  }
  
  if (rank == 0) {
    fvals->resize(1);
    float v;
    err = H5Aread(getId(), H5T_NATIVE_FLOAT, &v);
    (*fvals)[0] = v;
    return err;
  }
  
  // rank>0
  npoints = H5Sget_simple_extent_npoints(aspace);
  float* v = new float[npoints];
  err = H5Aread(getId(), H5T_NATIVE_FLOAT, v);
  fvals->resize(npoints);
  for (size_t i = 0; i<npoints; ++i) {
    (*fvals)[i] = v[i];
  }
  delete v;
  
  return err;
}

// JRC: This fat interface may not scale?  What about
// scalar attributes?
herr_t VsH5Attribute::getDoubleVectorValue(std::vector<double>* dvals) {
  herr_t err = 0;
  size_t npoints;
  hid_t atype = H5Aget_type(getId());
  H5T_class_t type = H5Tget_class(atype);
  hid_t aspace = H5Aget_space(getId());
  size_t rank = H5Sget_simple_extent_ndims(aspace);

  if (type != H5T_FLOAT) {
    VsLog::warningLog() <<"VsH5Attribute::getDoubleVectorValue() - Requested attribute " <<getShortName()
         <<" is not a floating point vector." <<std::endl;
    dvals->resize(0);
    return -1;
  }
  
  if (rank == 0) {
    dvals->resize(1);
    double v;
    err = H5Aread(getId(), H5T_NATIVE_DOUBLE, &v);
    (*dvals)[0] = v;
    return err;
  }
  
  // rank>0
  npoints = H5Sget_simple_extent_npoints(aspace);
  double* v = new double[npoints];
  err = H5Aread(getId(), H5T_NATIVE_DOUBLE, v);
  dvals->resize(npoints);
  for (size_t i = 0; i<npoints; ++i) {
    (*dvals)[i] = v[i];
  }
  delete v;
  
  return err;
}
