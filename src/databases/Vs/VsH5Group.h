/**
 * @file VsH5Group.h
 *      
 *  @class VsH5Group
 *  @brief Represents an hdf5 group from an hdf5 file.
 *  
 *  Provides accessors to get attributes and datasets 
 *  from a group in an hdf5 file.
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VSH5GROUP_H_
#define VSH5GROUP_H_

#include <string>
#include <vector>
#include <map>

#include "VsH5Object.h"
#include <hdf5.h>

class VsH5Attribute;
class VsH5Dataset;

class VsH5Group : public VsH5Object {
public:
  VsH5Group(VsRegistry* r, VsH5Object* parentObject, std::string groupName, hid_t id);
  virtual ~VsH5Group();
  
  void write();
  
private:

};

#endif /* VSH5GROUP_H_ */
