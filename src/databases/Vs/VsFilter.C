#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1,8,1)
/**
 *
 * @file        VsFilter.cpp
 *
 * @brief       Implementation for inspecting an HDF5 file
 *
 * @version $Id: VsFilter.C 496 2009-08-05 22:57:22Z mdurant $
 *
 * Copyright &copy; 2007-2008, Tech-X Corporation
 * See LICENSE file for conditions of use.
 *
 */

// vsh5
#include <VsFilter.h>
#include <VsSchema.h>
#include <VsUtils.h>
#include <hdf5.h>

struct RECURSION_DATA {
  VsH5Meta* meta;
  std::ostream* debugStream;
  int depth;
  std::string path;
};

VsFilter::VsFilter(std::ostream& dbgstrm) : debugStrmRef(dbgstrm) {
}

VsFilter::VsFilter(hid_t fId, std::ostream& dbgstrm) :
        debugStrmRef(dbgstrm), fileId(fId) {
  makeH5Meta();
}

void VsFilter::makeH5Meta() {
  h5meta.ptr = 0;
// Create the list of vs datasets and groups
// corresponding to variables, variables with mesh and meshes.
  RECURSION_DATA data;
  data.debugStream = &debugStrmRef;
  data.meta = &h5meta;
  data.depth = 0;
  data.path = "";

  H5Giterate(fileId, "/", NULL, visitIfc, &data);
}

herr_t VsFilter::visitIfc(hid_t locId, const char* name, void* opdata) {
  RECURSION_DATA* data = static_cast< RECURSION_DATA* >(opdata);
  VsH5Meta* metaPtr = data->meta;
  std::ostream& osRef = *(data->debugStream);

//cerr <<"working with interface " << endl;
  herr_t ret = 0;
  H5G_stat_t statbuf;
  H5Gget_objinfo (locId, name, false, &statbuf);

  //the fully qualified name of this object
  std::string fullName = makeCanonicalName(data->path, name);//data->path + "/" + name;

  switch (statbuf.type) {

    case H5G_GROUP: {
      osRef << "VsFilter::visitIfc: node '" << name << "' is a group." << std::endl;
      hid_t groupId = H5Gopen(locId, name, H5P_DEFAULT);
// Metadata for this group
      VsGMeta* gm = new VsGMeta();
      gm->name = name;
      gm->depth = data->depth + 1;
      gm->path = data->path;
      gm->iid = groupId;
// Collect attributes of the group
      std::pair<VsIMeta*, std::ostream*> gpd(gm, &osRef);
      H5Aiterate(groupId, H5_INDEX_NAME, H5_ITER_INC, NULL, visitAttrib, &gpd);
// Check is gm's isMesh is true and add to meshes
// Check if the group is a mesh and register.  Move the ptr to the mesh
      if (gm->isMesh) {
        osRef << "VsFilter::visitIfc: node '" << name << "' is a mesh."
              << std::endl;
        std::pair<std::string, VsGMeta*> el(fullName, gm);
        metaPtr->gMeshes2.insert(el);
        metaPtr->ptr = gm;
      }
      else if (gm->isVsVars) {
        osRef << "VsFilter::visitIfc: node '" << name << "' is vsVars." <<
                std::endl;
        std::pair<std::string, VsGMeta*> el(fullName, gm);
        metaPtr->vsVars2.insert(el);
        metaPtr->ptr = gm;
      }
      else {
// If group is not a mesh - delete it - needs to be changed to work with groups
// which are variables
        osRef << "VsFilter::visitIfc: node '" << name << "' is not a mesh."
              << "  Deleting." << std::endl;
        delete gm;
      }

      // recurse to examine child groups
      RECURSION_DATA nextLevelData;
      nextLevelData.debugStream = data->debugStream;
      nextLevelData.depth = data->depth + 1;
      nextLevelData.path = fullName;
      nextLevelData.meta = data->meta;
      osRef <<"VsFilter::visitIfc(...): Recursing with path " <<fullName <<" and depth " <<nextLevelData.depth <<"." <<std::endl;
      H5Giterate(locId, name, NULL, visitIfc, &nextLevelData);
      osRef <<"VsFilter::visitIfc(...): Returned from recursion" <<std::endl;

      break;
    }

    case H5G_DATASET: {
      osRef << "VsFilter::visitIfc: node '" << name <<
        "' is a dataset." << std::endl;
      hid_t datasetId = H5Dopen(locId, name, H5P_DEFAULT);
      osRef << "VsFilter::visitIfc: node '" << name <<
        "' opened." << std::endl;

// Metadata for this dataset
      VsDMeta* vm = new VsDMeta();
      std::string sname = name;
      vm->name = sname;
      vm->path = data->path;
      vm->depth = data->depth + 1;
      vm->type = H5Tget_native_type(H5Dget_type(datasetId), H5T_DIR_DEFAULT);
      vm->iid = datasetId;
      getDims(vm->iid, true, vm->dims);
// Collect attributes of this dataset
      std::pair<VsIMeta*, std::ostream*> gpd(vm, &osRef);
      H5Aiterate(datasetId, H5_INDEX_NAME, H5_ITER_INC, NULL, visitAttrib,
        &gpd);

      //all datasets get registered in a general list
      std::pair<std::string, VsDMeta*> el(fullName, vm);
      metaPtr->datasets.insert(el);

// Test isVariable etc and add register
      if (vm->isVariable) {
              std::pair<std::string, VsDMeta*> el2(fullName, vm);
        metaPtr->vars2.insert(el2);
        osRef <<"VsFilter::visitIfc: node '" <<name <<
          "' is a Variable." <<std::endl;
      }
      if (vm->isVariableWithMesh) {
              std::pair<std::string, VsDMeta*> el2(fullName, vm);
        metaPtr->varsWithMesh2.insert(el2);
        osRef <<"VsFilter::visitIfc: node '" <<name <<
          "' is a variable with mesh." <<std::endl;
      }
      if (vm->isMesh) {
              std::pair<std::string, VsDMeta*> el2(fullName, vm);
        metaPtr->dMeshes2.insert(el2);
        osRef <<"VsFilter::visitIfc: node '" <<name <<
          "' is a dataset mesh." <<std::endl;
      }
// If this dataset is not a var, mesh or varWith mesh, and its parent is a
// mesh group, attach to this mesh and do not change the ptr.  Otherwise delete it.
      if (metaPtr->ptr) {
        if ((!vm->isVariable) && (!vm->isVariableWithMesh)&&(!vm->isMesh) &&
            (static_cast<VsGMeta*>(metaPtr->ptr)->isMesh)) {
          (static_cast<VsGMeta*>(metaPtr->ptr))->datasets.push_back(vm);
        }
        if ((!vm->isVariable) && (!vm->isVariableWithMesh)&&(!vm->isMesh) &&
            (!static_cast<VsGMeta*>(metaPtr->ptr)->isMesh) &&
            (metaPtr->ptr)) {
          delete vm;
        }
      }
    }
    default:
      break;
  }
  return ret;
}

