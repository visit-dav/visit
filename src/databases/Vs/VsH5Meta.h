#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1, 8, 1)
/**
 * @file  VsH5Meta.h
 *
 * @class VsH5Meta
 *
 * @brief VsH5Meta contains the VizSchema HDF5 data.
 *
 * groups and datasets that are meshes, datasets that are variables
 * and variables with meshes.
 *
 * Copyright &copy; 2007-2008 by Tech-X Corporation
 */

#ifndef VS_H5_META_H
#define VS_H5_META_H

// std includes
#include <map>
#include <iostream>
#include <string>
#include <vector>

// HDF5
#include <hdf5.h>

// Local
#include "VsUtils.h"

// Attribute metadata
struct VsAMeta {

  VsAMeta() {
    this->depth = 0;
  }

  // Virtual distructor
  virtual ~VsAMeta() {
    H5Aclose(aid);
  }

  // Name
  std::string name;

  // Numerical type
  hid_t type;

  // Dimensions
  std::vector<int> dims;

  // Total length
  size_t getLength() {
    size_t len = 1;
    for (size_t i = 0; i < dims.size(); ++i) len *= dims[i];
    return len;
  }

  // HDF5 Id
  hid_t aid;

  // Depth in the hierarchy
  size_t depth;

  void write(std::ostream& os, const std::string& x = "attribute ") const {
    std::string offset = "";
    for(size_t i = 0; i<depth; ++i) offset += "  ";
    os << offset<< "Attribute "<< this->name << std::endl;
    os << offset<< "  rank = " << dims.size() << std::endl;
  }

};

// Metadata for interfaces (groups and datasets)
struct VsIMeta {

  VsIMeta () {
    this->isGroup = false;
    this->isMesh = 0;
    this->isVariable = 0;
    this->isVariableWithMesh = 0;
    this->depth = 0;
    this->name = "error";
    this->path = "error";
  }

  virtual ~VsIMeta() {
    std::vector<VsAMeta*>::const_iterator i;
    for (i = attribs.begin(); i != attribs.end(); ++i)
    delete (*i);
    attribs.clear();
  }

  // Name
  std::string name;

  // Fully qualified name
  std::string path;

  // HDF5 ID
  hid_t iid;

  // Depth in the hierarchy
  size_t depth;

  // List of attributes
  std::vector<VsAMeta*> attribs;

  std::string getFullName() const {
    return makeCanonicalName(path, name);
  }

  // Write attributes
  void write(std::ostream& os) const {
    if (attribs.empty()) return;
    std::vector<VsAMeta*>::const_iterator i;
    for (i=attribs.begin(); i != attribs.end(); ++i)
    (*i)->write(os);
  }
  // Find attribute by name, or return NULL if not found
  const VsAMeta* find(const std::string name) const {
    std::vector<VsAMeta*>::const_iterator i;
    for (i=attribs.begin(); i != attribs.end(); ++i) {
      if ((*i)->name == name) return *i;
    }
    return NULL;
  }

  // Is it a group or dataset
  bool isGroup;

  // Is it a variable
  bool isVariable;

  // Is it a variable with mesh
  bool isVariableWithMesh;

  // Is it a mesh
  bool isMesh;

  std::string getStringAttribute(std::string attName) const {
    const VsAMeta* att = find(attName);
    if (!att) {
      return "";
    }
    std::string result;
    getAttributeHelper(att->aid, &result, 0, 0);
    return result;
  }

};

// Metadata for datasets
struct VsDMeta : public VsIMeta {

  // Virtual destructor
  virtual ~VsDMeta() {
    H5Dclose(iid);
  }

  // Dimensions
  std::vector<int> dims;

  // Total length
  size_t getLength() {
    size_t len = 1;
    for (size_t i = 0; i < dims.size(); ++i) len *= dims[i];
    return len;
  }

  // Numerical type
  hid_t type;

  void write(std::ostream& os) const {
    std::string offset = "";
    for(size_t i = 0; i<depth; ++i) offset += "  ";
    os << offset<< "Dataset "<< this->name << std::endl;
    os << offset<<"Fully qualified name: " <<getFullName() <<std::endl;
    VsIMeta::write(os);
  }

};

