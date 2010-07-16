/**
 * @file VsH5File.h
 *      
 *  @class VsH5File
 *  @brief Represents an hdf5 data file
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VSH5FILE_H_
#define VSH5FILE_H_

#include <string>
#include "VsH5Group.h"
#include "VsH5Object.h"
#include <hdf5.h>

class VsH5File : public VsH5Object {
public:
  VsH5File(VsRegistry* r, std::string name, hid_t id);
  virtual ~VsH5File();
  
  std::string getName();
  void write();

private:
  std::string fileName;
};

#endif /* VSH5FILE_H_ */
