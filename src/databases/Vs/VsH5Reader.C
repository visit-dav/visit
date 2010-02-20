#include <hdf5.h>
#include <visit-hdf5.h>
#include <InvalidFilesException.h>

#if HDF5_VERSION_GE(1,8,1)
/**
 *
 * @file        VsH5Reader.cpp
 *
 * @brief       Implementation reader of data for VSH5 schema
 *
 * @version $Id: VsH5Reader.C 496 2009-08-05 22:57:22Z mdurant $
 *
 * Copyright &copy; 2007-2008, Tech-X Corporation
 * See LICENSE file for conditions of use.
 *
 */

#include <sstream>
#include <stdexcept>

#include <VsH5Reader.h>
#include <VsSchema.h>
#include <VsUtils.h>

VsH5Reader::VsH5Reader(const std::string& nm, std::ostream& dbgstrm, std::vector<int> strideSettings) :
debugStrmRef(dbgstrm) {

  debugStrmRef <<"VsH5Reader::VsH5Reader(" <<nm <<") entering." <<std::endl;

  debugStrmRef <<"VsH5Reader::VsH5Reader(" <<nm <<") handling stride settings." <<std::endl;
  stride = strideSettings;
  useStride = false;
  for (unsigned int i = 0; i < stride.size(); i++) {
    if (stride[i] != 1)
    useStride = true;
  }

  fileId = H5Fopen(nm.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  if (fileId<0) {
    std::string msg = "VsH5Reader::VsH5Reader(): HDF5 error opening the file '"
    + nm + "'.";
    debugStrmRef << msg << std::endl;
    throw std::invalid_argument(msg.c_str());
  }

  // Read metadata
  filter = new VsFilter(fileId, dbgstrm);
  h5meta = filter->getH5Meta();
  
  // Was this even a vizschema file?
  if (!h5meta->hasObjects()) {
    debugStrmRef <<"VsH5Reader::VsH5Reader(" <<nm <<") - file format not recognized." <<std::endl;
    delete (filter);
    filter = NULL;

    EXCEPTION1(InvalidFilesException, nm.c_str());
  }

  meta = new VsMeta();
  makeVsMeta();
  meta->write(debugStrmRef);
  debugStrmRef <<"VsH5Reader::VsH5Reader(" <<nm <<") exiting." << std::endl;
}

VsH5Reader::~VsH5Reader() {
  debugStrmRef <<"VsH5Reader::~VsH5Reader() entering." << std::endl;

  //close what we know about
  if (filter != NULL) {
    delete filter;
    filter = NULL;
  }

  if (meta != NULL) {
    delete meta;
    meta = NULL;
  }

  // Check if anything is still open
  int cnt = H5Fget_obj_count(fileId, H5F_OBJ_ALL);
  if (cnt) {
    debugStrmRef << "File still has " << cnt << " open objects:";
    hid_t* objIds = new hid_t[cnt];
    int numObjs = H5Fget_obj_ids(fileId, H5F_OBJ_ALL, cnt, objIds);
    char objName[1024];
    for (int i = 0; i < numObjs; ++i) {
      hid_t anobj = objIds[i];
      H5I_type_t ot = H5Iget_type(anobj);
      herr_t status = H5Iget_name(anobj, objName, 1024);
      debugStrmRef << " type = " << ot << ", name = " << objName;
      if (i < numObjs - 1) debugStrmRef << ";";
      else debugStrmRef << ".";
    }
    debugStrmRef << std::endl;
    delete [] objIds;
  }
  debugStrmRef << "Strong close should take care of this." << std::endl;

  if (fileId) {
    H5Fclose(fileId);
    fileId = 0;
  }

  debugStrmRef <<"VsH5Reader::~VsH5Reader() exiting." <<std::endl;
}

void VsH5Reader::makeVsVars () {
  debugStrmRef <<"VsH5Reader::MakeVsVars() entering." <<std::endl;
  std::map<std::string, VsGMeta*>::const_iterator i;
  for (i = h5meta->vsVars.begin(); i != h5meta->vsVars.end(); ++i) {
    VsGMeta* gm = i->second;
    std::vector<VsAMeta*>::const_iterator k;
    for (k = gm->attribs.begin(); k != gm->attribs.end(); ++k) {
      if (!((*k)->name == VsSchema::typeAtt)) {
        std::string s;
        herr_t err = getAttributeHelper((*k)->aid, &s, 0, 0);
        if (err == 0) {
          std::pair<std::string, std::string> el((*k)->name, s);
          meta->vsVars.insert(el);
        }
      }
    }
  }
  debugStrmRef <<"VsH5Reader::MakeVsVars() exiting." <<std::endl;
}

void VsH5Reader::makeVsMeta() {
  debugStrmRef <<"VsH5Reader::MakeVsMeta() entering." <<std::endl;
  // Meshes MUST come first, because the MD Vars procedures have to be able to reference the newly created md meshes
  // Insert meshes
  debugStrmRef <<"VsH5Reader::makeVsMeta() - building G-Meshes" <<std::endl;
  for (std::map<std::string, VsGMeta*>::const_iterator h5mesh = h5meta->gMeshes.begin(); h5mesh != h5meta->gMeshes.end(); ++h5mesh) {
    VsMeshMeta* vm = new VsMeshMeta();
    std::string name = h5mesh->second->getFullName();
    vm->path = h5mesh->second->path;
    vm->name = h5mesh->second->name;
    try {
      makeGMeshMeta(h5mesh->second, *vm);

      //if this mesh declares itself to be part of an MD mesh, put it there
      // otherwise, add it to the regular list
      std::string mdMeshName = vm->getStringAttribute(VsSchema::mdAtt);
      if (!mdMeshName.empty()) {
        VsMDMeshMeta* mdMeta = meta->getMDMesh(mdMeshName);
        if (mdMeta == NULL) {
          //create md mesh, add to list in "meta", save in mdMeta
          //note that the MD mesh inherits the "kind" of the first block
          //all other blocks must match this kind
          mdMeta = new VsMDMeshMeta(mdMeshName, vm->kind, vm->numSpatialDims, vm->indexOrder);
          std::pair<std::string, VsMDMeshMeta*> el(mdMeshName, mdMeta);
          meta->mdMeshes.insert(el);
        }
        std::string errorStr = mdMeta->addBlock(vm);
        if (!errorStr.empty()) {
          debugStrmRef <<"VsH5Reader::makeVsMeta() - failed to add block to MD Var: " <<errorStr <<std::endl;
          debugStrmRef <<"VsH5Reader::makeVsMeta() - will add to regular vars instead." <<std::endl;
          std::pair<std::string, VsMeshMeta*> el(name, vm);
          meta->meshes.insert(el);
        }
      } else { //not part of an MD mesh, add to regular mesh list
        std::pair<std::string, VsMeshMeta*> el(name, vm);
        meta->meshes.insert(el);
      }
    }
    catch (std::invalid_argument& ex) {
      debugStrmRef << "VsH5Reader::makeVsMeta(): exception: " <<
      ex.what() << ", mesh '" << name << "' not inserted." << std::endl;
      delete vm;
    }
  }/*


   debugStrmRef <<"VsH5Reader::MakeVsMeta() entering." <<std::endl;
   // Meshes MUST come first, because the MD Vars procedures have to be able to reference the newly created md meshes
   // Insert meshes
   debugStrmRef <<"VsH5Reader::makeVsMeta() - building G-Meshes" <<std::endl;
   std::map<std::string, VsGMeta*>::const_iterator j = h5meta->gMeshes.begin();
   while (j != h5meta->gMeshes.end()) {
   VsMeshMeta* vm = new VsMeshMeta();
   std::string name = j->second->getFullName();
   debugStrmRef <<"VsH5Reader::makeVsMeta() - building mesh " <<name <<std::endl;

   vm->path = j->second->path;
   vm->name = j->second->name;
   try {
   makeGMeshMeta(j->second, *vm);

   //if this mesh declares itself to be part of an MD mesh, put it there
   // otherwise, add it to the regular list
   debugStrmRef <<"VsH5Reader::makeVsMeta() - checking for MD membership" <<std::endl;
   std::string mdMeshName = vm->getStringAttribute(VsSchema::mdAtt);
   if (!mdMeshName.empty()) {
   VsMDMeshMeta* mdMeta = meta->getMDMesh(mdMeshName);
   if (mdMeta == NULL) {
   //create md mesh, add to list in "meta", save in mdMeta
   //note that the MD mesh inherits the "kind" of the first block
   //all other blocks must match this kind
   mdMeta = new VsMDMeshMeta(mdMeshName, vm->kind, vm->numSpatialDims, vm->indexOrder);
   std::pair<std::string, VsMDMeshMeta*> el(mdMeshName, mdMeta);
   meta->mdMeshes.insert(el);
   }
   std::string errorStr = mdMeta->addBlock(vm);
   if (!errorStr.empty()) {
   debugStrmRef <<"VsH5Reader::makeVsMeta() - failed to add block to MD Var: " <<errorStr <<std::endl;
   debugStrmRef <<"VsH5Reader::makeVsMeta() - will add to regular vars instead." <<std::endl;
   std::pair<std::string, VsMeshMeta*> el(name, vm);
   meta->meshes.insert(el);
   }
   } else {     //not part of an MD mesh, add to regular mesh list
   if (vm == NULL) {
   debugStrmRef <<"VsH5Reader::makeVsMeta() - vm is nuLL" <<std::endl;
   }
   if (meta == NULL) {
   debugStrmRef <<"VsH5Reader::makeVsMeta() - meta is nuLL" <<std::endl;
   }
   debugStrmRef <<"VsH5Reader::makeVsMeta() - building pair for mesh " <<name <<", vm is " <<vm <<std::endl;
   std::pair<std::string, VsMeshMeta*> el(name, vm);
   debugStrmRef <<"VsH5Reader::makeVsMeta() - inserting mesh " <<name <<" into mesh list. " <<std::endl;
   meta->meshes.insert(el);
   debugStrmRef <<"VsH5Reader::makeVsMeta() - done inserting mesh " <<name <<" into mesh list. " <<std::endl;
   }
   }
   catch (std::invalid_argument& ex) {
   debugStrmRef << "VsH5Reader::makeVsMeta(): exception: " <<
   ex.what() << ", mesh '" << name << "' not inserted." << std::endl;
   delete vm;
   }
   }
   */
  debugStrmRef <<"VsH5Reader::makeVsMeta() - building D-Meshes" <<std::endl;
  for (std::map<std::string, VsDMeta*>::const_iterator l = h5meta->dMeshes.begin(); l != h5meta->dMeshes.end(); ++l) {
    VsMeshMeta* vm = new VsMeshMeta();
    VsDMeta* meshMeta = l->second;
    std::string fullName = meshMeta->getFullName();
    vm->path = meshMeta->path;
    vm->name = meshMeta->name;
    try {
      makeDMeshMeta(meshMeta, *vm);
      //if this mesh declares itself to be part of an MD mesh, put it there
      // otherwise, add it to the regular list
      std::string mdMeshName = vm->getStringAttribute(VsSchema::mdAtt);

      if (!mdMeshName.empty()) {
        VsMDMeshMeta* mdMeta = meta->getMDMesh(mdMeshName);
        if (mdMeta == NULL) {
          //create md mesh, add to list in "meta", save in mdMeta
          //note that the MD mesh inherits the "kind" of the first block
          //all other blocks must match this kind
          mdMeta = new VsMDMeshMeta(mdMeshName, vm->kind, vm->numSpatialDims, vm->indexOrder);
          std::pair<std::string, VsMDMeshMeta*> el(mdMeshName, mdMeta);
          debugStrmRef <<"VsH5Reader::MakeVsMeta() - Created new MD Mesh named " << mdMeshName <<std::endl;
          meta->mdMeshes.insert(el);
        }
        std::string errorStr = mdMeta->addBlock(vm);
        if (!errorStr.empty()) {
          debugStrmRef <<"VsH5Reader::makeVsMeta() - failed to add block to MD Var: " <<errorStr <<std::endl;
          debugStrmRef <<"VsH5Reader::makeVsMeta() - will add to regular vars instead." <<std::endl;
          std::pair<std::string, VsMeshMeta*> el(fullName, vm);
          meta->meshes.insert(el);
        }
      } else {
        std::pair<std::string, VsMeshMeta*> el(fullName, vm);
        meta->meshes.insert(el);
      }
    }
    catch (std::invalid_argument& ex) {
      debugStrmRef << "VsH5Reader::makeVsMeta(): exception: " <<
      ex.what() << ", mesh '" << fullName<< "' not inserted." << std::endl;
      delete vm;
    }
  }

  // Get all derived variables (vsVars)
  debugStrmRef <<"VsH5Reader::makeVsMeta() - building Vs Vars" <<std::endl;
  makeVsVars();

  // Insert vars
  debugStrmRef <<"VsH5Reader::makeVsMeta() - building Vars" <<std::endl;
  for (std::map<std::string, VsDMeta*>::const_iterator i = h5meta->vars.begin();
      i != h5meta->vars.end(); ++i) {
    VsVariableMeta* vm = new VsVariableMeta();
    std::string name = i->second->getFullName();
    try {
      makeVariableMeta(i->second, *vm);

      //if this var declares itself to be part of an MD var, put it there
      // otherwise, add it to the regular list
      std::string mdVarName = vm->getStringAttribute (VsSchema::mdAtt);
      if (!mdVarName.empty()) {
        debugStrmRef <<"VsH5Reader::makeVsMeta() - Variable " + vm->getFullName() + " is declared to be part of MD variable " + mdVarName <<std::endl;
        VsMDVariableMeta* mdMeta = meta->getMDVar(mdVarName);
        if (mdMeta == NULL) {
          debugStrmRef <<"VsH5Reader::makeVsMeta() - MD variable " + mdVarName + " not found, creating." <<std::endl;
          //create md var, add to list in "meta", save in mdMeta

          //the md var lives on an md mesh, so cross-reference to get the md mesh
          const VsMDMeshMeta* mdMeshMeta = getMDParentForMesh(vm->getMesh());

          //did we find the appropriate md mesh?
          if (mdMeshMeta == NULL) {
            debugStrmRef <<"VsH5Reader::makeVsMeta() - WARNING: variable " + name + " is declared as part of MD Var " + mdVarName + " but referenced mesh " + vm->getMesh() + " is not part of an MD mesh." <<std::endl;
          } else {
            //note that the MD variable inherits index order and centering of first block
            mdMeta = new VsMDVariableMeta(mdMeshMeta->getFullName(), vm->centering, vm->indexOrder);

            debugStrmRef <<"VsH5Reader::MakeVsMeta() - Created new MD Variable named " << mdVarName <<" on mesh " <<mdMeshMeta->getFullName() <<std::endl;
            std::pair<std::string, VsMDVariableMeta*> el(mdVarName, mdMeta);
            meta->mdVars.insert(el);
          }
        }

        //see if we succeeded in creating an md var or not
        if (mdMeta == NULL) {
          std::pair<std::string, VsVariableMeta*> el(name, vm);
          meta->vars.insert(el);
        } else {
          std::string errorStr = mdMeta->addBlock(vm);
          if (!errorStr.empty()) {
            debugStrmRef <<"VsH5Reader::makeVsMeta() - failed to add block to MD Var: " <<errorStr <<std::endl;
            debugStrmRef <<"VsH5Reader::makeVsMeta() - will add to regular vars instead." <<std::endl;
            std::pair<std::string, VsVariableMeta*> el(name, vm);
            meta->vars.insert(el);
          }
        }
      } else {
        std::pair<std::string, VsVariableMeta*> el(name, vm);
        meta->vars.insert(el);
      }

    }
    catch (std::invalid_argument& ex) {
      debugStrmRef << "VsH5Reader::makeVsMeta(): exception: " <<
      ex.what() << ", variable '" << name << "' not inserted." << std::endl;
      delete vm;
    }
  }

  // Insert vars with mesh
  debugStrmRef <<"VsH5Reader::makeVsMeta() - building Var with Mesh" <<std::endl;
  for (std::map<std::string, VsDMeta*>::const_iterator k = h5meta->varsWithMesh.begin();
      k != h5meta->varsWithMesh.end(); ++k) {
    VsVariableWithMeshMeta* vm = new VsVariableWithMeshMeta();
    std::string name = k->second->getFullName();
    try {
      makeVariableWithMeshMeta(k->second, *vm);
      std::pair<std::string, VsVariableWithMeshMeta*> el(name, vm);
      meta->varsWithMesh.insert(el);
    }
    catch (std::invalid_argument& ex) {
      debugStrmRef << "VsH5Reader::makeVsMeta(...): exception: " <<
      ex.what() << ", variableWithMesh '" << name << "' not inserted." <<
      std::endl;
      delete vm;
    }
  }

  debugStrmRef <<"VsH5Reader::MakeVsMeta() exiting." <<std::endl;
}

herr_t VsH5Reader::makeVariableMeta(VsDMeta* dm, VsVariableMeta& vm) const {
  debugStrmRef <<"VsH5Reader::makeVariableMeta() entering." <<std::endl;
  // Set dataset
  vm.dataset = dm;
  vm.path = dm->path;

  // Read the mesh name and comp major/minor
  std::vector<VsAMeta*>::const_iterator k;
  for (k = dm->attribs.begin(); k != dm->attribs.end(); ++k) {
    if ((*k)->name == VsSchema::meshAtt) {
      std::string s;
      herr_t err = getAttributeHelper((*k)->aid, &s, 0, 0);
      if (err < 0 ) goto atterr;
      //is this a fully qualified mesh name or not?
      s = makeCanonicalName(dm->path, s);
      //if (s.find_first_of('/') != 0) {
      //  //not fully qualified, so prepend our path
      //  s = dm->path + "/" + s;
      // }

      vm.mesh = s;
    }
    if ((*k)->name == VsSchema::indexOrderAtt) {
      herr_t err = getAttributeHelper((*k)->aid, &vm.indexOrder, 0, 0);
      if (err < 0 ) {
        debugStrmRef << "VsH5Reader::makeVariableMeta(): error getting "
        "optional attribute '" << (*k)->name << "'." << std::endl;
      }
    }
    if ((*k)->name == VsSchema::centeringAtt) {
      herr_t err = getAttributeHelper((*k)->aid, &vm.centering, 0, 0);
      if (err < 0 ) {
        debugStrmRef << "VsH5Reader::makeVariableMeta(): error getting "
        "optional attribute '" << (*k)->name << "'." << std::endl;
      }
    }

  }

  //look for user-specified labels for components
  for (k = dm->attribs.begin(); k != dm->attribs.end(); ++k) {
    if ((*k)->name == VsSchema::labelsAtt) {
      //labels is a comma-delimited list of strings
      size_t len = H5Aget_storage_size((*k)->aid);
      char* v = new char[len];
      hid_t atype = H5Aget_type((*k)->aid);
      herr_t err = H5Aread((*k)->aid, atype, v);
      if (err < 0) {
        std::string msg = "VsH5Reader::makeVariableMeta(...): '";
        msg += dm->name;
        msg += "' does not have attribute '";
        msg += (*k)->name;
        msg += "'.";
        debugStrmRef << msg << std::endl;
      }

      //the string "v" contains the list of labels, but they're separated with ","
      //so we need to be smart here...
      vm.labelNames.clear();
      std::string nameBuffer;
      nameBuffer.clear();
      unsigned int labelIndex = 0;
      for (unsigned int i = 0; i < len; i++) {
        char currentChar = v[i];
        if (currentChar == ',') {
          //we have finished the current name
          //so we bump the label index
          //and start a new, blank name
          labelIndex++;
          vm.labelNames.push_back(nameBuffer);
          nameBuffer.clear();
        } else if (currentChar != ' ') {
          //ignore leading spaces
          //all other characters get appended to the label
          nameBuffer.push_back(currentChar);
        }
      }

      //is the last name still in the buffer?
      if (!nameBuffer.empty()) {
        vm.labelNames.push_back(nameBuffer);
        nameBuffer.clear();
      }

      //we construct a vector containing the proper labels
      debugStrmRef <<"VsH5Reader::makeVariableMeta() - found user-specified label names." <<std::endl;
      for (unsigned int i = 0; i < vm.labelNames.size(); i++) {
        debugStrmRef <<"VsH5Reader::makeVariableMeta() - label[" <<i <<"] = " <<vm.labelNames[i] <<std::endl;
      }
    }
  }

  debugStrmRef <<"VsH5Reader::makeVariableMeta() returning 0." <<std::endl;
  return 0;

  atterr:
  debugStrmRef << "VsH5Reader::makeVariableMeta(): error getting "
  "attribute '" << (*k)->name << "'." << std::endl;
  debugStrmRef <<"VsH5Reader::makeVariableMeta() returning -1 (error)." <<std::endl;
  return -1;

}

herr_t VsH5Reader::makeVariableWithMeshMeta(VsDMeta* dm,
    VsVariableWithMeshMeta& vm) const {
  debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta() entering." <<std::endl;

  // Set dataset
  vm.dataset = dm;

  //We have two ways of specifying information for varWithMesh:
  // 1. VsSpatialIndices indicates which columns contain spatial data (synergia style)
  // 2. Spatial information is in the first "vsNumSpatialDims" columns (regular style)

  //we start with synergia style, and drop through to regular style on any errors
  std::vector<VsAMeta*>::const_iterator k;

  bool numDimsSet = false;
  for (k = dm->attribs.begin(); k != dm->attribs.end(); ++k) {
    if ((*k)->name == VsSchema::spatialIndicesAtt) {
      debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta(...): found spatialIndices, trying synergia style" <<std::endl;
      std::vector<int> in;
      debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta(...): Getting " <<VsSchema::spatialIndicesAtt <<std::endl;
      herr_t err = getAttributeHelper((*k)->aid, 0, &in, 0);
      if (err < 0) {
        debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta() - did not find attribute " <<VsSchema::spatialIndicesAtt <<std::endl;
      } else {
        debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta(...): Got attribute, no error" <<std::endl;
        numDimsSet = true;
        vm.spatialIndices = in;
        debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta(...): Saved attribute in vm" <<std::endl;
      }
    }

    //NOTE: We load indexOrder regardless of whether we're in synergia style or not
    if ((*k)->name == VsSchema::indexOrderAtt) {
      debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta(...): found indexOrder" <<std::endl;
      herr_t err = getAttributeHelper((*k)->aid, &(vm.indexOrder), 0, 0);
      if (err < 0) {
        debugStrmRef << "VsH5Reader::makeVariableWithMeshMeta(...): '" <<
        dm->name << "' error getting optional attribute '" <<
        (*k)->name << "'." << std::endl;
      }
    }

  }

  if (!numDimsSet) {
    //we tried and failed to load spatialIndices synergia style
    //so we drop back into the default - get the number of spatial dimensions
    //We then construct a spatialIndices array containing [0, 1, ..., numSpatialDims - 1]
    //So for a 3-d mesh we have [0, 1, 2]
    debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta(...): did not find spatialIndices, trying regular style" <<std::endl;
    // Read num of spatial dims and comp major/minor
    for (k = dm->attribs.begin(); k != dm->attribs.end(); ++k) {
      if ((*k)->name == VsSchema::numSpatialDimsAtt) {
        std::vector<int> in;
        herr_t err = getAttributeHelper((*k)->aid, 0, &in, 0);
        if (err < 0) {
          std::string msg = "VsH5Reader::makeVariableWithMeshMeta(...): '";
          msg += dm->name;
          msg += "' does not have attribute '";
          msg += (*k)->name;
          msg += "'.";
          debugStrmRef << msg << std::endl;
          debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta() - Throwing exception." <<std::endl;
          throw std::invalid_argument(msg.c_str());
        }

        int numSpatialDims = in[0];

        //we construct a vector containing the proper spatialIndices
        vm.spatialIndices.resize(numSpatialDims);
        for (int i = 0; i < numSpatialDims; i++) {
          vm.spatialIndices[i] = i;
        }

        numDimsSet = true;
        debugStrmRef << "VsH5Reader::makeVariableWithMeshMeta(...): "
        "numSpatialDims = " << vm.getNumSpatialDims() << "." << std::endl;
      }
    }
  }

  // Check that all set as needed
  if (!numDimsSet) {
    std::string msg = "VsH5Reader::makeVariableWithMeshMeta(...): "
    "variableWithMesh '";
    msg += dm->name;
    msg += "' did not have the attribute '";
    msg += VsSchema::numSpatialDimsAtt;
    msg += "'.";
    debugStrmRef << msg << std::endl;
    debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta() - Throwing exception." <<std::endl;
    throw std::invalid_argument(msg.c_str());
  }

  //look for user-specified labels for components
  for (k = dm->attribs.begin(); k != dm->attribs.end(); ++k) {
    if ((*k)->name == VsSchema::labelsAtt) {

      //labels is a comma-delimited list of strings
      size_t len = H5Aget_storage_size((*k)->aid);
      char* v = new char[len];
      hid_t atype = H5Aget_type((*k)->aid);
      herr_t err = H5Aread((*k)->aid, atype, v);
      if (err < 0) {
        std::string msg = "VsH5Reader::makeVariableWithMeshMeta(...): '";
        msg += dm->name;
        msg += "' does not have attribute '";
        msg += (*k)->name;
        msg += "'.";
        debugStrmRef << msg << std::endl;
      }

      //the string "v" contains the list of labels, but they're separated with ","
      //so we need to be smart here...
      vm.labelNames.clear();
      std::string nameBuffer;
      nameBuffer.clear();
      unsigned int labelIndex = 0;
      for (unsigned int i = 0; i < len; i++) {
        char currentChar = v[i];
        if (currentChar == ',') {
          //we have finished the current name
          //so we bump the label index
          //and start a new, blank name
          labelIndex++;
          vm.labelNames.push_back(nameBuffer);
          nameBuffer.clear();
        } else if (currentChar != ' ') {
          //ignore leading spaces
          //all other characters get appended to the label
          nameBuffer.push_back(currentChar);
        }
      }

      //is the last name still in the buffer?
      if (!nameBuffer.empty()) {
        vm.labelNames.push_back(nameBuffer);
        nameBuffer.clear();
      }

      //we construct a vector containing the proper labels
      debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta() - found user-specified label names." <<std::endl;
      for (unsigned int i = 0; i < vm.labelNames.size(); i++) {
        debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta() - label[" <<i <<"] = " <<vm.labelNames[i] <<std::endl;
      }
    }
  }
  debugStrmRef <<"VsH5Reader::makeVariableWithMeshMeta() - Returning 0." <<std::endl;
  return 0;
}
/*
 herr_t VsH5Reader::getMeshKind(const std::string& name, std::string& kind) const {
 // Look through group meshes
 //  std::map<std::string, VsGMeta*>::const_iterator it = h5meta->gMeshes.find(name);
 debugStrmRef <<"VsH5Reader::getMeshKind(" <<name <<", " <<kind <<") - Entering." <<std::endl;
 const VsGMeta* gm = h5meta->getGMesh(name);
 if (gm != 0) {
 std::vector<VsAMeta*>::const_iterator k;
 for (k = gm->attribs.begin(); k != gm->attribs.end(); ++k) {
 if ((*k)->name == VsSchema::kindAtt) {
 herr_t err = getAttribute((*k)->aid, &kind, 0, 0);
 if (err < 0) {
 debugStrmRef <<"VsH5Reader::getMeshKind() - getAttribute returned error:" <<err <<std::endl;
 }
 }
 }
 debugStrmRef <<"VsH5Reader::getMeshKind(" <<name <<", " <<kind <<") - Returning 0." <<std::endl;
 return 0;
 }

 // Look through dataset meshes
 //std::map<std::string, VsDMeta*>::const_iterator it1 = h5meta->dMeshes.find(name);
 const VsDMeta* dm = h5meta->getDMesh(name);
 if (dm != 0) {
 std::vector<VsAMeta*>::const_iterator k;
 for (k = dm->attribs.begin(); k != dm->attribs.end(); ++k) {
 if ((*k)->name == VsSchema::kindAtt) {
 herr_t err = getAttribute((*k)->aid, &kind, 0, 0);
 if (err < 0) {
 debugStrmRef <<"VsH5Reader::getMeshKind() - getAttribute returned error:" <<err <<std::endl;
 }
 }
 }
 debugStrmRef <<"VsH5Reader::getMeshKind(" <<name <<", " <<kind <<") - Returning 0." <<std::endl;
 return 0;
 }
 debugStrmRef <<"VsH5Reader::getMeshKind(" <<name <<", " <<kind <<") - Returning -1." <<std::endl;
 return -1;
 }
 */
void VsH5Reader::getMeshesNames(std::vector<std::string>& names) const {
  debugStrmRef <<"VsH5Reader::getMeshesNames() - Entering" <<std::endl;
  std::map<std::string, VsMeshMeta*>::const_iterator it;
  for (it = meta->meshes.begin(); it != meta->meshes.end(); ++it)
  names.push_back(it->first);
  debugStrmRef <<"VsH5Reader::getMeshesNames() - Returning" <<std::endl;
}

void VsH5Reader::getMDMeshNames(std::vector<std::string>& names) const {
  debugStrmRef <<"VsH5Reader::getMDMeshesNames() - Entering" <<std::endl;
  std::map<std::string, VsMDMeshMeta*>::const_iterator it;
  for (it = meta->mdMeshes.begin(); it != meta->mdMeshes.end(); ++it)
  names.push_back(it->first);
  debugStrmRef <<"VsH5Reader::getMDMeshesNames() - Returning" <<std::endl;
}

void VsH5Reader::getVarsNames(std::vector<std::string>& names) const {
  debugStrmRef <<"VsH5Reader::getVarsNames() - Entering" <<std::endl;
  std::map<std::string, VsVariableMeta*>::const_iterator it;
  for (it = meta->vars.begin(); it != meta->vars.end(); ++it)
  names.push_back(it->first);
  debugStrmRef <<"VsH5Reader::getVarsNames() - Returning" <<std::endl;
}

void VsH5Reader::getMDVarsNames(std::vector<std::string>& names) const {
  debugStrmRef <<"VsH5Reader::getMDVarsNames() - Entering" <<std::endl;
  std::map<std::string, VsMDVariableMeta*>::const_iterator it;
  for (it = meta->mdVars.begin(); it != meta->mdVars.end(); ++it)
  names.push_back(it->first);
  debugStrmRef <<"VsH5Reader::getMDVarsNames() - Returning" <<std::endl;
}

void VsH5Reader::getVarsWithMeshNames(std::vector<std::string>& names) const {
  debugStrmRef <<"VsH5Reader::getVarsWithMeshNames() - Entering" <<std::endl;
  std::map<std::string, VsVariableWithMeshMeta*>::const_iterator it;
  for (it = meta->varsWithMesh.begin(); it != meta->varsWithMesh.end(); ++it)
  names.push_back(it->first);
  debugStrmRef <<"VsH5Reader::getVarsWithMeshNames() - Returning" <<std::endl;
}

const std::map<std::string, std::string>& VsH5Reader::getVsVars() const {
  debugStrmRef <<"VsH5Reader::getVsVars() - Entering/Returning" <<std::endl;
  return meta->vsVars;
}

herr_t VsH5Reader::makeGMeshMeta(VsGMeta* gm, VsMeshMeta& mm) const {
  debugStrmRef <<"VsH5Reader::makeGMeshMeta() - Entering" <<std::endl;

  //start by adding all datasets to MeshMeta
  std::vector<VsDMeta*>::const_iterator y;
  for (y = gm->datasets.begin(); y != gm->datasets.end(); ++y) {
    std::pair<std::string, VsDMeta*> el((*y)->name, *y);
    mm.dComps.insert(el);
  }

  //next add all attributes to MeshMeta
  std::vector<VsAMeta*>::const_iterator z;
  for (z = gm->attribs.begin(); z != gm->attribs.end(); ++z) {
    std::pair<std::string, VsAMeta*> el((*z)->name, *z);
    mm.aComps.insert(el);
  }

  // Find Kind
  mm.kind = mm.getStringAttribute(VsSchema::kindAtt);
  ///TODO: check for legal values for vsKind

  // Find IndexOrder
  mm.indexOrder = mm.getStringAttribute(VsSchema::indexOrderAtt);
  if (mm.indexOrder.empty()) {
    debugStrmRef <<"VsH5Reader::makeGMeshMeta() - did not find value for " <<VsSchema::indexOrderAtt <<std::endl;
    debugStrmRef <<"VsH5Reader::makeGMeshMeta() - defaulting to " <<VsSchema::compMinorCKey <<std::endl;
    mm.indexOrder = VsSchema::compMinorCKey;
  }
  else if ((mm.indexOrder == VsSchema::compMajorCKey) ||
      (mm.indexOrder == VsSchema::compMajorFKey)) {
    //Legal but not 100% implemented
    std::string msg = "VsH5Reader::makeGMeshMeta() - Attribute ";
    msg += VsSchema::indexOrderAtt;
    msg += " has value ";
    msg += mm.indexOrder;
    msg += " which may not be completely implemented.";
    debugStrmRef << msg <<std::endl;
  } else if ((mm.indexOrder == VsSchema::compMinorCKey) ||
      (mm.indexOrder == VsSchema::compMinorFKey)) {
    //Legal and implemented
  }
  else {
    debugStrmRef << "VsH5Reader::makeGMeshMeta() - Found unknown value for attribute \"" << VsSchema::indexOrderAtt
    << "\" - will use default value of \"" << VsSchema::compMinorCKey <<"\"" << std::endl;
    mm.indexOrder = VsSchema::compMinorCKey;
  }

  // Determine the spatial dimensionality
  if (mm.isUniformMesh()) {
    std::map<std::string, VsAMeta*>::const_iterator it = mm.aComps.find(VsSchema::Uniform::comp2);
    if (it != mm.aComps.end()) {
      mm.numSpatialDims = it->second->getLength();
    }
    else {
      std::string msg = "VsH5Reader::makeGMeshMeta(...): uniform cartesian "
      "mesh does not have attribute named '";
      msg += VsSchema::Uniform::comp2;
      msg += "'.";
      debugStrmRef << msg << std::endl;
      debugStrmRef <<"VsH5Reader::makeGMeshMeta() - Throwing exception." <<std::endl;
      throw std::invalid_argument(msg.c_str());
    }
  }
  else if (mm.isUnstructuredMesh()) {
    //at this point, we should have our acomps and dcomps loaded
    //so the accessors should be able to find our data
    VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)&mm;
    const VsDMeta* datasetMeta = unstructuredMesh->getPointsDataset();
    if (datasetMeta != NULL) {
      mm.numSpatialDims = datasetMeta->dims[1];
      mm.numnodes = datasetMeta->dims[0];
    }
    else {
      //it's possible that we have multiple points datasets
      debugStrmRef <<"unstructuredMesh->path = " <<unstructuredMesh->path <<std::endl;
      debugStrmRef <<"unstructuredMesh->vsPoints0 = " <<unstructuredMesh->getPointsDatasetName(0) <<std::endl;
      debugStrmRef <<"unstructuredMesh->vsPoints1 = " <<unstructuredMesh->getPointsDatasetName(1) <<std::endl;
      debugStrmRef <<"unstructuredMesh->vsPoints2 = " <<unstructuredMesh->getPointsDatasetName(2) <<std::endl;

      std::string datasetName = unstructuredMesh->getPointsDatasetName(0);
      const VsDMeta* points0 = h5meta->getDataset(datasetName);
      if (points0 == NULL) {
        debugStrmRef <<"ERROR - VsH5Reader::makeGMeshMeta() - Unable to load points data." << std::endl;
        debugStrmRef <<"ERROR - VsH5Reader::makeGMeshMeta() - Throwing Exception." <<std::endl;
        std::string msg = "VsH5Reader::makeGMeshMeta(...): unstructured mesh";
        msg += gm->name;
        msg += " does not contain point data. ";
        debugStrmRef <<msg <<std::endl;
        throw std::invalid_argument(msg.c_str());
      }

      //spatial dimensionality = number of vspoints datasets
      unstructuredMesh->numSpatialDims = 1;
      datasetName = unstructuredMesh->getPointsDatasetName(1);
      const VsDMeta* points1 = h5meta->getDataset(datasetName);
      if (points1 != NULL) {
        unstructuredMesh->numSpatialDims++;
        datasetName = unstructuredMesh->getPointsDatasetName(2);
        const VsDMeta* points2 = h5meta->getDataset(datasetName);
        if (points2 != NULL) {
          unstructuredMesh->numSpatialDims++;
        }
      }

      unstructuredMesh->numnodes = points0->dims[0];
    }
  }
  /*
   else if (mm.kind == VsSchema::Rectilinear::key) {
   //Rectilinear meshes are defined by a series of arrays
   //Each array represents one spatial dimension
   //i.e. numSpatialDims = number of arrays

   std::map<std::string, VsAMeta*>::iterator it;
   //look for the name of the third axis
   it = mm.aComps.find(VsSchema::Rectilinear::axis2Key);
   string axisName = "axis2"; //the default name
   if (it != mm.aComps.end()) {
   //found a non-default user-specified name
   axisName = it->second->name;
   }

   //now look for the actual dataSet for this axis
   std::map<std::string, VsDMeta*>::iterator it2;
   it2 = mm.dComps.find(axisName);
   if (it2 != mm.dComps.end()) {
   mm.numSpatialDims = 3;
   }
   else {
   mm.numSpatialDims = 2;
   }
   }*/
  else {
    std::string msg = "VsH5Reader::makeGMeshMeta(...): for mesh of kind '";
    msg += mm.kind;
    msg += "', numSpatialDims unknown.";
    debugStrmRef << msg << std::endl;
    debugStrmRef <<"VsH5Reader::makeGMeshMeta() - Throwing Exception" <<std::endl;
    throw std::invalid_argument(msg.c_str());
  }

  // Done!
  debugStrmRef <<"VsH5Reader::makeGMeshMeta() - Returning 0 (normal result)" <<std::endl;
  return 0;
}