herr_t VsFilter::visitAttrib(hid_t dId, const char* name,
       const H5A_info_t* ai, void* opdata) {

  std::pair<VsIMeta*, std::ostream*>* gpdPtr =
        static_cast<std::pair<VsIMeta*, std::ostream*>*>(opdata);
  // VsIMeta* iMetaPtr = static_cast<VsIMeta*>(opdata);
  VsIMeta* iMetaPtr = gpdPtr->first;
  std::ostream& osRef = *gpdPtr->second;

  osRef << "VsFilter::visitAttrib(...): getting attribute '" <<
        name << "'." << std::endl;

  hid_t attId = H5Aopen_name(dId, name);
// Metadata for this attribute
  VsAMeta* am = new VsAMeta();
  am->type = H5Tget_native_type(H5Aget_type(attId), H5T_DIR_DEFAULT);
  am->aid = attId;
  am->name = name;
// 0 shows that it is attribute
  getDims(am->aid, false, am->dims);

// add attrib metadata
  am->depth = iMetaPtr->depth+1;
  iMetaPtr->attribs.push_back(am);
// check for type attribute
  std::string aname = name;
  if (aname.compare(VsSchema::typeAtt) == 0) {
    std::string attValue;
    herr_t err = getAttributeHelper(attId, &attValue, 0, 0);
    if (err == 0) {
     osRef << "VsFilter::visitAttrib(...): attribute '" << name <<
        "' is '" << attValue << "'." << std::endl;
// See if this dataset or group is a mesh
      if (attValue.compare(VsSchema::meshKey) == 0) {
        iMetaPtr->isMesh = true;
      }
// See if this group is derived vars
      if ( (iMetaPtr->isGroup) && (attValue.compare(VsSchema::vsVarsKey) == 0)) {
        (static_cast<VsGMeta*>(iMetaPtr))->isVsVars = true;
      }
      if (!iMetaPtr->isGroup) {
// See if this dataset is a variable or variableWithMesh
        if(attValue.compare(VsSchema::varKey) == 0) {
          iMetaPtr->isVariable = true;
        }
        if(attValue.compare(VsSchema::varWithMeshKey) == 0) {
          iMetaPtr->isVariableWithMesh = true;
        }
      }
    }
    else {
     osRef << "VsFilter::visitAttrib(...): error getting attribute '" <<
        name << "'.  Is this of type H5T_STR_NULLTERM and a SCALAR?" << std::endl;
    }
  }
  else {
     osRef << "VsFilter::visitAttrib(...): attribute '" << name <<
        "' is not '" << VsSchema::typeAtt << "'." << std::endl;
  }
  return 0;
}

void VsFilter::write() const {
  h5meta.write(debugStrmRef);
}

void VsFilter::setFile(hid_t fId) {
  fileId = fId;
  h5meta.clear();
  makeH5Meta();
}

#endif
