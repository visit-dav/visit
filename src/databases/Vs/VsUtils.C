#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1,8,1)
#include <VsUtils.h>

// JRC: This fat interface may not scale?  What about
// scalar attributes?
herr_t getAttributeHelper(const hid_t id, std::string* sval, std::vector<int>* ivals,
    std::vector<float>* fvals) {

  herr_t err = 0;
  size_t npoints;
  hid_t atype = H5Aget_type(id);
  H5T_class_t type = H5Tget_class(atype);
  hid_t aspace = H5Aget_space(id);
  size_t rank = H5Sget_simple_extent_ndims(aspace);

/*
 hsize_t sdim[rank];
 ret = H5Sget_simple_extent_dims(aspace, sdim, NULL);
 dims->resize(rank);
 for (size_t i = 0; i < rank; ++i)
 (*dims)[i] = sdim[i];
 */

  if (type == H5T_INTEGER) {
    if (rank == 0) {
      ivals->resize(1);
      int v;
      // err = H5Aread(id, atype, &v);
      err = H5Aread(id, H5T_NATIVE_INT, &v);
      (*ivals)[0] = v;
      return err;
    }
// rank>0
    npoints =  H5Sget_simple_extent_npoints(aspace);
    int* v  = new int[npoints];
    err = H5Aread(id, H5T_NATIVE_INT, v);
    ivals->resize(npoints);
    for (size_t i = 0; i<npoints; ++i) {
      (*ivals)[i] = v[i];
    }
    delete v;
    return err;
  }

  if (type == H5T_FLOAT) {
    if (rank == 0) {
      fvals->resize(1);
      float v;
      err = H5Aread(id, H5T_NATIVE_FLOAT, &v);
      (*fvals)[0] = v;
      return err;
    }
// rank>0
    npoints =  H5Sget_simple_extent_npoints(aspace);
    float* v  = new float[npoints];
    err = H5Aread(id, H5T_NATIVE_FLOAT, v);
    fvals->resize(npoints);
    for (size_t i = 0; i<npoints; ++i) {
      (*fvals)[i] = v[i];
    }
    delete v;
    return err;
  }

  if (type == H5T_STRING) {
    if (rank != 0) {
      return -1;
    }
    size_t len = H5Aget_storage_size(id);
    sval->resize(len);
    char* v = new char[len];
    err = H5Aread(id, atype, v);
// JRC: is this right?
    // err = H5Aread(id, H5T_NATIVE_CHAR, v);
    for (size_t i = 0; i < len; ++i) {
      (*sval)[i] = v[i];
    }
    delete [] v;
    return err;
  }
  return err;

}

/*
void parseVars(const string& vsVars, vector<string>& vars) {
   size_t i1;
   size_t i2;    // JRC: This needs to be initialized!
   char del = '\"';
   i1 = 0;
   int inc = -1;
   while (i2 != string::npos) {
// inc alternates so that every other word between " is included
    i2 = vsVars.find(del, i1);
    if (inc>0) vars.push_back(vsVars.substr(i1, i2-i1));
    i1 = i2+1;
    inc *= -1;
  }
}
*/

void getDims(hid_t id, bool isDataset, std::vector<int>& dims) {
  hid_t space;
  if (!isDataset) space = H5Aget_space(id);
  else space = H5Dget_space(id);
  size_t rank = H5Sget_simple_extent_ndims(space);
  hsize_t sdim[rank];
  H5Sget_simple_extent_dims(space, sdim, NULL);
  dims.resize(rank);
  for (size_t i = 0; i < rank; ++i) {
    dims[i] = sdim[i];
  }
}

std::string makeCanonicalName(std::string name) {
  std::string answer = name;
  if ((name.length() > 0) && (name[0] == '/')) {
    answer = name.substr(1, name.length() - 1);
  }

  return answer;
}

std::string makeCanonicalName(std::string path, std::string name) {
  std::string answer = name;
  //only prepend the path if it is not empty, and if "name" does not start with '/'
  if ((path.length() > 0) && (name.length() > 0) && (name[0] != '/')) {
      answer = path + "/" + name;
  }
  //remove the leading slash if it exists
  if ((answer.length() > 0) && (answer[0] == '/')) {
    answer = answer.substr(1, answer.length());
  }

  return answer;
}

// Compare two object names to a target name
// Return the name that is "closest" to the target
// If the two object names are identical, returns the first (but doesn't matter)
// If the two object names are equally distant from the target, returns the first
//  i.e. getClosestName("abcX", "abcY", "name") == "abcX"
std::string getClosestName(std::string name1, std::string name2, std::string target) {
  //go through the two candidate names
  //find the first disagreement,
  //and see switch matches the target

  //Easy cases
  if (name1 == target)
    return name1;
  if (name2 == target)
    return name2;
  if (name1 == name2)
    return name1;

  //ok, now the real cases
  size_t shortestNameLength = std::min(name1.length(), name2.length());
  shortestNameLength = std::min(shortestNameLength, target.length());

  for (size_t i = 0; i < shortestNameLength; i++) {
    if (name1[i] != name2[i]) {
      //found a disagreeing character
      //which matches the target?
      if (name2[i] == target[i])
        return name2;
      //default behavior is to return name1
      return name1;
    }
  }

  //at this point, we know the following:
  // (name1 != name2 != target)
  // name1[0:n] == name2[0:n] == target[0:n] (where n == shortestNameLength)
  // so look at the next character
  std::string targetChar = "";
   if (target.length() > shortestNameLength)
     targetChar = target[shortestNameLength];

  std::string name1Char = "";
  if (name1.length() > shortestNameLength)
    name1Char = name1[shortestNameLength];

  std::string name2Char = "";
  if (name2.length() > shortestNameLength)
    name2Char = name2[shortestNameLength];

  //make the final decision
  if (name2Char == targetChar)
    return name2;

  //default behavior
  return name1;
}

//Adjusts var dimensions stored in an array of hsize_t
void adjustSize_hsize_t(hsize_t *dims, unsigned int rank, std::vector<int> stride, int before, int after) {
        //apply transform to each dimension
        for (unsigned int i = 0; i < rank; i++) {
                dims[i] += before;

                dims[i] = dims[i] / stride[i];
                //Don't allow the dimension to go below 1
                if (dims[i] < 1) {
                        dims[i] = 1;
                }

                dims[i] += after;
        }
}

//Adjusts var dimensions stored in a vector int
void adjustSize_vector(std::vector<int>* dims, int rank, std::vector<int> stride, int before, int after) {
        //apply transform to each dimension
        for (int i = 0; i < rank; i++) {
                (*dims)[i] += before;

                (*dims)[i] = (*dims)[i] / stride[i];
                //Don't allow the dimension to go below 1
                if ((*dims)[i] < 1) {
                        (*dims)[i] = 1;
                }

                (*dims)[i] += after;
  }
}
#endif