herr_t VsH5Reader::makeDMeshMeta(VsDMeta* dm, VsMeshMeta& mm) const {
  debugStrmRef <<"VsH5Reader::makeDMeshMeta() - Entering" <<std::endl;
  std::pair<std::string, VsDMeta*> el(dm->getFullName(), dm);
  mm.dComps.insert(el);
  mm.numSpatialDims = dm->dims[dm->dims.size()-1];

  // Find kind
  std::vector<VsAMeta*>::const_iterator k;
  for (k = dm->attribs.begin(); k != dm->attribs.end(); ++k) {
    if ((*k)->name == VsSchema::kindAtt) {
      getAttributeHelper((*k)->aid, &(mm.kind), 0, 0);
    }
  }

  // Go through attributes and add to components
  std::vector<VsAMeta*>::const_iterator z1;
  for (z1 = dm->attribs.begin(); z1 != dm->attribs.end(); ++z1) {
    if ( ((*z1)->name != VsSchema::kindAtt) &&
        ((*z1)->name != VsSchema::typeAtt) ) {
      std::pair<std::string, VsAMeta*> el((*z1)->name, *z1);
      mm.aComps.insert(el);
    }
  }
  debugStrmRef <<"VsH5Reader::makeGMeshMeta() - Returning 0 (normal)" <<std::endl;
  return 0;
}

