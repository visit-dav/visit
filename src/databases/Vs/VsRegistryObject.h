/**
 * @file VsRegistryObject.h
 *      
 *  @class VsRegistryObject
 *  @brief Superclass for all objects that will link to the VsRegistry object.
 *  
 *  Provides a pointer to the VsRegistry object.
 *
 *  Created on: Apr 30, 2010
 *      Author: mdurant
 */

#ifndef VSREGISTRYOBJECT_H_
#define VSREGISTRYOBJECT_H_

#include "VsRegistry.h"

class VsRegistryObject {
public:
  
  VsRegistryObject(VsRegistry* r);
  virtual ~VsRegistryObject();
  VsRegistry* getRegistry();
  virtual void write() const = 0;
  
  VsRegistry* registry;
};

#endif /* VSREGISTRYOBJECT_H_ */