// Metadata for groups
struct VsGMeta : public VsIMeta {

  VsGMeta() {
    isGroup= true;
    isVsVars = false;
  }

  virtual ~VsGMeta () {
    std::vector<VsDMeta*>::const_iterator i;
    for (i = datasets.begin(); i != datasets.end(); ++i) delete (*i);
    datasets.clear();
    H5Gclose(iid);
  }

  // List of datasets
  std::vector<VsDMeta*> datasets;

  // Is it a vsVars group?
  bool isVsVars;

  void write(std::ostream& os) const {
    std::string offset = "";
    for(size_t i = 0; i<depth; ++i) offset += "  ";
    os << offset << "Group = " << name << std::endl;
    std::vector<VsDMeta*>::const_iterator i;
    for (i=datasets.begin(); i != datasets.end(); ++i) (*i)->write(os);
    VsIMeta::write(os);
  }

};

struct VsH5Meta {

  // This struct is a list of all groups which are meshes,
  // all datasets that are meshes, varsWithMeshes and variables.
  // Each such dataset and group has a list of their attributes only.

  VsH5Meta() : ptr(NULL) {
  }

  virtual ~VsH5Meta() {
    clear();
  }

 const bool hasObjects() const {
    return !(gMeshes.empty() && dMeshes.empty() && vars.empty() && varsWithMesh.empty() && vsVars.empty());
  }

  //list of all datasets
  std::map<std::string, VsDMeta*> datasets;
  const VsDMeta* getDataset (const std::string& name) const {
    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //     std::cerr << "Datasets  - Looking for " << fullName << " from " << name << std::endl;
    //     std::map<std::string, VsDMeta*>::const_iterator i;
    //     for (i = datasets.begin(); i != datasets.end(); ++i) std::cerr << (const std::string)i->first << std::endl;

    //look for fully qualified name
    std::map<std::string, VsDMeta*>::const_iterator it = datasets.find(fullName);
    if (it != datasets.end()) {
      //found it!
      return (const VsDMeta*)it->second;
    }

    return NULL;
  }
  
  //list of "orphaned" datasets - i.e. have no known parent
  std::vector<VsDMeta*> orphanDatasets;

  // List of group-meshes
  std::map<std::string, VsGMeta*> gMeshes;
  const VsGMeta* getGMesh(const std::string& name) const {
    //if the name does not start with a "/",
    // it is not fully qualified
    // and we assume it is in the root group
    // If not found in root, we search the namespace
    // and if we find that mesh, and it's unique, we return it
    // If there is a name conflict, we return 0

    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //look for fully qualified name
    std::map<std::string, VsGMeta*>::const_iterator it = gMeshes.find(fullName);
    if (it != gMeshes.end()) {
      //found it!
      return (const VsGMeta*)it->second;
    }

    //Didn't find fully qualified name, do a global search for simple name
    VsGMeta* foundMesh = 0;
    for (it = gMeshes.begin(); it != gMeshes.end(); ++it) {
      VsGMeta* candidate = it->second;
      if (candidate->name.compare(name) == 0) {
        //Found a matching name
        //But have we already found a match, and this is a conflict?
        if (foundMesh != 0) {
          //THIS NAME IS A CONFLICT IN A FLATTENED NAMESPACE
          //so we're unable to decide which to return
          return 0;
        }
        foundMesh = candidate;
      }
    } //end loop

    return (const VsGMeta*)foundMesh; //might still be zero
  }

  // List of dataset-meshes
  std::map<std::string, VsDMeta*> dMeshes;
  const VsDMeta* getDMesh(const std::string& name) const {
    //if the name does not start with a "/",
    // it is not fully qualified
    // and we assume it is in the root group
    // If not found in root, we search the namespace
    // and if we find that mesh, and it's unique, we return it
    // If there is a name conflict, we return 0

    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //look for fully qualified name
    std::map<std::string, VsDMeta*>::const_iterator it = dMeshes.find(fullName);
    if (it != dMeshes.end()) {
      //found it!
      return (const VsDMeta*)it->second;
    }

    //not found, do global search
    VsDMeta* foundMesh = 0;
    for (it = dMeshes.begin(); it != dMeshes.end(); ++it) {
      VsDMeta* candidate = it->second;
      if (candidate->name.compare(name) == 0) {
        //Found a matching name
        //But have we already found a match, and this is a conflict?
        if (foundMesh != 0) {
          //THIS NAME IS A CONFLICT IN A FLATTENED NAMESPACE
          //so we're unable to decide which to return
          return 0;
        }
        foundMesh = candidate;
      }
    } //end loop

    return (const VsDMeta*)foundMesh; //might be zero
  }

