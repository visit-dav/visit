/**
 * @file VsObject.h
 *  
 *  @class VsObject
 *  @brief Superclass for all objects present in an hdf5 file.
 *  
 *  Provides a common interface for all objects that are
 *  directly present in an hdf5 file.  Maintains the hierarchy
 *  structure present in the data file.
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VS_OBJECT_H_
#define VS_OBJECT_H_

#include <string>
#include <hdf5.h>
#include <vector>
#include <map>
#include "VsRegistryObject.h"

class VsAttribute;

class VsObject : public VsRegistryObject {
public:
  VsObject(VsRegistry* r, VsObject* parentObject, 
           const std::string& objectName, hid_t id);
  virtual ~VsObject();
  
  std::string getShortName() const;
  std::string getFullName() const;
  std::string getPath() const;
  hid_t getId() const;
  int getDepth() const;
  
  void addAttribute(const std::string name, hid_t id);
  VsAttribute* getAttribute(const std::string name) const;
  const std::map<std::string, VsAttribute*>& getAllAttributes() const;
  
private:
  std::string name;
  std::string fullName;
  VsObject* parent;
  hid_t resourceId;
  int depth;
  
  std::map<std::string, VsAttribute*> attributes;
  
};

#endif /* VS_OBJECT_H_ */
