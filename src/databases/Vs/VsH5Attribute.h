/**
 * @file VsH5Attribute.h
 *      
 *  @class VsH5Attribute
 *  @brief Represents an hdf5 attribute from an hdf5 file.
 *  
 *  Saves the name and hdf5 id of an attribute.  Provides
 *  accessors to retrieve the actual value of the attribute
 *  from the file.
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#ifndef VSH5ATTRIBUTE_H_
#define VSH5ATTRIBUTE_H_

#include "VsH5Object.h"
#include <vector>
#include <string>
#include <map>

class VsH5Attribute: public VsH5Object {
public:
  VsH5Attribute(VsH5Object* parentObject, std::string attName, hid_t id);
  virtual ~VsH5Attribute();
  
  void write();
  hid_t getType() { return dataType; }
  size_t getLength();

  // Get the value of this attribute
  // CAUTION: Calling the incorrect accessor will result in invalid or null data
  herr_t getStringValue(std::string* val);
  herr_t getIntVectorValue(std::vector<int>* ivals);
  herr_t getFloatVectorValue(std::vector<float>* fvals);
  herr_t getDoubleVectorValue(std::vector<double>* dvals);
  
private:
  hid_t dataType;
  std::vector<int> dims;
};

#endif /* VSH5ATTRIBUTE_H_ */