herr_t VsH5Reader::getVariable(const std::string& name, void* data) const {
  //std::map<std::string, VsVariableMeta*>::const_iterator it = meta->vars.find(name);
  debugStrmRef <<"VsH5Reader::getVariable(" <<name <<", data) - Entering" <<std::endl;
  const VsVariableMeta* var = getVariableMeta(name);//meta->getVar(name);
  if (var == 0) {
    debugStrmRef << "VsH5Reader::getVariable(" <<name <<"): Unable to find variable " <<name << "'." << std::endl;
    debugStrmRef << "VsH5Reader::getVariable(" <<name <<"): Returning -1 (error)." << std::endl;
    return -1;
  }

  hid_t id = var->dataset->iid;
  herr_t err = 0;
  if (!useStride) {
    err = H5Dread(id, var->dataset->type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  } else {
    ///TODO:: Improve error handling
    debugStrmRef << "VsH5Reader::getVariable(" <<name <<"): Loading partial data set." << std::endl;
    //do some fancy H5S hyperslab selection stuff here, then load partial data set
    //id is the dataset
    hid_t dataspace = H5Dget_space(id);
    int rank = H5Sget_simple_extent_ndims (dataspace);
    std::vector<hsize_t> dims_out(rank);
    H5Sget_simple_extent_dims (dataspace, &dims_out[0], NULL);
    if (rank == 2) {
      debugStrmRef <<"Dimensions: " <<(unsigned long)(dims_out[0]) <<" x " <<(unsigned long)(dims_out[1]) <<std::endl;
    } else if (rank == 3) {
      debugStrmRef <<"Dimensions: " <<(unsigned long)(dims_out[0]) <<" x " <<(unsigned long)(dims_out[1])
      <<" x " <<(unsigned long)(dims_out[2]) <<std::endl;
    }

    //count is how many elements to choose in each direction
    std::vector<hsize_t> count(rank);
    const VsMeshMeta* meshMeta = getMeshMeta(var->mesh);
    int addBefore = 0;
    int addAfter = 0;
    bool uniformMesh = false;
    bool structuredMesh = false;
    if (meshMeta) {
      if (meshMeta->isUniformMesh())
      uniformMesh = true;
      else if (meshMeta->isStructuredMesh())
      structuredMesh = true;
    } else {
      debugStrmRef <<"VsH5Reader::getVariable(...): " <<"Unable to load mesh metadata with name : " <<var->mesh <<std::endl;
      debugStrmRef <<"VsH5Reader::getVariable(...): " <<"Assuming uniform mesh, but who knows?" <<std::endl;
    }

    if (uniformMesh) {
      if (var->isZonal()) {
        debugStrmRef <<"VsH5Reader::getVariable(...): " <<"Zonal on uniform = no change" <<std::endl;
        adjustSize_hsize_t(&count[0], rank, stride, addBefore, addAfter);
      } else {
        //nodal
        debugStrmRef <<"VsH5Reader::getVariable(...): " <<"Nodal on uniform = -1/+1" <<std::endl;
        addBefore = -1;
        addAfter = 1;
        adjustSize_hsize_t(&count[0], rank, stride, addBefore, addAfter);
      }
    } else if (structuredMesh) {
      if (var->isZonal()) {
        debugStrmRef <<"VsH5Reader::getVariable(...): " <<"Zonal on structured = +1/-1" <<std::endl;
        std::vector<int> meshDims;
        getMeshDims(var->mesh, &meshDims);
        for (int i = 0; i < rank; i++)
        {
          debugStrmRef <<"VsH5Reader::getVariable(...): " <<"About to override size " <<count[i] <<" with size from mesh - 1: " <<(meshDims[i] - 1) <<std::endl;
          count[i] = meshDims[i] - 1;
        }

      } else {
        //nodal
        debugStrmRef <<"VsH5Reader::getVariable(...): " <<"Nodal on structured" <<std::endl;
        std::vector<int> meshDims;
        getMeshDims(var->mesh, &meshDims);
        for (int i = 0; i < rank; i++)
        {
          debugStrmRef <<"VsH5Reader::getVariable(...): " <<"About to override size " <<count[i] <<" with size from mesh: " <<(meshDims[i]) <<std::endl;
          count[i] = meshDims[i];
        }
      }
    }

    /*
     * Define hyperslab in the dataset.
     */
    //offset is the starting location
    std::vector<hsize_t> offset(rank);
    //stride is the stride in each direction
    std::vector<hsize_t> strideArray(rank);

    debugStrmRef <<"About to select variable hyperslab with size: ";
    for (int i = 0; i < rank; i++) {
      offset[i] = 0;
      strideArray[i] = stride[i];
      debugStrmRef <<count[i] <<" x ";
    }
    debugStrmRef <<std::endl;

    herr_t status = H5Sselect_hyperslab (dataspace, H5S_SELECT_SET, &offset[0], &strideArray[0],
        &count[0], NULL);

    /*
     * Define the memory dataspace.
     */
    std::vector<hsize_t> dimsm(rank);
    for (int i = 0; i < rank; i++) {
      dimsm[i] = count[i];
    }
    hid_t memspace = H5Screate_simple (rank, &dimsm[0], NULL);

    /*
     * Define memory hyperslab.
     */
    std::vector<hsize_t> offset_out(rank);
    std::vector<hsize_t> count_out(rank);
    for (int i = 0; i < rank; i++) {
      offset_out[i] = 0;
      count_out[i] = count[i];
    }

    status = H5Sselect_hyperslab (memspace, H5S_SELECT_SET, &offset_out[0], NULL,
        &count_out[0], NULL);

    /*
     * Read data from hyperslab in the file into the hyperslab in
     * memory and display.
     */
    status = H5Dread (id, var->dataset->type, memspace, dataspace,
        H5P_DEFAULT, data);

    //    H5Dclose (dataset);
    H5Sclose (dataspace);
    H5Sclose (memspace);
  }

  if (err != 0) {
    debugStrmRef << "VsH5Reader::getVariable(...): error " << err <<
    " in reading variable '" << name << "'." << std::endl;
  }
  debugStrmRef << "VsH5Reader::getVariable(" <<name <<"): Returning " <<err <<"." << std::endl;
  return err;
}

bool VsH5Reader::isVariable(const std::string& name) const {
  //std::map<std::string, VsVariableMeta*>::const_iterator it = meta->vars.find(name);
  //if (it != meta->vars.end()) return true;
  //else return false;
  debugStrmRef << "VsH5Reader::isVariable(" <<name <<"): Entering/Returning." << std::endl;
  if (meta->getVar(name) != 0)
  return true;

  if (meta->getMDVar(name) != 0)
  return true;

  if (findSubordinateMDVar(name) != 0)
  return true;

  return false;
}

herr_t VsH5Reader::getVariableComponent(const std::string& name, size_t indx, void* data) {
  debugStrmRef << "VsH5Reader::getVariableComponent(" <<name <<", " <<indx <<"): Entering." << std::endl;

  herr_t err = 0;
  const VsVariableMeta* meta = getVariableMeta(name);
  if (!meta) {
    debugStrmRef << "VsH5Reader::getVariableComponent(): error: var " << name <<
    " has no metadata." << std::endl;
    debugStrmRef << "VsH5Reader::getVariableComponent(): Returning 1 (error)." << std::endl;
    return 1;
  }
  std::vector<int> dims = meta->getDims();
  ///do we need -1 because we're in a component variable situation here?
  // answer - No, because we need to actually reference the component index
  size_t rank = dims.size();
  debugStrmRef << "VsH5Reader::getVariableComponent(...): " << name <<
  " has rank, " << rank << "." << std::endl;

  std::vector<hsize_t> count(rank);
  std::vector<hsize_t> start(rank);
  hid_t dataspace = H5Dget_space(meta->dataset->iid);

  if (meta->isCompMajor()) {
    for (size_t i = 1; i<rank; ++i) {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[0] = 1;
    start[0] = indx;

  }
  else { //compMinor
    for (size_t i = 0; i<(rank-1); ++i) {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[rank-1] = 1;
    start[rank-1] = indx;
  }

  //this gets complicated because it depends on the kind of variable (zonal vs nodal)
  // and the kind of the mesh (uniform vs structured)
  // We must also be careful to stay away if the mesh is NOT uniform or structured
  if (useStride) {
    const VsMeshMeta* meshMeta = getMeshMeta(meta->mesh);
    if (!meshMeta) {
      debugStrmRef <<"ERROR VsH5Reader::getVariableComponent(...): Unable to get mesh metadata for name: " <<meta->mesh <<std::endl;
      debugStrmRef <<"ERROR VsH5Reader::getVariableComponent(...): returning -1" <<std::endl;
      return -1;
    }
    int addBefore = 0;
    int addAfter = 0;

    if (meshMeta->isUniformMesh()) {
      if (meta->isZonal()) {
        debugStrmRef <<"VsH5Reader::getVariableComponent(...): " <<"Zonal on uniform = no change" <<std::endl;
        adjustSize_hsize_t(&count[0], rank - 1, stride, addBefore, addAfter);
      } else {
        //nodal
        debugStrmRef <<"VsH5Reader::getVariableComponent(...): " <<"Nodal on uniform = -1/+1" <<std::endl;
        addBefore = -1;
        addAfter = 1;
        adjustSize_hsize_t(&count[0], rank - 1, stride, addBefore, addAfter);
      }
    } else if (meshMeta->isStructuredMesh()) {
      if (meta->isZonal()) {
        debugStrmRef <<"VsH5Reader::getVariableComponent(...): " <<"Zonal on structured = +1/-1" <<std::endl;
        std::vector<int> meshDims;
        getMeshDims(meta->mesh, &meshDims);
        for (unsigned int i = 0; i < rank - 1; i++)
        {
          debugStrmRef <<"VsH5Reader::getVariableComponent(...): " <<"About to override size " <<count[i] <<" with size from mesh - 1: " <<(meshDims[i] - 1) <<std::endl;
          count[i] = meshDims[i] - 1;
        }

      } else {
        //nodal
        debugStrmRef <<"VsH5Reader::getVariableComponent(...): " <<"Nodal on structured" <<std::endl;
        std::vector<int> meshDims;
        getMeshDims(meta->mesh, &meshDims);
        for (unsigned int i = 0; i < rank - 1; i++)
        {
          debugStrmRef <<"VsH5Reader::getVariableComponent(...): " <<"About to override size " <<count[i] <<" with size from mesh: " <<(meshDims[i]) <<std::endl;
          count[i] = meshDims[i];
        }
      }
    }
  }

  debugStrmRef << "VsH5Reader::getVariableComponent(...): start =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << start[i];
  debugStrmRef << std::endl;
  debugStrmRef << "VsH5Reader::getVariableComponent(...): count =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << count[i];
  debugStrmRef << std::endl;

  std::vector<hsize_t> strideArray(rank);
  for (unsigned int i = 0; i < rank; i++) {
    if (i < stride.size())
    strideArray[i] = (hsize_t)stride[i];
    else strideArray[i] = 1;
  }

  // Select data
  err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], &strideArray[0],
      &count[0], NULL);
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, &count[0], NULL);
  // Read data
  err = H5Dread(meta->dataset->iid, meta->getType(), memspace, dataspace,
      H5P_DEFAULT, data);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getVariableComponent(...): error " << err <<
    " in reading variable '" << name << "'." << std::endl;
  }
  err = H5Sclose(memspace);
  err = H5Sclose(dataspace);
  debugStrmRef << "VsH5Reader::getVariableComponent(): Returning " <<err <<"." << std::endl;
  return err;
}