  // List of dataset-variables
  std::map<std::string, VsDMeta*> vars;
  const VsDMeta* getVar(const std::string& name) const {
    //if the name does not start with a "/",
    // it is not fully qualified
    // and we assume it is in the root group
    // If not found in root, we search the namespace
    // and if we find that mesh, and it's unique, we return it
    // If there is a name conflict, we return 0

    // Make name fully qualified
    std::string fullName = makeCanonicalName(name);

    //look for fully qualified name
    std::map<std::string, VsDMeta*>::const_iterator it = vars.find(fullName);
    if (it != vars.end()) {
      //found it
      return (const VsDMeta*)it->second;
    }

    //not found, do global search
    VsDMeta* foundVar = 0;
    for (it = vars.begin(); it != vars.end(); ++it) {
      VsDMeta* candidate = it->second;
      if (candidate->name.compare(name) == 0) {
        //Found a matching name
        //But have we already found a match, and this is a conflict?
        if (foundVar != 0) {
          //THIS NAME IS A CONFLICT IN A FLATTENED NAMESPACE
          //so we're unable to decide which to return
          return 0;
        }
        foundVar = candidate;
      }
    } //end loop

    return (const VsDMeta*)foundVar; //might be zero
  }

  // List of dataset-variable with meshes
  std::map<std::string, VsDMeta*> varsWithMesh;

  // List of vsVars groups
  std::map<std::string, VsGMeta*> vsVars;

  // Pointer to the last iterated group
  void* ptr;

  // Clean itself
  void clear() {

    // Need to delete all pointers
    std::map<std::string, VsGMeta*>::const_iterator i;
    for (i=gMeshes.begin(); i != gMeshes.end(); ++i)
      delete i->second;
    for(i=vsVars.begin(); i !=vsVars.end(); ++i)
      delete i->second;

    std::map<std::string, VsDMeta*>::const_iterator k;
    for (k=dMeshes.begin(); k != dMeshes.end(); ++k)
      delete k->second;
    for (k=vars.begin(); k != vars.end(); ++k)
      delete k->second;
    for (k=varsWithMesh.begin(); k != varsWithMesh.end(); ++k)
      delete k->second;

    std::vector<VsDMeta*>::const_iterator j;
    for (j=orphanDatasets.begin(); j != orphanDatasets.end(); ++j)
      delete (*j);

    // Clean lists
    gMeshes.clear();
    dMeshes.clear();
    vars.clear();
    varsWithMesh.clear();
    vsVars.clear();
    orphanDatasets.clear();
  }

  // Write
  void write(std::ostream& os) const {
    std::map<std::string, VsGMeta*>::const_iterator i;
    if (gMeshes.size()) os << "gMeshes"<<std::endl;
    for (i=gMeshes.begin(); i != gMeshes.end(); ++i) {
      i->second->write(os);
    }

    std::map<std::string, VsDMeta*>::const_iterator k;

    if (dMeshes.size()) os <<"dMeshes"<< std::endl;
    for (k=dMeshes.begin(); k != dMeshes.end(); ++k) {
      k->second->write(os);
    }

    if (vars.size()) os << "vars" << std::endl;
    for (k=vars.begin(); k != vars.end(); ++k) {
      k->second->write(os);
    }

    if (varsWithMesh.size()) os << "varsWithMesh" << std::endl;
    for (k=varsWithMesh.begin(); k != varsWithMesh.end(); ++k) {
      k->second->write(os);
    }

    if (vsVars.size()) os <<"vsVars" << std::endl;
    for (i=vsVars.begin(); i !=vsVars.end(); ++i) {
      i->second->write(os);
    }
  }

};

#endif
#endif

