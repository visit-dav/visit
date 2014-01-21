#include <hdf5.h>
#include <VsUtils.h>
#include <VsLog.h>

/*
void getDims(hid_t id, bool isDataset, std::vector<int>& dims) {
  hid_t space;
  if (!isDataset) space = H5Aget_space(id);
  else space = H5Dget_space(id);
  size_t rank = H5Sget_simple_extent_ndims(space);
  std::vector<hsize_t> sdim(rank);
  H5Sget_simple_extent_dims(space, &sdim[0], NULL);
  dims.resize(rank);
  for (size_t i = 0; i < rank; ++i) {
    dims[i] = sdim[i];
  }
}
*/
std::string makeCanonicalName(const std::string& name) {
  std::string answer = name;

  //remove the leading slash(es) if it(they) exist(s)
  while ((answer.length() > 0) && (answer[0] == '/')) {
    answer = answer.substr(1, answer.length() - 1);
  }

  return answer;
}

std::string makeCanonicalName(const std::string& path, 
                              const std::string& name) {
  std::string answer = name;
  //only prepend the path if it is not empty, and if "name" does not start with '/'
  if ((path.length() > 0) && (name.length() > 0) && (name[0] != '/')) {
    answer = path + "/" + name;
  }

  //remove the leading slash(es) if it(they) exist(s)
  while ((answer.length() > 0) && (answer[0] == '/')) {
    answer = answer.substr(1, answer.length());
  }

  return answer;
}

bool isDoubleType(hid_t dataType) {
    if (H5Tequal(dataType, H5T_NATIVE_DOUBLE)) {
      return true;
    }
    
    hid_t nativeType = H5Tget_native_type(dataType, H5T_DIR_ASCEND);
    bool answer = H5Tequal(nativeType, H5T_NATIVE_DOUBLE);
    
    H5Tclose(nativeType);
    return answer;
}

bool isFloatType(hid_t dataType) {
  if (H5Tequal(dataType, H5T_NATIVE_FLOAT)) {
        return true;
      }
      
  hid_t nativeType = H5Tget_native_type(dataType, H5T_DIR_ASCEND);
  bool answer = H5Tequal(nativeType, H5T_NATIVE_FLOAT);
  
  H5Tclose(nativeType);
  return answer;
}

bool isIntType(hid_t dataType) {
  if (H5Tequal(dataType, H5T_NATIVE_INT)) {
    return true;
  }
      
  hid_t nativeType = H5Tget_native_type(dataType, H5T_DIR_ASCEND);
  bool answer = H5Tequal(nativeType, H5T_NATIVE_INT);
  
  H5Tclose(nativeType);
  return answer;
}

bool isShortType(hid_t dataType) {
  if (H5Tequal(dataType, H5T_NATIVE_SHORT)) {
    return true;
  }
      
  hid_t nativeType = H5Tget_native_type(dataType, H5T_DIR_ASCEND);
  bool answer = H5Tequal(nativeType, H5T_NATIVE_SHORT);
  
  H5Tclose(nativeType);
  return answer;
}

bool isCharType(hid_t dataType) {
  if (H5Tequal(dataType, H5T_NATIVE_CHAR)) {
    return true;
  }
      
  hid_t nativeType = H5Tget_native_type(dataType, H5T_DIR_ASCEND);
  bool answer = H5Tequal(nativeType, H5T_NATIVE_CHAR);
  
  H5Tclose(nativeType);
  return answer;
}

bool isUnsignedCharType(hid_t dataType) {
  if (H5Tequal(dataType, H5T_NATIVE_UCHAR)) {
    return true;
  }
      
  hid_t nativeType = H5Tget_native_type(dataType, H5T_DIR_ASCEND);
  bool answer = H5Tequal(nativeType, H5T_NATIVE_UCHAR);
  
  H5Tclose(nativeType);
  return answer;
}