void* VsH5Reader::getVariableComponent(const std::string& name, size_t indx,
    size_t partnumber, size_t numparts, size_t* splitDims)
{
  debugStrmRef << "VsH5Reader::getVariableComponent(" <<name <<", " <<indx
  <<", " <<partnumber <<", " <<numparts <<", splitDims): Entering." << std::endl;

  if (partnumber >= numparts) {
    debugStrmRef <<"VsH5Reader::getVariableComponent() - Variable has " <<numparts
    <<"parts but we were asked for part number #" <<partnumber <<std::endl;
    debugStrmRef <<"VsH5Reader::getVariableComponent() - returning NULL." <<std::endl;
    return NULL;
  }

  const VsVariableMeta* meta = getVariableMeta(name);
  if (!meta)
  {
    debugStrmRef << "VsH5Reader::getVariableComponent(): error: var " <<
    name << " has no metadata." << std::endl;
    debugStrmRef << "VsH5Reader::getVariableComponent(): Returning 0." << std::endl;
    return 0;
  }

  std::vector<int> dims = meta->getDims();
  size_t rank = dims.size();
  debugStrmRef << "VsH5Reader::getVariableComponent(...): " << name <<
  " has rank " << rank << "." << std::endl;

  std::vector<hsize_t> count(rank);
  std::vector<hsize_t> start(rank);
  hid_t dataspace = H5Dget_space(meta->dataset->iid);

  if (meta->isCompMajor())
  {
    for (size_t i = 1; i<rank; ++i)
    {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[0] = 1;
    start[0] = indx;

  }
  else
  { //compMinor
    for (size_t i = 0; i<(rank-1); ++i)
    {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[rank-1] = 1;
    start[rank-1] = indx;
  }

  // Diagnostic output
  debugStrmRef << "VsH5Reader::getVariableComponent() start =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << start[i];
  debugStrmRef << std::endl;
  debugStrmRef << "VsH5Reader::getVariableComponent() count =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << count[i];
  debugStrmRef << std::endl;

  if (numparts > 1)
  {
    // Determine along which axis to split
    // NOTE: We assume that all spatial extents are larger than 1 so that we will
    // never split along the "component direction"
    size_t splitAxis = 0;
    size_t largestCount = count[splitAxis];

    for (size_t currAxis = 1; currAxis < rank; ++currAxis)
    {
      if (count[currAxis] > largestCount)
      {
        splitAxis = currAxis;
        largestCount = count[currAxis];
      }
    }

    // Split along axis
    size_t numCellsAlongSplitAxis = count[splitAxis] - 1;
    if (numCellsAlongSplitAxis)
    {
      size_t numCellsPerPart = numCellsAlongSplitAxis / numparts;
      size_t numPartsWithAdditionalCell = numCellsAlongSplitAxis % numparts;

      if (partnumber < numPartsWithAdditionalCell)
      {
        start[splitAxis] = partnumber * (numCellsPerPart + 1);
        count[splitAxis] = (numCellsPerPart + 1) + 1;
      }
      else
      {
        start[splitAxis] = numPartsWithAdditionalCell * (numCellsPerPart + 1) +
        (partnumber - numPartsWithAdditionalCell) * numCellsPerPart;
        count[splitAxis] = numCellsPerPart + 1;
      }
    }
  }

  if (splitDims)
  {
    if (meta->isCompMajor())
    {
      for (size_t i = 0; i<(rank-1); ++i)
      {
        splitDims[i] = count[i+1];
      }
    }
    else
    { //compMinor
      for (size_t i = 0; i<(rank-1); ++i)
      {
        splitDims[i] = count[i];
      }
    }
  }

  // Select data
  herr_t err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], NULL,
      &count[0], NULL);
  if (err < 0)
  {
    debugStrmRef << "VsH5Reader::getVariableComponent(): error " << err <<
    " selecting hyperslab for variable '" << name << "'." << std::endl;
    debugStrmRef << "VsH5Reader::getVariableComponent(): Returning 0." << std::endl;
    return 0;
  }

  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, &count[0], NULL);

  // Allocate memory
  hid_t type = meta->getType();
  int numElems = 1;
  for (size_t i = 0; i < rank; ++i) numElems *= count[i];
  void* data = new unsigned char[numElems*H5Tget_size(type)];

  // Read data
  err = H5Dread(meta->dataset->iid, type, memspace, dataspace,
      H5P_DEFAULT, data);
  if (err < 0)
  {
    debugStrmRef << "VsH5Reader::getVariableComponent(...): error " << err <<
    " in reading variable '" << name << "'." << std::endl;
    delete [] static_cast<unsigned char*>(data);
    debugStrmRef << "VsH5Reader::getVariableComponent(): Returning 0." << std::endl;
    return 0;
  }

  // Cleanup
  H5Sclose(memspace);
  H5Sclose(dataspace);

  // Return
  debugStrmRef << "VsH5Reader::getVariableComponent(): Returning data." << std::endl;
  return data;
}

