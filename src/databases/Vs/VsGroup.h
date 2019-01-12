/**
 * @file VsGroup.h
 *      
 *  @class VsGroup
 *  @brief Represents an hdf5 group from an hdf5 file.
 *  
 *  Provides accessors to get attributes and datasets 
 *  from a group in an hdf5 file.
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VS_GROUP_H_
#define VS_GROUP_H_

#include <string>
#include <vector>
#include <map>

#include "VsObject.h"
#include <hdf5.h>

class VsAttribute;
class VsDataset;

class VsGroup : public VsObject {
public:
  VsGroup(VsRegistry* r, VsObject* parentObject, std::string groupName, hid_t id);
  virtual ~VsGroup();
  
  void write() const;
  
};

#endif /* VS_GROUP_H_ */