void printType(hid_t dataType) {
  if (H5Tequal(dataType, H5T_IEEE_F64BE)) VsLog::debugLog() <<"H5T_IEEE_F64BE";
  else if (H5Tequal(dataType, H5T_NATIVE_CHAR)) VsLog::debugLog() <<"H5T_NATIVE_CHAR";
  else if (H5Tequal(dataType, H5T_NATIVE_SCHAR)) VsLog::debugLog() <<"H5T_NATIVE_SCHAR";
  else if (H5Tequal(dataType, H5T_NATIVE_UCHAR)) VsLog::debugLog() <<"H5T_NATIVE_UCHAR";
  else if (H5Tequal(dataType, H5T_NATIVE_SHORT)) VsLog::debugLog() <<"H5T_NATIVE_SHORT";
  else if (H5Tequal(dataType, H5T_NATIVE_USHORT)) VsLog::debugLog() <<"H5T_NATIVE_USHORT";
  else if (H5Tequal(dataType, H5T_NATIVE_INT)) VsLog::debugLog() <<"H5T_NATIVE_INT";
  else if (H5Tequal(dataType, H5T_NATIVE_UINT)) VsLog::debugLog() <<"H5T_NATIVE_UINT";
  else if (H5Tequal(dataType, H5T_NATIVE_LONG)) VsLog::debugLog() <<"H5T_NATIVE_LONG";
  else if (H5Tequal(dataType, H5T_NATIVE_ULONG)) VsLog::debugLog() <<"H5T_NATIVE_ULONG";
  else if (H5Tequal(dataType,H5T_NATIVE_LLONG)) VsLog::debugLog() <<"H5T_NATIVE_LLONG";
  else if (H5Tequal(dataType, H5T_NATIVE_ULLONG)) VsLog::debugLog() <<"H5T_NATIVE_ULLONG";
  else if (H5Tequal(dataType, H5T_NATIVE_FLOAT)) VsLog::debugLog() <<"H5T_NATIVE_FLOAT";
  else if (H5Tequal(dataType, H5T_NATIVE_DOUBLE)) VsLog::debugLog() <<"H5T_NATIVE_DOUBLE";
  else if (H5Tequal(dataType, H5T_NATIVE_LDOUBLE)) VsLog::debugLog() <<"H5T_NATIVE_LDOUBLE";
  else if (H5Tequal(dataType, H5T_NATIVE_HSIZE)) VsLog::debugLog() <<"H5T_NATIVE_HSIZE";
  else if (H5Tequal(dataType, H5T_NATIVE_HSSIZE)) VsLog::debugLog() <<"H5T_NATIVE_HSSIZE";
  else if (H5Tequal(dataType, H5T_NATIVE_HERR)) VsLog::debugLog() <<"H5T_NATIVE_HERR";
  else if (H5Tequal(dataType, H5T_NATIVE_HBOOL)) VsLog::debugLog() <<"H5T_NATIVE_HBOOL";
  else if (dataType == -1) VsLog::debugLog() <<"Negative 1";
  else VsLog::debugLog() <<"Unknown type";
}

// Compare two object names to a target name
// Return the name that is "closest" to the target
// If the two object names are identical, returns the first (but doesn't matter)
// If the two object names are equally distant from the target, returns the first
//  i.e. getClosestName("abcX", "abcY", "name") == "abcX"
/*std::string getClosestName(std::string name1, std::string name2, std::string target) {
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
}*/

// Break a string into pieces
void tokenize(const std::string& text, 
              char separator, std::vector<std::string>& tokens) {
  //start with a clean list of tokens
  tokens.clear();
  
  std::string nameBuffer;
  nameBuffer.clear();
  
  for (unsigned int i = 0; i < text.length(); i++) {
    char currentChar = text[i];
    if (currentChar == separator) {
      //we have finished the current token
      //so save it and start a new token
      tokens.push_back(nameBuffer);
      nameBuffer.clear();
    } else if (currentChar != ' ') {
      //ignore leading spaces
      //all other characters get appended to the token
      nameBuffer.push_back(currentChar);
    }
  }
  
  //is the last name still in the buffer?
  if (!nameBuffer.empty()) {
    tokens.push_back(nameBuffer);
    nameBuffer.clear();
  }
  
}

//Adjusts var dimensions stored in an array of hsize_t
void adjustSize_hsize_t(hsize_t *dims, unsigned int rank, 
                        const std::vector<int>& stride, 
                        int before, int after) {
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
void adjustSize_vector(std::vector<int>* dims, int rank, 
                       const std::vector<int>& stride, 
                       int before, int after) {
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