herr_t VsH5Reader::getDatasetMeshComponent(const std::string& name,
    const VsMeshMeta& mmeta, void* data) const {
  debugStrmRef << "VsH5Reader::getDatasetMeshComponent(" <<name <<", mmeta, data) - Entering." << std::endl;

  VsDMeta* dataSet = mmeta.getDataset(name);
  if (dataSet == NULL) {
    debugStrmRef << "VsH5Reader::getDatasetMeshComponent(...): "
    << "error getting mesh component '" << name << "'." << std::endl;
    debugStrmRef << "VsH5Reader::getDatasetMeshComponent(" <<name <<", mmeta, data) - Returning -1." << std::endl;
    return -1;
  }
  debugStrmRef << "VsH5Reader::getDatasetMeshComponent(...): getting '" <<
  name << "' from dataset " << dataSet->name << "." << std::endl;
  if (H5Tequal(dataSet->type, H5T_NATIVE_INT)) {
    debugStrmRef << "VsH5Reader::getDatasetMeshComponent(...): type is "
    "H5T_NATIVE_INT." << std::endl;
  }
  else if (H5Tequal(dataSet->type, H5T_NATIVE_FLOAT)) {
    debugStrmRef << "VsH5Reader::getDatasetMeshComponent(...): type is "
    "H5T_NATIVE_FLOAT." << std::endl;
  }
  else if (H5Tequal(dataSet->type, H5T_NATIVE_DOUBLE)) {
    debugStrmRef << "VsH5Reader::getDatasetMeshComponent(...): type is "
    "H5T_NATIVE_DOUBLE." << std::endl;
  }

  herr_t err;
  if (!useStride) {
    err = H5Dread(dataSet->iid, dataSet->type, H5S_ALL, H5S_ALL,
        H5P_DEFAULT, data);
    if (err < 0) {
      debugStrmRef << "VsH5Reader::getDatasetMeshComponent(...): error " << err <<
      " in reading mesh component '" << name << "'." << std::endl;
    }

    //debug
    /*
     debugStrmRef <<"Dumping data: " <<std::endl;
     for (int i = 0; i < 10; i++) {
     if (H5Tequal(dataSet->type, H5T_NATIVE_DOUBLE)) {
     debugStrmRef <<"data[" <<i <<"] = " <<((double*)data)[i] <<std::endl;}
     else if (H5Tequal(dataSet->type, H5T_NATIVE_FLOAT)) {
     debugStrmRef <<"data[" <<i <<"] = " <<((float*)data)[i] <<std::endl;}
     else if (H5Tequal(dataSet->type, H5T_NATIVE_INT)) {
     debugStrmRef <<"data[" <<i <<"] = " <<((int*)data)[i] <<std::endl;}
     }
     debugStrmRef <<"Finished dumping data. " <<std::endl;
     */
    //end debug

  } else {
    std::vector<int> dims;
    getMeshDims(name, &dims);
    hsize_t rank = dims.size();

    debugStrmRef <<"VsH5Reader::getDatasetMeshComponent() - about to set up arguments." <<std::endl;
    std::vector<hsize_t> count(rank);
    std::vector<hsize_t> start(rank);
    std::vector<hsize_t> strideArray(rank);
    for (unsigned int i = 0; i < rank; i++) {
      if (i < rank - 1)
      strideArray[i] = (hsize_t)stride[i];
      else strideArray[i] = 1;

      start[i] = 0;
      count[i] = dims[i];

      debugStrmRef <<"For i = " <<i <<", start = " <<start[i] <<", count = " <<count[i] <<", and strideArray = " <<strideArray[i] <<std::endl;
    }
    hid_t dataspace = H5Dget_space(dataSet->iid);

    // Select data
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], &strideArray[0],
        &count[0], NULL);
    debugStrmRef <<"After selecting the hyperslab, err is " <<err <<std::endl;
    // Create memory space for the data
    hid_t memspace = H5Screate_simple(rank, &count[0], NULL);
    // Read data
    err = H5Dread(dataSet->iid, dataSet->type, memspace, dataspace,
        H5P_DEFAULT, data);
    if (err < 0) {
      debugStrmRef << "VsH5Reader::getDatasetMeshComponent(...): error " << err <<
      " in reading variable '" << name << "'." << std::endl;
    }
    err = H5Sclose(memspace);
    err = H5Sclose(dataspace);
  }

  debugStrmRef << "VsH5Reader::getDatasetMeshComponent(" <<name <<", mmeta, data) - Returning " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getSplitMeshData(std::string points0, std::string points1, std::string points2,
    const VsMeshMeta& mmeta, void* data) const {
  debugStrmRef << "VsH5Reader::getSplitMeshData(" <<points0 <<", " <<points1 <<", " <<points2 <<") - Entering." << std::endl;

  std::map<std::string, VsDMeta*>::const_iterator it = h5meta->datasets.find(points0);
  if (it == h5meta->datasets.end()) {
    debugStrmRef << "VsH5Reader::getSplitMeshData(...): "
    << "error getting mesh component '" << points0 << "'." << std::endl;
    debugStrmRef << "VsH5Reader::getSplitMeshData(" <<points0 <<", mmeta, data) - Returning -1." << std::endl;
    return -1;
  }

  herr_t err;
  if (!useStride) {
    std::vector<int> dims;
    getDims(it->second->iid, true, dims);
    int numPoints;
    //all data gets loaded as though it has 3 dimensions
    //if only 2-d, the third dimension is left blank
    int dimensions = 3;
    if (mmeta.isCompMajor()) {
      numPoints = dims[1];
      dimensions = 3;
    } else {
      numPoints = dims[0];
      dimensions = 3;
    }

    // Create memory space for the data
    debugStrmRef << "VsH5Reader::getSplitMeshData(): making call to H5Screate_simple: " <<std::endl;
    hsize_t memSpaceSize[1];
    memSpaceSize[0] = dimensions * numPoints;
    hid_t memspace = H5Screate_simple(1, memSpaceSize, NULL);

    //select first hyperslab in memory space
    hsize_t stride[1];
    stride[0] = dimensions;
    hsize_t start[1];
    start[0] = 0;
    hsize_t count[1];
    count[0] = numPoints;
    H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);

    //read entire points0 into hyperslab
    err = H5Dread(it->second->iid, it->second->type, memspace, H5S_ALL, H5P_DEFAULT, data);
    if (err < 0) {
      debugStrmRef << "VsH5Reader::getSplitMeshData(...): error " << err <<
      " in reading mesh component vsPoints0." << std::endl;
      return err;
    }

    //get second dataset
    it = h5meta->datasets.find(points1);
    if (it == h5meta->datasets.end()) {
      debugStrmRef << "VsH5Reader::getSplitMeshData(...): "
      << "error getting mesh component 'points1'." << std::endl;
      debugStrmRef << "VsH5Reader::getSplitMeshData(" <<points0 <<", mmeta, data) - Returning -1." << std::endl;
      return -1;
    }

    //select second hyperslab in memory
    start[0] = 1;
    H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);

    //read entire points1 into hyperslab
    err = H5Dread(it->second->iid, it->second->type, memspace, H5S_ALL, H5P_DEFAULT, data);
    if (err < 0) {
      debugStrmRef << "VsH5Reader::getSplitMeshData(...): error " << err <<
      " in reading mesh component vsPoints1." << std::endl;
      return err;
    }

    //get third dataset (if it exists)
    it = h5meta->datasets.find(points2);
    if (it == h5meta->datasets.end()) {
      //although we didn't find data, we need to zero these points
      for (int i = 2; i < numPoints; i++) {
        ((float*)&data)[i] = 0;
      }
      err = H5Sclose(memspace);
      return err;
    }

    //select third hyperslab in memory
    start[0] = 2;
    H5Sselect_hyperslab(memspace, H5S_SELECT_SET, start, stride, count, NULL);

    //read entire points2 into hyperslab
    err = H5Dread(it->second->iid, it->second->type, memspace, H5S_ALL, H5P_DEFAULT, data);
    if (err < 0) {
      debugStrmRef << "VsH5Reader::getSplitMeshData(...): error " << err <<
      " in reading mesh component vsPoints1." << std::endl;
      return err;
    }

    err = H5Sclose(memspace);

    debugStrmRef <<"VsH5Reader::getSplitMeshData() - returning data." <<std::endl;
    return err;
  } else {
    debugStrmRef <<"VsH5Reader::getSplitMeshData() - ERROR - not implemented for useStride" <<std::endl;
    return -1;
  }

  debugStrmRef << "VsH5Reader::getSplitMeshData(" <<points0 <<", mmeta, data) - Returning " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getAttMeshComponent(const std::string& name,
    const VsMeshMeta& mmeta, void* data) const {
  debugStrmRef << "VsH5Reader::getAttMeshComponent(" <<name <<", mmeta, data) - Entering." << std::endl;
  VsAMeta* attribute = mmeta.getAttribute(name);
  if (attribute == NULL) {
    debugStrmRef << "VsH5Reader::getAttMeshComponent(" <<name <<", mmeta, data) - Unable to find attribute, returning -1." << std::endl;
    return -1;
  }

  herr_t err = H5Aread(attribute->aid, attribute->type, data);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getAttMeshComponent(...): error " << err <<
    " in reading attribute '" << name << "'." << std::endl;
  }
  debugStrmRef << "VsH5Reader::getDatasetMeshComponent(" <<name <<", mmeta, data) - Returning " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getVarWithMeshMesh(const VsVariableWithMeshMeta& meta,
    void* data) const {
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh(meta, data)" << std::endl;

  //Since we allow the spatial indices to be specified individually,
  // we load the position data in slices
  // old style: start at 0 and read "numSpatialDims" columns
  // new style: read a total of "numSpatialDims" columns, one at a time
  herr_t err = 0;
  std::vector<int> dims = meta.getDims();
  size_t rank = dims.size();
  //rank should always be 2 for VarWithMesh data
  if (rank != 2) {
    debugStrmRef <<"VsH5Reader::getMeshVarWithMesh(meta, data) - Rank must be 2 but is " <<rank <<"." <<std::endl;
    debugStrmRef <<"VsH5Reader::getMeshVarWithMesh(meta, data) - unable to load data." <<std::endl;
    return -1;
  }

  std::vector<hsize_t> memCount(rank); //tracks the size of the memory block we need to load the data
  std::vector<hsize_t> start(rank); //start position of each slice
  std::vector<hsize_t> sliceCount(rank); //tracks the size of each slice
  std::vector<hsize_t> strideCount(rank);
  hid_t dataspace = H5Dget_space(meta.dataset->iid);
  int numPoints = 0;
  if (meta.isCompMajor())
  numPoints = dims[rank-1];
  else numPoints = dims[0];
  int dimensions = meta.getNumSpatialDims();

  //adjust points for stride
  if (stride[0] != 1) {
    debugStrmRef <<"VsH5Reader::getMeshVarWithMesh(meta, data) -" <<"Filtering points based on stride.  Before = " <<numPoints <<std::endl;
    numPoints = numPoints / stride[0];
    debugStrmRef <<"VsH5Reader::getMeshVarWithMesh(meta, data) -" <<"Filtering points based on stride.  After = " <<numPoints <<std::endl;
  }

  size_t indexToChange = -1; //which index of the start array needs to be changed for each slice

  if (meta.isCompMajor()) {
    for (size_t i = 1; i<rank; ++i) {
      memCount[i] = numPoints;
      sliceCount[i] = numPoints;
      start[i] = 0;
    }
    indexToChange = 0;
    memCount[0] = meta.getNumSpatialDims();
    sliceCount[0] = 1; //the slice takes a single spatial dimension at a time
    start[0] = 0;
    strideCount[0] = 1;
    strideCount[1] = stride[0];
  }
  else { //compMinor
    for (size_t i = 0; i<rank-1; ++i) {
      memCount[i] = numPoints;
      sliceCount[i] = numPoints;
      start[i] = 0;
    }
    indexToChange = rank-1;
    memCount[rank-1] = meta.getNumSpatialDims();
    sliceCount[rank-1] = 1; //the slice takes a single spatial dimension at a time
    start[rank-1] = 0;
    strideCount[0] = stride[0];
    strideCount[1] = 1;
  }
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - start =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << start[i];
  debugStrmRef << std::endl;
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - memCount =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << memCount[i];
  debugStrmRef << std::endl;
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - sliceCount =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << sliceCount[i];
  debugStrmRef << std::endl;
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - stride =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << stride[i];
  debugStrmRef << std::endl;

  herr_t lastError = 0;

  // Select data
  // Note that we select each spatial index separately
  H5Sselect_none(dataspace);
  hsize_t numElements = H5Sget_select_npoints(dataspace);
  debugStrmRef <<" VsH5Reader::getMeshVarWithMesh(): - we have selected this many elements: " <<numElements <<std::endl;

  // Create memory space for the data
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): making call to H5Screate_simple: " <<std::endl;
  hsize_t memSpaceSize[1];
  memSpaceSize[0] = dimensions * numPoints;
  hid_t memspace = H5Screate_simple(1, memSpaceSize, NULL);

  for (unsigned int d = 0; d < meta.getNumSpatialDims(); d++) {
    start[indexToChange] = meta.getSpatialDim(d);
    debugStrmRef <<std::endl <<"Spatial Dim #" <<d <<std::endl;
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - start =";
    for (size_t i=0; i<rank; ++i) debugStrmRef << " " << start[i];
    debugStrmRef << std::endl;
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - sliceCount =";
    for (size_t i=0; i<rank; ++i) debugStrmRef << " " << sliceCount[i];
    debugStrmRef << std::endl;

    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], &strideCount[0], &sliceCount[0], NULL);
    hsize_t numElements = H5Sget_select_npoints(dataspace);
    debugStrmRef <<" VsH5Reader::getMeshVerWithMesh(): - we have selected this many elements in dataset: " <<numElements <<std::endl;
    if (err < 0) {
      debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): error in H5select_hyperslab: " <<err << std::endl;
      lastError = err;
    }

    //NOTE: This routine will work perfectly for C-order data
    // but is not tested and probably will not work for Fortran-order data
    //select the destination hyperslab in memory
    hsize_t memStart[1];
    hsize_t memStride[1];
    hsize_t memSliceCount[1];
    // We use stride to spread the data out into the correct places
    // starting location is all zeros, but we bump the location by 1 for each dimension
    // Thus first pass starts at 0 and we get x 0 0 x 0 0 x 0 0 (for dimensionality == 3)
    // Second pass starts at 1 and we get x y 0 x y 0 x y 0 (for dimensionality == 3)
    // Third pass starts at 2 and we get x y z x y z x y z (for dimensionality == 3)
    // Note that we're treating the memory block as a single long array
    // instead of as a multi-dimensional matrix
    memStride[0] = dimensions;
    memStart[0] = d;
    memSliceCount[0] = numPoints;

    debugStrmRef <<"memStride[0] = " <<memStride[0] <<std::endl;
    debugStrmRef <<"memSliceCount[0] = " <<memSliceCount[0] <<std::endl;
    debugStrmRef <<"memStart[0] = " <<memStart[0] <<std::endl;

    H5Sselect_hyperslab(memspace, H5S_SELECT_SET, memStart, memStride, memSliceCount, NULL);
    numElements = H5Sget_select_npoints(memspace);
    debugStrmRef <<" VsH5Reader::getMeshVerWithMesh(): - we have selected this many elements in memory: " <<numElements <<std::endl;

    // Read data
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): making call to H5Dread: " <<std::endl;
    err = H5Dread(meta.dataset->iid, meta.getType(), memspace, dataspace,
        H5P_DEFAULT, data);
    if (err < 0) {
      debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): error in H5Dread: " <<err << std::endl;
      lastError = err;
    }

  }

  // Read data
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): making call to H5Dread: " <<std::endl;
  err = H5Dread(meta.dataset->iid, meta.getType(), memspace, dataspace,
      H5P_DEFAULT, data);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): error in H5Dread: " <<err << std::endl;
    lastError = err;
  }

  debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): making call to H5Sclose: " <<std::endl;
  err = H5Sclose(memspace);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): error in H5Sclose(memspace): " <<err << std::endl;
    lastError = err;
  }

  err = H5Sclose(dataspace);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): error in H5Sclose(dataspace): " <<err << std::endl;
    lastError = err;
  }

  debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - Returning " <<lastError <<"." << std::endl;
  return lastError;
}

