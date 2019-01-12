/**
 * @file VsAttribute.h
 *      
 *  @class VsAttribute
 *  @brief Represents an hdf5 attribute from an hdf5 file.
 *  
 *  Saves the name and hdf5 id of an attribute.  Provides
 *  accessors to retrieve the actual value of the attribute
 *  from the file.
 *
 *  Created on: Apr 28, 2010
 *      Author: mdurant
 */

#ifndef VS_ATTRIBUTE_H_
#define VS_ATTRIBUTE_H_

#include "VsObject.h"
#include <vector>
#include <string>
#include <map>

class VsAttribute: public VsObject {
public:
  VsAttribute(VsObject* parentObject, std::string attName, hid_t id);
  virtual ~VsAttribute();
  
  void write() const;
  hid_t getType() const { return dataType; }
  size_t getLength() const;

  // Get the value of this attribute
  // CAUTION: Calling the incorrect accessor will result in invalid or null data
  int getStringValue(std::string* val);
  int getIntVectorValue(std::vector<int>* ivals);
  int getFloatVectorValue(std::vector<float>* fvals);
  int getDoubleVectorValue(std::vector<double>* dvals);
  
private:
  hid_t dataType;
  std::vector<int> dims;
};

#endif /* VS_ATTRIBUTE_H_ */
