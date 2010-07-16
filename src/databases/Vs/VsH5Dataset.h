/**
 * @file VsH5Dataset.h
 *      
 *  @class VsH5Dataset
 *  @brief Represents a dataset from an hdf5 file.
 *  
 *  Provides accessors to retrieve attributes and data from
 *  the hdf5 file.
 *
 *  Created on: Apr 27, 2010
 *      Author: mdurant
 */

#ifndef VSH5DATASET_H_
#define VSH5DATASET_H_

#include <string>
#include "VsH5Object.h"
#include <hdf5.h>
#include <vector>
#include <map>

class VsH5Attribute;

class VsH5Dataset : public VsH5Object {
public:
  VsH5Dataset(VsRegistry* r, VsH5Object* parentObject, std::string datasetName, hid_t id);
  virtual ~VsH5Dataset();
  
  void write();
  size_t getLength();
  std::vector<int> getDims();
  hid_t getType() { return dataType; }
  
private:
  std::vector<int> dims;
  hid_t dataType;
  std::vector<VsH5Attribute*> attributes;

  void loadDims();
};

#endif /* VSH5DATASET_H_ */