herr_t VsH5Reader::getVarWithMeshMesh(const VsVariableWithMeshMeta& meta,
    void* data, size_t partStart, size_t partCount) const {
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh()" << std::endl;

  std::vector<int> dims = meta.getDims();
  size_t rank = dims.size();
  if (rank != 2)
  {
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): don't know what to do if rank != 2" << std::endl;
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): Returning 1." << std::endl;
    return 1;
  }
  hsize_t count[2];
  hsize_t start[2];
  hid_t dataspace = H5Dget_space(meta.dataset->iid);

  if (meta.isCompMajor()) {
    count[1] = partCount;
    start[1] = partStart;
    count[0] = meta.getNumSpatialDims();
    start[0] = 0;
  }
  else {
    count[0] = partCount;
    start[0] = partStart;
    count[1] = meta.getNumSpatialDims();
    start[1] = 0;
  }

  //WORK HERE - MARC - to select hyperslabs in correct order for spatialDims
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - start =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << start[i];
  debugStrmRef << std::endl;
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh() - count =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << count[i];
  debugStrmRef << std::endl;

  // Select data
  herr_t err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL,
      count, NULL);
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, count, NULL);
  // Read data
  err = 10 * err + H5Dread(meta.dataset->iid, meta.getType(), memspace, dataspace,
      H5P_DEFAULT, data);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): read error " << std::endl;
  }
  err = 10 * err + H5Sclose(memspace);
  err = 10 * err + H5Sclose(dataspace);
  debugStrmRef << "VsH5Reader::getMeshVarWithMesh(): Returning cumulative error code: " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getVarWithMeshComponent(const std::string& name, size_t idx,
    void* data) const {
  debugStrmRef << "VsH5Reader::getVarWithMeshComponent(" <<name <<", " <<idx
  <<", data) " << std::endl;

  const VsVariableWithMeshMeta* meta = getVariableWithMeshMeta(name);
  if (!meta) {
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): error: " << name <<
    " has no metadata." << std::endl;
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): Returning 1." << std::endl;
    return 1;
  }
  std::vector<int> dims = meta->getDims();

  size_t rank = dims.size();
  std::vector<hsize_t> count(rank);
  std::vector<hsize_t> start(rank);
  std::vector<hsize_t> strideCount(rank);
  hid_t dataspace = H5Dget_space(meta->dataset->iid);

  if (useStride) {
    if (meta->isCompMajor()) {
      dims[1] = dims[1] / stride[0];
      strideCount[0] = 1;
      strideCount[1] = stride[0];
    } else {
      dims[0] = dims[0] / stride[0];
      strideCount[0] = stride[0];
      strideCount[1] = 1;
    }
  } else {
    strideCount[0] = 1;
    strideCount[1] = 1;
  }

  //assert(dims[dims.size()-1] >= (int)(idx));
  if ((int)idx > dims[dims.size()-1]) {
    debugStrmRef <<"VsH5Reader::getVarWithMeshComponent() - WARNING: failed assertion idx < dims[dims.size()-1]" <<std::endl;
    debugStrmRef <<"dims.size() is " <<dims.size() <<std::endl;
    debugStrmRef <<"dims[dims.size()-1] is " <<dims[dims.size() - 1] <<std::endl;
    debugStrmRef <<"meta->numSpatialDims is " <<meta->getNumSpatialDims() <<std::endl;
    debugStrmRef <<"idx is " <<idx <<std::endl;
    std::string msg = "VsH5Reader::VsH5Reader: index out of range.";
    debugStrmRef << msg << std::endl;
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): Throwing exception." << std::endl;
    throw std::out_of_range(msg.c_str());
    //     debugStrmRef <<"VsH5Reader::getVarWithMeshComponent() - returning NULL" <<std::endl;
    //    return NULL;
  }
  if (meta->isCompMajor()) {
    for (size_t i = 1; i<rank; ++i) {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[0] = 1;
    start[0] = idx;
  }
  else { //compMinor
    for (size_t i = 0; i<(rank-1); ++i) {
      count[i] = dims[i];
      start[i] = 0;
    }
    count[rank-1] = 1;
    start[rank-1] = idx;
  }

  debugStrmRef << "start =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << start[i];
  debugStrmRef << std::endl;
  debugStrmRef << "count =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << count[i];
  debugStrmRef << std::endl;
  debugStrmRef << "stride =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << strideCount[i];
  debugStrmRef << std::endl;

  // Select data
  herr_t err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, &start[0], &strideCount[0],
      &count[0], NULL);
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, &count[0], NULL);
  // Read data
  err = 10 * err + H5Dread(meta->dataset->iid, meta->getType(), memspace, dataspace,
      H5P_DEFAULT, data);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(...): error " << err <<
    " reading variable '" << name << "'." << std::endl;
  }
  err = 10 * err + H5Sclose(memspace);
  err = 10 * err + H5Sclose(dataspace);
  debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): Returning cumulative error: " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getVarWithMeshComponent(const std::string& name, size_t idx,
    void* data, size_t partStart, size_t partCount) const {
  debugStrmRef << "VsH5Reader::getVarWithMeshComponent(" <<name <<", " <<idx <<", data, "
  <<partStart <<", " <<partCount <<") - Entering." << std::endl;

  const VsVariableWithMeshMeta* meta = getVariableWithMeshMeta(name);
  if (!meta) {
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): error: " << name <<
    " has no metadata." << std::endl;
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): Returning 1." << std::endl;
    return 1;
  }
  std::vector<int> dims = meta->getDims();
  size_t rank = dims.size();
  if (rank != 2)
  {
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): don't know what to do if rank != 2" << std::endl;
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): Returning 1." << std::endl;
    return 1;
  }
  hsize_t count[2];
  hsize_t start[2];
  hid_t dataspace = H5Dget_space(meta->dataset->iid);

  // Assert used to be this:
  // assert(dims[dims.size()-1] >= (int)(meta->numSpatialDims+idx));
  // But we decided that idx can be between 0 and dims[dims.size() - 1]
  // And that we dont' need to take into account the spatial dims
  // Because those are allowable components.
  if ((int)idx > dims[dims.size()-1]) {
    debugStrmRef <<"VsH5Reader::getVarWithMeshComponent() - WARNING: failed assertion idx < dims[dims.size()-1]" <<std::endl;
    debugStrmRef <<"dims.size() is " <<dims.size() <<std::endl;
    debugStrmRef <<"dims[dims.size()-1] is " <<dims[dims.size() - 1] <<std::endl;
    debugStrmRef <<"meta->numSpatialDims is " <<meta->getNumSpatialDims() <<std::endl;
    debugStrmRef <<"idx is " <<idx <<std::endl;
    debugStrmRef <<"VsH5Reader::getVarWithMeshComponent() - returning NULL" <<std::endl;
    return NULL;
  }

  if (meta->isCompMajor()) {
    count[1] = partCount;
    start[1] = partStart;
    count[0] = 1;
    start[0] = meta->getNumSpatialDims()+idx;
  }
  else { //compMinor
    count[0] = partCount;
    start[0] = partStart;
    count[1] = 1;
    start[1] = meta->getNumSpatialDims()+idx;
  }

  debugStrmRef << "VsH5Reader::getVarWithMeshComponent() - start =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << start[i];
  debugStrmRef << std::endl;
  debugStrmRef << "VsH5Reader::getVarWithMeshComponent() - count =";
  for (size_t i=0; i<rank; ++i) debugStrmRef << " " << count[i];
  debugStrmRef << std::endl;

  // Select data
  herr_t err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL,
      count, NULL);
  // Create memory space for the data
  hid_t memspace = H5Screate_simple(rank, count, NULL);
  // Read data
  err = 10 * err + H5Dread(meta->dataset->iid, meta->getType(), memspace, dataspace,
      H5P_DEFAULT, data);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getVarWithMeshComponent(...): error " << err <<
    " reading variable '" << name << "'." << std::endl;
  }
  err = 10 * err + H5Sclose(memspace);
  err = 10 * err + H5Sclose(dataspace);
  debugStrmRef << "VsH5Reader::getVarWithMeshComponent(): Returning cumulative error: " <<err <<"." << std::endl;
  return err;
}

