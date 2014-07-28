#include <hdf5.h>
/**
 *
 * @file        VsFilter.cpp
 *
 * @brief       Implementation for inspecting an HDF5 file
 *
 * @version $Id: VsFilter.cpp 2 2013-02-20 20:11:25Z sveta $
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
#include <stdlib.h>
#include "VsLog.h"
#include "VsFile.h"
#include "VsObject.h"
#include "VsDataset.h"
#include "VsAttribute.h"
#include "VsRegistry.h"

struct RECURSION_DATA {
  VsRegistry* registry;
  VsObject* parent;
};

VsFile* VsFilter::readFile(VsRegistry* registry, std::string fileName) {  
  hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fclose_degree(fapl, H5F_CLOSE_SEMI);
  hid_t fileId = H5Fopen(fileName.c_str(), H5F_ACC_RDONLY, fapl);
  H5Pclose(fapl);
  if (fileId < 0) {
    VsLog::errorLog() << "VsFile::readFile(): HDF5 error opening the file '"
      << fileName << "'." << std::endl;
    return NULL;
  }
  
  VsFile* file = new VsFile(registry, fileName, fileId);
 
  RECURSION_DATA data;
  data.registry = registry;
  data.parent = NULL;
  H5Literate(fileId, H5_INDEX_NAME, H5_ITER_INC, 0, visitLinks, &data);
  
  return file;
}

int VsFilter::visitLinks(hid_t locId, const char* name,
    const H5L_info_t *linfo, void* opdata) {
  VsLog::debugLog() <<"VsFilter::visitLinks() - looking at object " <<name <<std::endl;
  
  switch (linfo->type) {
    case H5L_TYPE_HARD: {

      H5O_info_t objinfo;

      /* Stat the object */
      if(H5Oget_info_by_name(locId, name, &objinfo, H5P_DEFAULT) < 0) {
        VsLog::errorLog() <<"VsFilter::visitLinks() - unable to open object with name " <<name <<std::endl;
        VsLog::errorLog() <<"VsFilter::visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }

      switch(objinfo.type)
      {
        case H5O_TYPE_GROUP:
        return visitGroup( locId, name, opdata );
        break;
        case H5O_TYPE_DATASET:
        return visitDataset( locId, name, opdata );
        break;

        default:
        VsLog::debugLog() << "VsFilter::visitLinks: node '" << name <<
        "' has an unknown type " << objinfo.type << std::endl;
        break;
      }
    }
    break;
    //end of case H5L_TYPE_HARD
    case H5L_TYPE_EXTERNAL: {

      char *targbuf = (char*) malloc( linfo->u.val_size );

      if (H5Lget_val(locId, name, targbuf, linfo->u.val_size, H5P_DEFAULT) < 0) {
        VsLog::errorLog() <<"VsFilter::visitLinks() - unable to open external link with name " <<targbuf <<std::endl;
        VsLog::errorLog() <<"VsFilter::visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }
      
      const char *filename;
      const char *targname;

      if (H5Lunpack_elink_val(targbuf, linfo->u.val_size, 0, &filename, &targname) < 0) {
        VsLog::errorLog() <<"VsFilter::visitLinks() - unable to open external file with name " <<filename <<std::endl;
        VsLog::errorLog() <<"VsFilter::visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }
      
      VsLog::debugLog() << "VsFilter::visitLinks(): node '" << name << "' is an external link." << std::endl;
      VsLog::debugLog() << "VsFilter::visitLinks(): node '" << targname << "' is an external target group." << std::endl;

      free(targbuf);
      targbuf = NULL;
      
      // Get info of the linked object.
      H5O_info_t objinfo;
#ifndef H5_USE_16_API
      hid_t obj_id = H5Oopen(locId, name, H5P_DEFAULT);
#else
      hid_t obj_id = H5Oopen(locId, name);        
#endif
      
      if (obj_id < 0) {
        VsLog::errorLog() <<"VsFilter::visitLinks() - unable to get id for external object " <<name <<std::endl;
        VsLog::errorLog() <<"VsFilter::visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }

      //Test-open the linked object
      if (H5Oget_info (obj_id, &objinfo) < 0) {
        VsLog::errorLog() <<"VsFilter::visitLinks() - unable to open external object " <<name <<std::endl;
        VsLog::errorLog() <<"VsFilter::visitLinks() - this object and all children will be dropped." <<std::endl;
        return 0;
      }
      
      //Close the linked object to release hdf5 id
      H5Oclose( obj_id );

      //Finally, decide what to do depending on what type of object this is
      switch(objinfo.type)
      {
        case H5O_TYPE_GROUP:
        return visitGroup( locId, name, opdata );
        break;
        case H5O_TYPE_DATASET:
        return visitDataset( locId, name, opdata );
        break;

        default:
          VsLog::debugLog() << "VsFilter::visitLinks: node '" << name <<
        "' has an unknown type " << objinfo.type << std::endl;
        break;
      }
    }
    break;
      //END OF CASE H5L_TYPE_EXTERNAL
    
    default:
    VsLog::debugLog() << "VsFilter::visitLinks: node '" << name <<
    "' has an unknown object type " << linfo->type << std::endl;
    break;
  }

  return 0;
}

int VsFilter::visitGroup(hid_t locId, const char* name, void* opdata) {
  RECURSION_DATA* data = static_cast< RECURSION_DATA* >(opdata);
  VsGroup* parent = static_cast< VsGroup*> (data->parent);
  VsRegistry* registry = data->registry;
  
  VsLog::debugLog() << "VsFilter::visitGroup: node '" << name
    << "' is a group." << std::endl;

  if (std::string(name) == "..")
  {
    VsLog::debugLog() << "VsFilter::visitGroup: skipping group '..'" << std::endl;
    return 0;
  }

  hid_t groupId = H5Gopen(locId, name, H5P_DEFAULT);
  
  //If unable to get a handle to the hdf5 object, we just drop the object
  //But return 0 to continue iterating over objects
  if (groupId < 0) {
    VsLog::errorLog() <<"VsFilter::visitGroup() - Unable to open group with name " <<name <<std::endl;
    VsLog::errorLog() <<"VsFilter::visitGroup() - This object and all children will be dropped." <<std::endl;
    return 0;
  }
  
  VsGroup* newGroup = new VsGroup(registry, parent, name, groupId);
  
  RECURSION_DATA nextLevelData;
  nextLevelData.registry = registry;
  nextLevelData.parent = newGroup;
  
  // Recurse over all attributes of the group
  VsLog::debugLog() <<"VsFilter::visitGroup(): Recursing on attributes of group " <<newGroup->getFullName() <<std::endl;
  H5Aiterate(groupId, H5_INDEX_NAME, H5_ITER_INC, NULL, visitAttrib, &nextLevelData);

  // Recurse to examine child groups
  VsLog::debugLog() <<"VsFilter::visitGroup(): Recursing on children of group " <<newGroup->getFullName() <<std::endl;
  H5Literate(groupId, H5_INDEX_NAME, H5_ITER_INC, NULL, visitLinks, &nextLevelData);

  // Not needed because the newly declared VsGroup takes ownership of the id
  // And will do the H5GClose when it is deleted
  // H5Gclose(groupId);

  VsLog::debugLog() <<"VsFilter::visitGroup(): Returning." <<std::endl;
  return 0;
}

int VsFilter::visitDataset(hid_t locId, const char* name, void* opdata) {
  RECURSION_DATA* data = static_cast< RECURSION_DATA* >(opdata);
  VsGroup* parent = static_cast< VsGroup*>(data->parent);
  VsRegistry* registry = data->registry;

  VsLog::debugLog() << "VsFilter::visitDataset: node '" << name
    <<"' is a dataset." << std::endl;

  hid_t datasetId = H5Dopen(locId, name, H5P_DEFAULT);

  VsDataset* newDataset = new VsDataset(registry, parent, name, datasetId);
  
  //If unable to get a handle to the hdf5 object, we just drop the object
  //But return 0 to continue iterating over objects
  if (datasetId < 0) {
    VsLog::errorLog() <<"VsFilter::visitDataset() - Unable to open dataset with name " <<name <<std::endl;
    VsLog::errorLog() <<"VsFilter::visitDataset() - This object and all children will be dropped." <<std::endl;
    return 0;
  }

  RECURSION_DATA nextLevelData;
  nextLevelData.registry = registry;
  nextLevelData.parent = newDataset;
  
  // Recurse over attributes
  H5Aiterate(datasetId, H5_INDEX_NAME, H5_ITER_INC, NULL, visitAttrib, &nextLevelData);

  VsLog::debugLog() <<"VsFilter::visitDataset: Returning." <<std::endl;
  return 0;
}

int VsFilter::visitAttrib(hid_t dId, const char* name,
    const H5A_info_t* ai, void* opdata) {
  RECURSION_DATA* data = static_cast< RECURSION_DATA* >(opdata);
  VsObject* parent = data->parent;

  VsLog::debugLog() << "VsFilter::visitAttrib(...): getting attribute '" <<
    name << "'." << std::endl;
  
  if (!parent) {
    VsLog::errorLog() <<"VsFilter::visitAttrib(): Parent is NULL?" <<std::endl;
    return 0;
  }

  //TODO - actually load & cache string attribute values here

  hid_t attId = H5Aopen_name(dId, name);
  parent->addAttribute(name, attId);
  
  return 0;
}

