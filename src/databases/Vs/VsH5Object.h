/**
 * @file VsH5Object.h
 *  
 *  @class VsH5Object
 *  @brief Superclass for all objects present in an hdf5 file.
 *  
 *  Provides a common interface for all objects that are
 *  directly present in an hdf5 file.  Maintains the hierarchy
 *  structure present in the data file.
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VSH5OBJECT_H_
#define VSH5OBJECT_H_

#include <string>
#include <hdf5.h>
#include <vector>
#include <map>
#include "VsRegistryObject.h"

class VsH5Attribute;

class VsH5Object : public VsRegistryObject {
public:
  VsH5Object(VsRegistry* r, VsH5Object* parentObject, std::string objectName, hid_t id);
  virtual ~VsH5Object();
  
  std::string getShortName();
  std::string getFullName();
  std::string getPath();
  hid_t getId();
  int getDepth();
  
  void addAttribute(std::string name, hid_t id);
  VsH5Attribute* getAttribute(std::string name);
  std::map<std::string, VsH5Attribute*>* getAllAttributes();
  
private:
  std::string name;
  std::string fullName;
  VsH5Object* parent;
  hid_t resourceId;
  int depth;
  
  std::map<std::string, VsH5Attribute*> attributes;
  
};

#endif /* VSH5OBJECT_H_ */