herr_t VsH5Reader::getVariableWithMesh(const std::string& name, void* data) const {
  const VsVariableWithMeshMeta* var = meta->getVarWithMesh(name);
  if (var == 0) {
    debugStrmRef << "VsH5Reader::getVariableWithMesh(...): unable to find "
    << "variable with mesh by name " << name << "'." << std::endl;
    return -1;
  }

  herr_t err = H5Dread(var->dataset->iid, var->dataset->type,
      H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
  if (err < 0) {
    debugStrmRef << "VsH5Reader::getVariableWithMesh(...): error " << err <<
    " in reading dataset '" << name << "'." << std::endl;
  }
  debugStrmRef << "VsH5Reader::getVariableWithMesh(): Returning " <<err <<"." << std::endl;
  return err;
}

const VsVariableMeta* VsH5Reader::getVariableMeta(const std::string& name) const {
  debugStrmRef << "VsH5Reader::getVariableMeta(" <<name <<"): Entering." << std::endl;

  const VsVariableMeta* answer = meta->getVar(name);
  if (!answer) {
    debugStrmRef << "VsH5Reader::getVariableMeta(" <<name <<"): Metadata not found in regular variable list, checking md." << std::endl;
    answer = findSubordinateMDVar(name);
  }

  if (!answer) {
    debugStrmRef << "VsH5Reader::getVariableMeta(" <<name <<"): Metadata not found in md variable list." << std::endl;
  }

  debugStrmRef << "VsH5Reader::getVariableMeta(" <<name <<"): Returning." << std::endl;
  return answer;
}

const VsMDVariableMeta* VsH5Reader::getMDVariableMeta(const std::string& name) const {
  debugStrmRef << "VsH5Reader::getMDVariableMeta(" <<name <<"): Entering/Returning." << std::endl;
  return meta->getMDVar(name);
}

const VsVariableWithMeshMeta* VsH5Reader::getVariableWithMeshMeta(
    const std::string& name) const {
  debugStrmRef << "VsH5Reader::getVariableWithMeshMeta(" <<name <<"): Entering/Returning." << std::endl;
  return meta->getVarWithMesh(name);
}

const VsMeshMeta* VsH5Reader::getMeshMeta(const std::string& name) const {
  debugStrmRef << "VsH5Reader::getMeshMeta(" <<name <<"): Entering." << std::endl;
  const VsMeshMeta* answer = meta->getMesh(name);
  if (!answer) {
    debugStrmRef << "VsH5Reader::getMeshMeta(" <<name <<"): Mesh not found, checking in subordinate md mesh list." << std::endl;
    answer = findSubordinateMDMesh(name);
  }

  if (!answer) {
    debugStrmRef << "VsH5Reader::getMeshMeta(" <<name <<"): Mesh not found." << std::endl;
  }

  debugStrmRef << "VsH5Reader::getMeshMeta(" <<name <<"): Returning." << std::endl;
  return answer;
}

const VsMDMeshMeta* VsH5Reader::getMDMeshMeta(const std::string& name) const {
  debugStrmRef << "VsH5Reader::getMDMeshMeta(" <<name <<"): Entering/Returning." << std::endl;
  return (const VsMDMeshMeta*)meta->getMDMesh(name);
}

int VsH5Reader::getDomainNumberForMesh(const std::string& name) const {
  debugStrmRef << "VsH5Reader::getDomainNumberForMesh(" <<name <<"): Entering." << std::endl;
  std::map<std::string, VsMDMeshMeta*> mdMeshList = meta->mdMeshes;
  for (std::map<std::string, VsMDMeshMeta*>::const_iterator it = mdMeshList.begin(); it != mdMeshList.end(); it++) {
    VsMDMeshMeta* meshMeta = it->second;
    for (unsigned int i = 0; i < meshMeta->blocks.size(); i++) {
      if (meshMeta->blocks[i]->getFullName() == name) {
        debugStrmRef << "VsH5Reader::getDomainNumberForMesh(" <<name <<"): Returning " <<i <<"." << std::endl;
        return i;
      }
    }
  }
  debugStrmRef << "VsH5Reader::getDomainNumberForMesh(" <<name <<"): Returning -1." << std::endl;
  return -1;
}

const VsMDMeshMeta* VsH5Reader::getMDParentForMesh(const std::string& name) const {
  debugStrmRef << "VsH5Reader::getMDParentForMesh(" <<name <<"): Entering." << std::endl;
  std::map<std::string, VsMDMeshMeta*> mdMeshList = meta->mdMeshes;
  for (std::map<std::string, VsMDMeshMeta*>::const_iterator it = mdMeshList.begin(); it != mdMeshList.end(); it++) {
    VsMDMeshMeta* meshMeta = it->second;
    for (unsigned int i = 0; i < meshMeta->blocks.size(); i++) {
      if (meshMeta->blocks[i]->getFullName() == name) {
        debugStrmRef << "VsH5Reader::getMDParentForMesh(" <<name <<"): Returning result." << std::endl;
        return meshMeta;
      }
    }
  }
  debugStrmRef << "VsH5Reader::getMDParentForMesh(" <<name <<"): Returning NULL." << std::endl;
  return NULL;
}

VsMeshMeta* VsH5Reader::findSubordinateMDMesh(const std::string& name) const {
  debugStrmRef << "VsH5Reader::findSubordinateMDMesh(" <<name <<"): Entering." << std::endl;
  std::map<std::string, VsMDMeshMeta*> mdMeshList = meta->mdMeshes;
  for (std::map<std::string, VsMDMeshMeta*>::const_iterator it = mdMeshList.begin(); it != mdMeshList.end(); it++) {
    VsMDMeshMeta* meshMeta = it->second;
    for (unsigned int i = 0; i < meshMeta->blocks.size(); i++) {
      if (meshMeta->blocks[i]->getFullName() == name) {
        debugStrmRef << "VsH5Reader::findSubordinateMDMesh(" <<name <<"): Returning result." << std::endl;
        return meshMeta->blocks[i];
      }
    }
  }
  debugStrmRef << "VsH5Reader::findSubordinateMDMesh(" <<name <<"): Returning NULL." << std::endl;
  return NULL;
}

const VsVariableMeta* VsH5Reader::findSubordinateMDVar(const std::string& name) const {
  debugStrmRef << "VsH5Reader::findSubordinateMDVar(" <<name <<"): Entering." << std::endl;
  std::map<std::string, VsMDVariableMeta*> mdVarList = meta->mdVars;
  for (std::map<std::string, VsMDVariableMeta*>::const_iterator it = mdVarList.begin(); it != mdVarList.end(); it++) {
    VsMDVariableMeta* varMeta = it->second;
    for (unsigned int i = 0; i < varMeta->blocks.size(); i++) {
      if (varMeta->blocks[i]->getFullName() == name) {
        debugStrmRef << "VsH5Reader::findSubordinateMDVar(" <<name <<"): Returning result." << std::endl;
        return varMeta->blocks[i];
      }
    }
  }
  debugStrmRef << "VsH5Reader::findSubordinateMDVar(" <<name <<"): Returning NULL." << std::endl;
  return NULL;
}

size_t VsH5Reader::getMDNumComps(const std::string& mdVarName) const {
  debugStrmRef << "VsH5Reader::getMDNumComps(" <<mdVarName <<"): Entering." << std::endl;
  // A bit more complicated for MD than regular vars
  // Could be done more efficiently, but this will do for now.
  // Look up all subordinate vars in the MD var
  // Get num components for each (should all be the same)
  // Return the smallest
  const VsMDVariableMeta* mdMeta = getMDVariableMeta(mdVarName);
  if (mdMeta == NULL) {
    debugStrmRef <<"VsH5Reader::getMDNumComps(" <<mdVarName <<") - unable to find metadata, returning 0." <<std::endl;
    return 0;
  }

  debugStrmRef <<"VsH5Reader::getMDNumComps(" <<mdVarName <<") - Looking through " <<mdMeta->getNumBlocks() <<"blocks." <<std::endl;
  int fewestComponents = -1;
  for (unsigned int i = 0; i < mdMeta->getNumBlocks(); i++) {
    VsVariableMeta* varMeta = mdMeta->getBlock(i);
    if (varMeta == NULL) {
      debugStrmRef <<"VsH5Reader::getMDNumComps(" <<mdVarName <<") - unable to find block #"
      <<i <<"?  Returning 0." <<std::endl;
      return 0;
    }
    int comps = getNumComps(varMeta->getFullName());
    debugStrmRef <<"VsH5Reader::getMDNumComps(" <<mdVarName <<") - Block " <<varMeta->getFullName() <<" has " <<comps <<" components." <<std::endl;

    //first time through the loop we just initialize the comparison variable
    if (fewestComponents == -1) {
      fewestComponents = comps;
    } else {
      //the rest of the times through the loop we do some comparisons and error checking
      if (comps < fewestComponents) {
        debugStrmRef <<"WARNING VsH5Reader::getMDNumComps(" <<mdVarName <<") - mismatch between number of components in subordinate blocks" <<std::endl;
        fewestComponents = comps;
      } else if (comps > fewestComponents) {
        debugStrmRef <<"WARNING VsH5Reader::getMDNumComps(" <<mdVarName <<") - mismatch between number of components in subordinate blocks" <<std::endl;
        //note that we do NOT adjust fewestComponents here
      }
    }

    debugStrmRef <<"VsH5Reader::getMDNumComps(" <<mdVarName <<") - Smallest number of components so far is " <<fewestComponents <<"." <<std::endl;
  }

  debugStrmRef <<"VsH5Reader::getMDNumComps(" <<mdVarName <<") - Returning " <<fewestComponents <<"." <<std::endl;
  return fewestComponents;
}

size_t VsH5Reader::getNumComps(const std::string& name) const {
  debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): Entering." << std::endl;

  // Find VsVariable with this name, the name of the mesh
  const VsVariableMeta* vm = getVariableMeta(name);

  //If we didn't find anything in the regular list,
  // look for it in the MD stuff
  if (!vm) {
    debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): Did not find var in regular list, checking MD." << std::endl;
    vm = findSubordinateMDVar(name);
  }

  //Still nothing?
  if (vm == NULL) {
    debugStrmRef <<"VsH5Reader::getNumComps(" <<name <<") - unable to find metadata for var.  Returning 0." <<std::endl;
    return 0;
  }
  std::vector<int> ddims = vm->dataset->dims;

  //we do not want to adjust the size of the variable array here
  // Because we're trying to determine how many components the variable has
  // and so we don't know if we would need to adjust ALL of the dimensions
  // or just all-but-one.
  //adjustSize_vector(&ddims, stride, debugStrmRef);

  //we need an absolute path in order to load the mesh
  std::string mname = makeCanonicalName(vm->mesh);
  std::vector<int> mdims;
  getMeshDims(mname, &mdims);

  // Debugging information
  debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): ";
  debugStrmRef << "For data, " << name << ", ddims =";
  for (size_t i=0; i<ddims.size(); ++i) debugStrmRef << " " << ddims[i];
  debugStrmRef << "." << std::endl;;
  debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"):";
  debugStrmRef << "For mesh, " << mname << ", mdims =";
  for (size_t i=0; i<mdims.size(); ++i) debugStrmRef << " " << mdims[i];
  debugStrmRef << "." << std::endl;

  // For unstructured meshes
  const VsMeshMeta* meshMeta = getMeshMeta(mname);

  size_t res = 0;
  if (meshMeta->isUnstructuredMesh()) {
    if ((mdims.size() != 1) && (mdims.size() != 2)) {
      debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): error: dataset '" << name <<
      "' does not have 1 or 2 dimensions.  Number of components unknown." <<
      std::endl;
      debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): Returning 0." << std::endl;
      return 0;
    }

    //is ddims a single component?
    if (ddims.size() == 1) {
      debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): This dataset is 1-d, so 1 component." <<std::endl;
      //yes, it's a simple list of values
      res = 1;
    }
    else {
      if (vm->isCompMajor()) {
        res = ddims.front();
      }
      else { //compMinor
        res = ddims.back();
      }
      debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): This dataset has " <<res <<" components." <<std::endl;
    }
    goto dimwarn;
  }

  // Compare dims
  if (meshMeta->isStructuredMesh()) {
    // Structure mesh has an extra dimension for the coordinate index
    if (mdims.size() > 1) mdims.resize(mdims.size() - 1);
  }
  if (mdims.size() == ddims.size()) {
    res = 1;
    goto dimwarn;
  }
  if ((ddims.size() == 1)
      || ((ddims.size() == 2) && (ddims[0] == 1))) {
    res = 1;
    goto dimwarn;
  }
  if (mdims.size() != (ddims.size() - 1) ) {
    debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): error - mesh '" << mname <<
    "' has dimensions of size, " << mdims.size() << ", while dataset '" <<
    name << "' has dimensions of size, " << ddims.size() <<
    ", which does not equal mdims.size() or mdims.size() + 1." << std::endl;
    debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): Returning 0." << std::endl;
    return 0;
  }

  // check that each of mesh sizes are correct (for compMinor data here)
  if (!vm->isCompMajor()) {
    /* JRC: Commenting out.  For center data, this is more complicated.
     For cell based data, need different comparison, with number of
     cells for unstructured, for number in each valid direction minus 1
     for structured.
     for (size_t i = 0; i < mdims.size(); ++i)
     if (ddims[i] != mdims[i]) {
     debugStrmRef << "VsH5Reader::getNumComps error - mesh's dimensions "
     "do not match dimensions of " << name << std::endl;
     return 0;
     }
     */
    res = ddims.back();
    goto dimwarn;
  }

  // Check that sizes are the same
  /* JRC: Commenting out.  See above.
   for (size_t i = 0; i < mdims.size(); ++i)
   if (ddims[i+1] != mdims[i]) {
   debugStrmRef << "VsH5Reader::getNumComps error - mesh's "
   "dimensions do not match dimensions of " << name << std::endl;
   return 0;
   }
   */
  res = ddims.front();

  dimwarn:
  debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): warning the size of the "
  "variable '" << name << "' not validated.  Future work." << std::endl;
  debugStrmRef << "VsH5Reader::getNumComps(" <<name <<"): Returning " <<res <<" components." << std::endl;
  return res;

}

