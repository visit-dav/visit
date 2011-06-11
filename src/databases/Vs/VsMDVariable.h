/**
 * @file VsMDVariable.h
 *      
 *  @class VsMDVariable
 *  @brief Represents a multi-domain variable - made up of other VsVariable objects.
 *  
 *  A VsMDVariable allows the grouping of multiple VsVariable objects into a single variable.
 *  Note that VsVariableWithMesh objects are NEVER part of an MDVariable.
 *
 *  Created on: Apr 29, 2010
 *      Author: mdurant
 */

#ifndef VSMDVARIABLE_H_
#define VSMDVARIABLE_H_

#include "VsRegistryObject.h"

#include <string>
#include <hdf5.h>
#include <vector>
#include <map>

class VsVariable;

class VsMDVariable : public VsRegistryObject {
public:
  VsMDVariable(VsVariable* firstVar, VsMDMesh* mdMesh, std::string mdVarName);
  virtual ~VsMDVariable();
  std::string getLabel(int componentIndex);
  bool isZonal();
  bool addBlock(VsVariable* newBlock);
  unsigned int getNumBlocks();
  std::string getNameForBlock(unsigned int domain);
  VsVariable* getBlock(unsigned int domain);   
  std::string getMesh();
  std::string getCentering();
  hid_t getType();
  std::string getName();
  std::string getFullName();
  void write();
  
  /** List of all subordinate blocks in this var
   * Order is important (block 0 = blocks[0], etc)*/
  std::vector<VsVariable*> blocks;  //TODO make private

  void createComponents();
  size_t getNumComps();
  
private:
  int numComponents;
  
  std::string name;

  // Name of the mesh
  std::string mesh;

  // Component order, if defined
  std::string indexOrder;

  // Centering, if defined
  std::string centering;

  //fully qualified path to this object
  std::string path;
};

#endif /* VSMDVARIABLE_H_ */
