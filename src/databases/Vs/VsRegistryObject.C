/*
 * VsRegistryObject.C
 *
 *  Created on: Apr 30, 2010
 *      Author: mdurant
 */

#include "VsRegistryObject.h"
#include "VsRegistry.h"

VsRegistryObject::VsRegistryObject(VsRegistry* r) {
  registry = r;
}

VsRegistry* VsRegistryObject::getRegistry() {
  return registry;
}

VsRegistryObject::~VsRegistryObject() {
}