size_t VsH5Reader::getMeshDims(const std::string& name, std::vector<int>* dims) const {
  debugStrmRef << "VsH5Reader::getMeshDims(" <<name <<", dims): Entering." << std::endl;

  // Default is zero
  size_t len = 0;

  const VsMeshMeta* meshMeta = getMeshMeta(name);
  if (meshMeta == NULL) {
    debugStrmRef <<"VsH5Reader::getMeshDims(" <<name <<") : Failed to load metadata for mesh." <<std::endl;
    debugStrmRef <<"VsH5Reader::getMeshDims(" <<name <<") : returning error -1" <<std::endl;
    return -1;
  }

  std::string kind = meshMeta->kind;

  // If this is uniform cartesian, read dims of totalNumCells attribute
  if (meshMeta->isUniformMesh()) {
    // find mesh in group meshes
    const VsGMeta* gm = h5meta->getGMesh(name);
    if (gm == 0) {
      debugStrmRef << "VsH5Reader::getMeshDims error: mesh " << name
      << " not found" << std::endl;
      debugStrmRef << "VsH5Reader::getMeshDims(" <<name <<", dims): Returning 0." << std::endl;
      return 0;
    }
    std::vector<VsAMeta*>::const_iterator k;
    for (k = gm->attribs.begin(); k != gm->attribs.end(); ++k) {
      if ((*k)->name == VsSchema::Uniform::comp2) {
        getAttributeHelper((*k)->aid, 0, dims, 0);
      }
    }

    if (useStride) {
      debugStrmRef <<"VsH5Reader::getMeshDims() - Adjusting size of mesh using stride." <<std::endl;
      for (unsigned int i = 0; i < dims->size(); i++) {
        int value = (*dims)[i];
        debugStrmRef <<"VsH5Reader::getMeshDims() - dims[" <<i <<"] = " <<value <<" stride[" <<i <<"] = " <<stride[i] <<std::endl;
        value = value / stride[i];

        //we round UP if there is a remainder
        // NOT FOR UNIFORM
        //if ((*dims)[i] % stride[i] != 0) {
        //  debugStrmRef <<"VsH5Reader::getMeshDims() - Added 1 because there was a remainder." <<std::endl;
        //      value ++;
        //}

        if (value < 1)
        value = 1;

        (*dims)[i] = value;
        debugStrmRef <<"VsH5Reader::getMeshDims() - dims[" <<i <<"] was adjusted to " <<(*dims)[i] <<std::endl;
      }
    }
  }

  // If this is unstructured, read the dims of points dataset
  if (meshMeta->isUnstructuredMesh()) {
    VsUnstructuredMesh* unstructuredMesh = (VsUnstructuredMesh*)meshMeta;
    std::string datasetName = unstructuredMesh->getPointsDatasetName();
    if (!datasetName.empty()) {
      const VsDMeta* datasetMeta = unstructuredMesh->getPointsDataset();
      if (datasetMeta != NULL) {
        *dims = datasetMeta->dims;
      } else {
        debugStrmRef <<"ERROR VsH5Reader::getMeshDims() - tried to find points data under name: " <<datasetName <<std::endl;
        debugStrmRef <<"ERROR VsH5Reader::getMeshDims() - but failed." <<std::endl;
        return -1;
      }
    }
    else {
      //it's possible that we have multiple points datasets
      std::string datasetName = unstructuredMesh->getPointsDatasetName(0);
      const VsDMeta* points0 = h5meta->getDataset(datasetName);
      if (points0 == NULL) {
        debugStrmRef <<"ERROR - VsH5Reader::getMeshDims() - Unable to load points data." << std::endl;
        debugStrmRef <<"ERROR - VsH5Reader::getMeshDims() - Throwing Exception." <<std::endl;
        std::string msg = "VsH5Reader::getMeshDims(...): unstructured mesh";
        msg += meshMeta->name;
        msg += " does not contain point data. ";
        debugStrmRef <<msg <<std::endl;
        throw std::invalid_argument(msg.c_str());
      }

      dims->resize(2);

      //points0 is only one of the datasets
      //so we need to adjust dims to match union of ALL datasets
      //spatial dimensionality = number of vspoints datasets
      if (unstructuredMesh->isCompMinor()) {
        (*dims)[0] = points0->dims[0];
        (*dims)[1] = unstructuredMesh->numSpatialDims;
      } else {
        (*dims)[0] = unstructuredMesh->numSpatialDims;
        (*dims)[1] = points0->dims[0];
      }

      debugStrmRef <<"VsH5Reader::getMeshDims() - dims[0] is " <<(*dims)[0] <<std::endl;
      debugStrmRef <<"VsH5Reader::getMeshDims() - dims[1] is " <<(*dims)[1] <<std::endl;

    }
  }

  // If this is structured, read dataset's dims
  if (meshMeta->isStructuredMesh()) {
    const VsDMeta* dm = h5meta->getDMesh(name);
    if (dm == 0) {
      debugStrmRef << "VsH5Reader::getMeshDims error: mesh " << name
      << " not found" << std::endl;
      debugStrmRef << "VsH5Reader::getMeshDims(" <<name <<", dims): Returning 0." << std::endl;
      return 0;
    }
    *dims = dm->dims;

    if (useStride) {
      debugStrmRef <<"VsH5Reader::getMeshDims() - Adjusting size of mesh using stride." <<std::endl;

      //we do dims - 1 because structured meshes store their coordinates in the last dimension
      // and we don't want to resize it
      for (unsigned int i = 0; i < dims->size() - 1; i++) {
        int value = (*dims)[i];
        debugStrmRef <<"VsH5Reader::getMeshDims() - dims[" <<i <<"] = " <<value <<" stride[" <<i <<"] = " <<stride[i] <<std::endl;

        value = value / stride[i];

        //we round UP if there is a remainder
        if ((*dims)[i] % stride[i] != 0) {
          debugStrmRef <<"VsH5Reader::getMeshDims() - Added 1 because there was a remainder." <<std::endl;
          value ++;
        }

        if (value < 1)
        value = 1;

        (*dims)[i] = value;
        debugStrmRef <<"VsH5Reader::getMeshDims() - dims[" <<i <<"] was adjusted to " <<(*dims)[i] <<std::endl;
      }
    }
    //end structured
  }

  if ( !meshMeta->isUnstructuredMesh() &&
      !meshMeta->isStructuredMesh() &&
      !meshMeta->isUniformMesh() ) {
    debugStrmRef << "VsH5Reader::getMeshDims error: mesh kind '" << kind
    << "' not implemented " << std::endl;
    debugStrmRef << "VsH5Reader::getMeshDims(" <<name <<", dims): Returning 0." << std::endl;
    return 0;
  }

  len = 1;
  for (size_t i = 0; i < dims->size(); ++i) len *= (*dims)[i];

  debugStrmRef << "VsH5Reader::getMeshDims(" <<name <<", dims): Returning " <<len <<"." << std::endl;
  return len;
}

void VsH5Reader::setFile (const std::string& fn) {
  debugStrmRef << "VsH5Reader::setFile(" <<fn <<"): Entering." << std::endl;
  fileId = H5Fopen(fn.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  filter->setFile(fileId);
  h5meta = filter->getH5Meta();
  if (meta) delete meta;

  debugStrmRef << "VsH5Reader::setFile(" <<fn <<"): Making new VsMeta()." << std::endl;
  meta = new VsMeta();
  makeVsMeta();
  debugStrmRef << "VsH5Reader::setFile(" <<fn <<"): Exiting." << std::endl;
}

bool VsH5Reader::registerComponentInfo(std::string componentName, std::string varName, int componentNumber) {
  //yes, I should use a std::hash_map for this

  //first, look for a match and report failure if the name is already registered
  NamePair foundPair;
  getComponentInfo(componentName, &foundPair);
  if (!foundPair.first.empty()) {
    std::string temp = foundPair.first;
    int tempIndex = foundPair.second;
    if ((varName != temp) || (componentNumber != tempIndex)) {
      debugStrmRef <<"ERROR VsH5Reader::registerComponentInfo() - " <<componentName <<" is already registered to component " <<temp <<" index " <<tempIndex <<std::endl;
      return false;
    } else {
      debugStrmRef <<"VsH5Reader::registerComponentInfo() - recieved duplicate registration for " <<varName <<" and index " <<componentNumber <<std::endl;
      debugStrmRef <<"VsH5Reader::registerComponentInfo() - but all info matches, so it should be ok" <<std::endl;
      return true;
    }
  }

  //Ok, register the new name mapping
  std::pair<std::string, int> innerPair;
  innerPair.first = varName;
  innerPair.second = componentNumber;
  std::pair<std::string, std::pair<std::string, int> > newPair;
  newPair.first = componentName;
  newPair.second = innerPair;
  componentNames.push_back(newPair);
  //debugStrmRef <<"newpair.first = " <<newPair.first <<std::endl;
  //debugStrmRef <<"newpair.second.first = " <<newPair.second.first <<std::endl;
  //debugStrmRef <<"newpair.second.second = " <<newPair.second.second <<std::endl;

  debugStrmRef <<"VsH5Reader::registerComponentInfo(" <<componentName <<", " <<varName <<", " <<componentNumber <<") - registration succeeded." <<std::endl;
  return true;
}

void VsH5Reader::getComponentInfo(std::string componentName, NamePair* namePair) {
  //yes, I should use a std::hash_map for this

  //look for a match and return the value if the name is registered
  for (unsigned int i = 0; i < componentNames.size(); i++) {
    std::pair<std::string, NamePair > foundPair = componentNames[i];
    if (foundPair.first == componentName) {
      debugStrmRef <<"VsH5Reader::getComponentInfo(" <<componentName <<") - Found registered name, returning. " <<std::endl;
      namePair->first = foundPair.second.first;
      namePair->second = foundPair.second.second;
      return;
    }
  }

  namePair->first = "";
  namePair->second = -1;
}

std::string VsH5Reader::getUniqueComponentName(std::string userName, std::string varName, int componentIndex) {
  //generates a unique name for the given component info
  //uses the supplied userName if possible
  //otherwise creates a name of the form "varName_index"
  //If that name is not available, another level of namespace will be added:
  // like "varName_index_number"
  NamePair foundInfo;

  //if the user supplied a name, start there
  if (!userName.empty()) {
    getComponentInfo(userName, &foundInfo);
    if (foundInfo.first.empty()) {
      //the user-supplied name is available
      //so return it
      return userName;
    }
  }

  //the user-supplied name is no good
  //try generating a name
  std::string compName = varName;
  std::stringstream ss;
  ss << componentIndex;
  compName.append("_");
  compName.append(ss.str());

  int secondIndex = 0;
  std::string generatedName = compName;
  getComponentInfo(compName, &foundInfo);
  while (!foundInfo.first.empty()) {
    debugStrmRef <<"VsH5Reader::getUniqueComponentName() - WARNING - component names in conflict" <<std::endl;
    debugStrmRef <<"VsH5Reader::getUniqueComponentName() - WARNING - names " <<userName <<" and " <<generatedName <<" are already taken." <<std::endl;
    ss.clear();
    ss <<varName <<"_" <<componentIndex <<"_" <<secondIndex;
    generatedName = ss.str();

    secondIndex++;
    getComponentInfo(generatedName, &foundInfo);
  }

  return generatedName;
}

std::string VsH5Reader::getOldComponentName(std::string varName, int componentIndex) {
  //generates an old-style name for the component
  //of the form "varName_index"

  std::string compName = varName;
  std::stringstream ss;
  ss << componentIndex;
  compName.append("_");
  compName.append(ss.str());

  return compName;
}
#endif
