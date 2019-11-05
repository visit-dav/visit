/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ***************************************************************************
//
// file PMDParticle.cpp
//
// Purpose:
//             PMDParticle Class methods
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// ***************************************************************************

#include "PMDParticle.h"

// ***************************************************************************
// Method: PMDParticle::PMDParticle
// Purpose:
//             Constructor
//
// Programmer: Mathieu Lobet
//
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
PMDParticle::PMDParticle()
{
      int i;

      // If there is momentum datasets
      this->momentumAvailable = 0;
      // Index of the scalar position data sets
      for(i=0;i<3;i++)
      {
            this->positionsId[i]=-1;
      }
      // Number of momentum dimensions
      this->numDimsMomenta = 0;
      // Number of position dimensions
      this->numDimsPositions = 0;
      this->charge = 0;
      this->mass   = 0;
      strcpy(this->name,"none");
      strcpy(this->path,"none");
}

// ***************************************************************************
// Method: PMDParticle::~PMDParticle
//
// Purpose:
//             Destructor
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
PMDParticle::~PMDParticle()
{

}

// ***************************************************************************
// Method: PMDParticle::ScanProperties(hid_t particleGroupId)
//
// Purpose:
//             This method scans the attributes, the groups and datasets contained
//             in a given particle group to build all properties of the
//            particle object.
//
// Programmer: Mathieu Lobet
//
// Creation: Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDParticle::ScanParticleGroup(hid_t particleGroupId)
{
      // Local variables
      int                   i;
      char                  objectName[64];
      ssize_t             length;
      hsize_t             numObjects;
      herr_t                   err;
      H5O_info_t             objectInfo;

      // Get the number of objects in the current particle group
      err = H5Gget_num_objs(particleGroupId, &numObjects);

      // Iteration over the objects of the group "particles"
      for (i = 0; i < numObjects; i++)
      {
            // Get the particle group name
            length = H5Gget_objname_by_idx(particleGroupId, (hsize_t)i,
                  objectName, (size_t) 64);

            // Get info in order to get the type: group, dataset...
            err = H5Oget_info_by_name(particleGroupId, objectName,
                                      &objectInfo, H5P_DEFAULT);

            // Checking of the type
            switch(objectInfo.type)
            {
            // If the object is a group...
            case H5O_TYPE_GROUP:

                  // We first treat the groups that we can recognize
                  // Analyzing of the charge group
                  if (strcmp(objectName,"charge")==0)
                  {
                        this->ScanCharge(particleGroupId,objectName);
                  }
                  // Analyzing of the mass group
                  else if (strcmp(objectName,"mass")==0)
                  {
                        this->ScanMass(particleGroupId,objectName);
                  }
                  // Special treatment for the positions
                  else if (strcmp(objectName,"position")==0)
                  {
                        this->ScanPositions(particleGroupId,objectName);
                  }
                  // Special treatment for the momenta
                  else if (strcmp(objectName,"momentum")==0)
                  {
                        this->ScanMomenta(particleGroupId,objectName);
                  }
                  // Position Offset
                  else if (strcmp(objectName,"positionOffset")==0)
                  {
                        // This is not implemented
                  }
                  // Then, we analyse all the other groups and their datasets.
                  // We create the corresponding vector and scalar data
                  else
                  {
                        this->ScanGroup(particleGroupId,objectName);
                  }
                  break;

            // If the object is a dataset...
            case H5O_TYPE_DATASET:

                  // Datasets at the root of the particle group are treated
                  // as scalar data
                  this->ScanDataSet(particleGroupId,objectName);
                  break;

            default:

                  cerr << " Non-valid object type while "
                          "scanning the 'particles' group: "
                       << objectName << ", this object is ignored." << endl;

                  break;
            }

      }

}

// ***************************************************************************
// Method: PMDParticle::ScanCharge
//
// Purpose:
//             This method scans the group `charge` contained in
//             the particle groups and get the useful attributes.
//
// Programmer: Mathieu Lobet
//
// Creation: Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDParticle::ScanCharge(hid_t particleGroupId, char * objectName)
{

      int                   i;
      int                   numAttr;
      int                   ndims;
      char                   attrName[64];
      hsize_t             numObjects;
    hid_t                chargeGroupId;
      hid_t                   attrId;
      hid_t                  attrType;
      hid_t                  attrSpace;
      herr_t                   err;
      hsize_t             sdim[64];

      // Openning of the group charge
      chargeGroupId = H5Gopen2(particleGroupId, objectName , H5P_DEFAULT);

      // First, get the useful attributes

      // Number of attributes
      numAttr = H5Aget_num_attrs(chargeGroupId);

      // Iteration over the attributes
    for (i = 0; i < numAttr; i++)
    {
          // Open the attribute using its loop id.
            attrId = H5Aopen_idx(chargeGroupId, (unsigned int)i );

            // Get the attribute name
            H5Aget_name(attrId, 64, attrName );

            //cerr << attrName << endl;

            // The type of the attribute
            attrType  = H5Aget_type(attrId);
            // Space
          attrSpace = H5Aget_space(attrId);
          // Number of dimensions
          ndims = H5Sget_simple_extent_ndims(attrSpace);
          //
          err = H5Sget_simple_extent_dims(attrSpace, sdim, NULL);

            if (strcmp(attrName,"value")==0)
            {
                  // Check it is a 32-bit float
                  if (H5T_FLOAT == H5Tget_class(attrType))
                  {
                        err = H5Aread(attrId, attrType, &this->charge);
                        //cerr << "charge value: "<< this->charge << endl;
                  }
                  else
                  {
                        cerr << " Particle charge is not a 32bit-float" << endl;
                  }
            }
            else if (strcmp(attrName,"shape")==0)
            {
                  // Check if this parameter is an integer
                  // Check it is a 32-bit float
                  if (H5T_INTEGER== H5Tget_class(attrType))
                  {
                        err = H5Aread(attrId, attrType, &this->numParticles);
                  }
                  else
                  {
                        cerr << " Particle shape is not an integer" << endl;
                  }
            }
            H5Aclose(attrId);
    }

      // Then, get the number of objects in the current group
      err = H5Gget_num_objs(chargeGroupId, &numObjects);

      // If no object, the charge is the same for all particles of this group
      if (numObjects>0)
      {
            this->charge = 0;
            // Read the dataset of charges
      }

}

// ***************************************************************************
// Method: PMDParticle::ScanMass
//
// Purpose:
//             This method scans the group `charge` contained in the particle
//             groups and get the useful attributes.
//
// Programmer: Mathieu Lobet
//
// Creation:   Fri Oct 14  2016
//
// Modifications:
//
// ***************************************************************************
void PMDParticle::ScanMass(hid_t particleGroupId, char * objectName)
{
      int                   i;
      int                   numAttr;
      int                   ndims;
      char                   attrName[64];
      hsize_t             numObjects;
    hid_t                chargeGroupId;
      hid_t                   attrId;
      hid_t                  attrType;
      hid_t                  attrSpace;
      herr_t                   err;
      hsize_t             sdim[64];

      // Openning of the group charge
      chargeGroupId = H5Gopen2(particleGroupId, objectName , H5P_DEFAULT);

      // First, get the useful attributes

      // Number of attributes
      numAttr = H5Aget_num_attrs(chargeGroupId);

      // Iteration over the attributes
    for (i = 0; i < numAttr; i++)
    {
          // Open the attribute using its loop id.
            attrId = H5Aopen_idx(chargeGroupId, (unsigned int)i );

            // Get the attribute name
            H5Aget_name(attrId, 64, attrName );

            //cerr << attrName << endl;

            // The type of the attribute
            attrType  = H5Aget_type(attrId);
            // Space
          attrSpace = H5Aget_space(attrId);
          // Number of dimensions
          ndims = H5Sget_simple_extent_ndims(attrSpace);
          //
          err = H5Sget_simple_extent_dims(attrSpace, sdim, NULL);

            if (strcmp(attrName,"value")==0)
            {
                  // Check it is a 32-bit float
                  if (H5T_FLOAT == H5Tget_class(attrType))
                  {
                        err = H5Aread(attrId, attrType, &this->mass);
                        //cerr << "charge value: "<< this->charge << endl;
                  }
                  else
                  {
                        cerr << " Particle mass is not a 32bit-float" << endl;
                  }
            }
            H5Aclose(attrId);
    }

      // Then, get the number of objects in the current group
      err = H5Gget_num_objs(chargeGroupId, &numObjects);

      // If no object, the charge is the same for all particles of this group
      if (numObjects>0)
      {
            this->mass = 0;
            // Read the dataset of charges
      }
}

// ***************************************************************************
// Method: PMDParticle::ScanPositions
//
// Purpose:
//             This method scans the group `Position` located in the particle
//             groups for each iteration.
//
//             This method reads and store the useful attributes from the
//             group itself and the datasets. A `scalarDataSet` structure object is
//            created for each dataset
//             and is put in the vector of *scalar* data `this->scalarDataSets`
//            (member of the class `Particle`).
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDParticle::ScanPositions(hid_t particleGroupId, char * objectName)
{

#ifdef VERBOSE_MODE
      cerr << " PMDParticle::SetPositions(" << objectName << ")" << endl;
#endif

    int               i;
    char              bufferName[64];
    char              datasetName[64];
    ssize_t           length;
    hsize_t           numObjects;
    hsize_t           datasetStorageSize;
    hid_t             groupId;
    hid_t             dataSetId;
    hid_t             datasetType;
    scalarDataSet     scalar;
    herr_t            err;
    H5O_info_t        objectInfo;

    strcpy(bufferName,this->name);
    strcat(bufferName,"/position/");

      // Openning of the group Position
      groupId = H5Gopen2(particleGroupId, objectName , H5P_DEFAULT);

      // First, we get the useful attributes from the position group
      SetScalarAttributes(groupId,&scalar);

      // Then, we scan the different datasets contained in the position group:

      // Get the number of objects
      err = H5Gget_num_objs(groupId, &numObjects);

      // Iteration over the objects of the group "particles"
      for (i = 0; i < numObjects; i++)
      {
            // Get the object name
            length = H5Gget_objname_by_idx(groupId, (hsize_t)i,
                  datasetName, (size_t) 64);

            // Get info in order to get the type: group, dataset...
            err = H5Oget_info_by_name(groupId, datasetName , &objectInfo,
                                        H5P_DEFAULT);

            // We check that the object is well a dataset, else,
            // this object is ignored
            if (objectInfo.type == H5O_TYPE_DATASET)
            {

                  // Open the dataset for the attributes
                  dataSetId = H5Dopen2(groupId, datasetName , H5P_DEFAULT);

                  // Get attributes from the dataset
                  SetScalarAttributes(dataSetId,&scalar);

                  // Name
                  strcpy(scalar.name,bufferName);
                  strcat(scalar.name,datasetName);

                  // Path
                  strcpy(scalar.path,this->path);
                  strcat(scalar.path,"/");
                  strcat(scalar.path,objectName);
                  strcat(scalar.path,"/");
                  strcat(scalar.path,datasetName);

                  // data Size
                  datasetType = H5Dget_type(dataSetId);
                  scalar.dataSize = H5Tget_size(datasetType);
                  // data Class
                  scalar.dataClass = H5Tget_class(datasetType);
                  // Storage size
                  datasetStorageSize = H5Dget_storage_size(dataSetId);
                  // Number of elements
                  scalar.numElements = int(datasetStorageSize/scalar.dataSize);

                  H5Dclose(dataSetId);
            }

            // We add this scalar object to the list of scalar datasets
            this->scalarDataSets.push_back(scalar);

            // We store the index of the position datasets in positionsId to
            // find them easily
            if (strcmp(datasetName,"x")==0)
            {
                  // index of the dataset x in the list of datsets
                  this->positionsId[0] = this->scalarDataSets.size()-1;
                  // Add a new dimension
                  this->numDimsPositions += 1;
            }
            if (strcmp(datasetName,"y")==0)
            {
                  // index of the dataset y in the list of datsets
                  this->positionsId[1] = this->scalarDataSets.size()-1;
                  // Add a new dimension
                  this->numDimsPositions += 1;
            }
            if (strcmp(datasetName,"z")==0)
            {
                  // index of the dataset z in the list of datsets
                  this->positionsId[2] = this->scalarDataSets.size()-1;
                  // Add a new dimension
                  this->numDimsPositions += 1;
            }
      }

      H5Gclose(groupId);
}

// ***************************************************************************
// Method: PMDParticle::ScanMomenta
//
// Purpose:
//             This method scans the group `Momentum` located in the particle
//             groups for each iteration.
//
//             This method analyzes the group `momentum` for a given particle group.
//             The group `Momentum`, when existing, provides 3 datasets: x, y, z
//            in 3D.
//             These datasets can be used to build vtk scalar lists, vtk vectors (p)
//             and vtk expressions (|p| and gamma).
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDParticle::ScanMomenta(hid_t particleGroupId, char * objectName)
{

#ifdef VERBOSE_MODE
      cerr << " PMDParticle::SetMomenta(" << objectName << ")" << endl;
#endif

    int                 i;
    int                 vectorDataSetId[3] = {-1,-1,-1};
    char                bufferName[64];
    char                datasetName[64];
    hsize_t             datasetStorageSize;
    ssize_t             length;
    hid_t               groupId;
    hid_t               dataSetId;
    hid_t               datasetType;
    scalarDataSet       scalar;
    vectorDataSet       vector;
    hsize_t             numObjects;
    herr_t              err;
    H5O_info_t          objectInfo;

    // Momenta become available
    this->momentumAvailable = 1;

    strcpy(bufferName,this->name);
    strcat(bufferName,"/momentum");

    // Openning of the group Momentum
    groupId = H5Gopen2(particleGroupId, objectName , H5P_DEFAULT);

    // First, we get the useful attributes from the momentum group
    // For the scalar datasets
    SetScalarAttributes(groupId,&scalar);

    // For the vector datasets
    SetVectorAttributes(groupId,&vector);

    // Then, we scan the different datasets contained in the position group:

    // Get the number of objects
    err = H5Gget_num_objs(groupId, &numObjects);

    // Iteration over the objects of the group "particles"
    for (i = 0; i < numObjects; i++)
    {
          // Get the object name
          length = H5Gget_objname_by_idx(groupId, (hsize_t)i,
                datasetName, (size_t) 64);

          // Get info in order to get the type: group, dataset...
          err = H5Oget_info_by_name(groupId, datasetName , &objectInfo,
                                      H5P_DEFAULT);

          // We check that the object is well a dataset, else,
          // this object is ignored
          if (objectInfo.type == H5O_TYPE_DATASET)
          {

                // Open the dataset for the attributes
                dataSetId = H5Dopen2(groupId, datasetName , H5P_DEFAULT);

                // Get attributes from the dataset
                SetScalarAttributes(dataSetId,&scalar);

                // Name
                strcpy(scalar.name,bufferName);
                strcat(scalar.name,"/");
                strcat(scalar.name,datasetName);

                // Path
                strcpy(scalar.path,this->path);
                strcat(scalar.path,"/");
                strcat(scalar.path,objectName);
                strcat(scalar.path,"/");
                strcat(scalar.path,datasetName);

                // data Size
                datasetType = H5Dget_type(dataSetId);
                scalar.dataSize = H5Tget_size(datasetType);
                // data Class
                scalar.dataClass = H5Tget_class(datasetType);
                // Storage size
                datasetStorageSize = H5Dget_storage_size(dataSetId);
                // Number of elements
                scalar.numElements = int(datasetStorageSize/scalar.dataSize);

                // We add this scalar object to the vector of scalar datasets
                this->scalarDataSets.push_back(scalar);

                // We keep the position of the datasets
                // for the vector construction
                if (strcmp(datasetName,"x")==0)
                {
                      vector.dataSetId [0] = this->scalarDataSets.size()-1;
                      // Add a new dimension
                      this->numDimsMomenta += 1;
                }
                else if (strcmp(datasetName,"y")==0)
                {
                      vector.dataSetId [1] = this->scalarDataSets.size()-1;
                      // Add a new dimension
                      this->numDimsMomenta += 1;
                }
                else if (strcmp(datasetName,"z")==0)
                {
                      vector.dataSetId [2] = this->scalarDataSets.size()-1;
                      // Add a new dimension
                      this->numDimsMomenta += 1;
                }
          }
    }

    // Vector Name
    strcpy(vector.name,bufferName);

    // Vector Path
    strcpy(vector.path,this->path);
    strcat(vector.path,"/");
    strcat(vector.path,objectName);
    strcat(vector.path,"/");

    // We add this vector object to the vector of vector datasets
    this->vectorDataSets.push_back(vector);

}

// ***************************************************************************
// Method: PMDParticle::ScanGroup
//
// Purpose:
//      This method scans a particle group and add the resulting scalar
//      and vector data respectively to the vectors `scalarDataSets`
//            and `vectorDataSets`.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 21 2016
//
// Modifications:
//
// ***************************************************************************
void PMDParticle::ScanGroup(hid_t particleGroupId,char * objectName)
{

#ifdef VERBOSE_MODE
      cerr << " PMDParticle::ScanGroup(" << objectName << ")" << endl;
#endif

    int                 i;
    int                 vectorDataSetId[3] = {-1,-1,-1};
    char                datasetName[128];
    ssize_t             length;
    hsize_t             datasetStorageSize;
    hid_t               groupId;
    hid_t               dataSetId;
    hid_t               datasetType;
    scalarDataSet       scalar;
    vectorDataSet       vector;
    hsize_t             numObjects;
    herr_t              err;
    H5O_info_t          objectInfo;

      // Openning of the corresponding group
      groupId = H5Gopen2(particleGroupId, objectName , H5P_DEFAULT);

      // First, we get the useful attributes from the group
      // For the scalar datasets
      SetScalarAttributes(groupId,&scalar);

      // For the vector datasets
      SetVectorAttributes(groupId,&vector);

      // Then, we scan the different datasets contained in the position group:

      // Get the number of objects
      err = H5Gget_num_objs(groupId, &numObjects);

      // Iteration over the objects of the group "particles"
      for (i = 0; i < numObjects; i++)
      {
            // Get the object name
            length = H5Gget_objname_by_idx(groupId, (hsize_t)i,
                  datasetName, (size_t) 64);

            // Get info in order to get the type: group, dataset...
            err = H5Oget_info_by_name(groupId, datasetName , &objectInfo,
                                        H5P_DEFAULT);

            // We check that the object is well a dataset, else,
            // this object is ignored
            if (objectInfo.type == H5O_TYPE_DATASET)
            {

                  // Open the dataset for the attributes
                  dataSetId = H5Dopen2(groupId, datasetName , H5P_DEFAULT);

                  // Get attributes from the dataset
                  SetScalarAttributes(dataSetId,&scalar);

                  // Name
                  strcpy(scalar.name,this->name);
                  strcat(scalar.name,"/");
                  strcat(scalar.name,objectName);
                  strcat(scalar.name,"/");
                  strcat(scalar.name,datasetName);

                  // Path
                  strcpy(scalar.path,this->path);
                  strcat(scalar.path,"/");
                  strcat(scalar.path,objectName);
                  strcat(scalar.path,"/");
                  strcat(scalar.path,datasetName);

                  // data Size
                  datasetType = H5Dget_type(dataSetId);
                  scalar.dataSize = H5Tget_size(datasetType);
                  // data Class
                  scalar.dataClass = H5Tget_class(datasetType);
                  // Storage size
                  datasetStorageSize = H5Dget_storage_size(dataSetId);
                  // Number of elements
                  scalar.numElements = int(datasetStorageSize/scalar.dataSize);

                  // We add this scalar object to the vector of scalar datasets
                  this->scalarDataSets.push_back(scalar);

                  // We keep the position of the datasets for the vector
                  // construction
                  // First vector dimension
                  if (strcmp(datasetName,"x")==0)
                  {
                        vector.dataSetId [0] = this->scalarDataSets.size()-1;
                  }
                  // Second vector dimension
                  else if (strcmp(datasetName,"y")==0)
                  {
                        vector.dataSetId [1] = this->scalarDataSets.size()-1;
                  }
                  // Third vector dimension
                  else if (strcmp(datasetName,"z")==0)
                  {
                        vector.dataSetId [2] = this->scalarDataSets.size()-1;
                  }

            }

      }

      // Vector Name
      strcpy(vector.name,this->name);
      strcat(vector.name,"/");
      strcat(vector.name,objectName);

      // Vector Path
      strcpy(vector.path,this->path);
      strcat(vector.path,"/");
      strcat(vector.path,objectName);
      strcat(vector.path,"/");

      // We add this vector object to the vector of vector datasets
      this->vectorDataSets.push_back(vector);

}

// ***************************************************************************
// Method: PMDParticle::ScanDataSet
//
//            This method scans a particle dataset and add the resulting scalar data
//            to the vector scalarDataSets.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 21 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDParticle::ScanDataSet(hid_t particleGroupId,char * objectName)
{

#ifdef VERBOSE_MODE
      cerr << " PMDParticle::ScanDataSet(" << objectName << ")" << endl;
#endif

    scalarDataSet       scalar;
    hid_t                dataSetId;
    hid_t                datasetType;
    hsize_t             datasetStorageSize;

      // Openning of the dataset
      dataSetId = H5Dopen2(particleGroupId, objectName , H5P_DEFAULT);

      // We get the useful attributes from the dataset
      SetScalarAttributes(dataSetId,&scalar);

      // Name
      strcpy(scalar.name,this->name);
      strcat(scalar.name,"/");
      strcat(scalar.name,objectName);

      // Path
      strcpy(scalar.path,this->path);
      strcat(scalar.path,"/");
      strcat(scalar.path,objectName);

      // data Size
      datasetType = H5Dget_type(dataSetId);
      scalar.dataSize = H5Tget_size(datasetType);
      // data Class
      scalar.dataClass = H5Tget_class(datasetType);
      // Storage size
      datasetStorageSize = H5Dget_storage_size(dataSetId);
      // Number of elements
      scalar.numElements = int(datasetStorageSize/scalar.dataSize);

      // We add this scalar object to the vector of scalar datasets
      this->scalarDataSets.push_back(scalar);

}

// ***************************************************************************
// Method: PMDParticle::SetScalarAttributes
//
// Purpose:
//             This method scans the attributes of the object of id objectId and
//             return it in the scalarDataSet data structure scalarObject.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDParticle::SetScalarAttributes(hid_t objectId, scalarDataSet * scalarObject)
{

#ifdef VERBOSE_MODE
      cerr << " PMDParticle::SetVarAttributes" << endl;
#endif

    int                   i;
    int                   numAttr;
    char                   attrName[64];
    hid_t                   attrId;
    hid_t                   attrType;
    hid_t                   attrSpace;

    // Number of attributes
    numAttr = H5Aget_num_attrs(objectId);

    // Iteration over the attributes
    for (i = 0; i < numAttr; i++)
    {
        // Open the attribute using its loop id.
        attrId = H5Aopen_idx(objectId, (unsigned int)i );

        // The type of the attribute
        attrType  = H5Aget_type(attrId);
        // Space
        attrSpace = H5Aget_space(attrId);

        // Get the attribute name
        H5Aget_name(attrId, 64, attrName);

        // Read useful attributes
        if (strcmp(attrName,"unitDimension")==0)
        {
            scalarObject->unitLabel =
            this->SetUnitDimension(attrName, attrId, attrType, attrSpace);
        }
        else if (strcmp(attrName,"unitSI")==0)
        {
        scalarObject->unitSI = SetUnitSI(attrName,
                                           attrId, attrType, attrSpace);
        }
    }
}

// ***************************************************************************
// Method: PMDParticle::SetVectorAttributes
//
// Purpose:
//             This method scan the attributes of the object of id objectId and
//             return it in the vectorDataSet data structure vectorObject.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 21 2016
//
// Modifications:
//
// ***************************************************************************
void
PMDParticle::SetVectorAttributes(hid_t objectId, vectorDataSet * vectorObject)
{

#ifdef VERBOSE_MODE
      cerr << " PMDParticle::SetVectorAttributes" << endl;
#endif

      int                   i;
      int                   numAttr;
      char                   attrName[64];
      hid_t                   attrId;
      hid_t                   attrType;
      hid_t                   attrSpace;

      // Number of attributes
      numAttr = H5Aget_num_attrs(objectId);

      // Iteration over the attributes
    for (i = 0; i < numAttr; i++)
    {
          // Open the attribute using its loop id.
            attrId = H5Aopen_idx(objectId, (unsigned int)i );

            // The type of the attribute
            attrType  = H5Aget_type(attrId);
            // Space
          attrSpace = H5Aget_space(attrId);

            // Get the attribute name
            H5Aget_name(attrId, 64, attrName);

            if (strcmp(attrName,"unitDimension")==0)
            {
                  vectorObject->unitLabel =
                  this->SetUnitDimension(attrName, attrId, attrType, attrSpace);
            }

    }
}

// ***************************************************************************
// Method: PMDParticle::SetUnitSI
//
// Purpose:
//             This method captures the arrtibute unitSI from a group or a dataset.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
double
PMDParticle::SetUnitSI(char * name,
                                   hid_t attrId,
                                   hid_t attrType,
                                   hid_t attrSpace)
{
      herr_t       err;
      double unitSI = 0;
    if (H5T_FLOAT == H5Tget_class(attrType)) {

        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        err = H5Aread(attrId, attrType, &unitSI);

    }
    else
    {
          cerr << " PMDParticle::GetUnitSI: unitSI not a float" << endl;
    }
    return unitSI;
}

// ***************************************************************************
// Method: PMDParticle::GetUnitDimension
//
// Purpose:
//             This method reads the UnitDimension attributes and returns
//             the unitsLabel paramerer.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
string
PMDParticle::SetUnitDimension(char * name,
                                            hid_t attrId,
                                            hid_t attrType,
                                            hid_t attrSpace)
{
    herr_t    err;
    int       i;
    string    units;
    string    unitLabel;
    int       firstunits = 0;

    unitLabel = "";

    if (H5T_FLOAT == H5Tget_class(attrType)) {

        int npoints = H5Sget_simple_extent_npoints(attrSpace);

        double *powers = new double[npoints];

        err = H5Aread(attrId, attrType, powers);

        for(i=0;i<7;i++)
        {

            if (int(powers[i])!=0)
            {

                // If this is the first units, then we don't put a dot,
                // else there is a dot between two units
                if (firstunits==0)
                {
                    units = "";
                    firstunits=1;
                }
                else
                {
                    units = ".";
                }

                // List of SI units
                switch(i)
                {
                // Distance, meter
                case 0:
                    units += "m";
                    break;
                // ass, kg
                case 1:
                    units += "kg";
                    break;
                // time, second
                case 2:
                    units += "s";
                    break;
                // Electric Current, Ampere
                case 3:
                    units += "A";
                    break;
                // Temperature, Kelvin
                case 4:
                    units += "K";
                    break;
                //amount of substance, mole
                case 5:
                    units += "mol";
                    break;
                //luminous intensity, candela
                case 6:
                    units += "candela";
                    break;
                }

                if (int(powers[i]) != 1)
                {
                    char power[8];
                    sprintf(power, "%d", int(powers[i]));
                    units += "^";
                    units += power;
                }
                unitLabel += units;
            }
        }

        delete powers;
    }
    //cerr << unitLabel << endl;
    return unitLabel;
}

// ***************************************************************************
// Method: PMDParticle::GetNumScalarDatasets
//
// Purpose:
//             This method return the number of scalar datasets stored in the vector
//             this->scalarDataSets.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
int PMDParticle::GetNumScalarDatasets()
{
      return this->scalarDataSets.size();
}

// ***************************************************************************
// Method: PMDParticle::GetNumVectorDatasets
//
// Purpose:
//             This method return the number of vector datasets stored in the vector
//             this->scalarDataSets.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
int PMDParticle::GetNumVectorDatasets()
{
      return this->vectorDataSets.size();
}

// ***************************************************************************
// Method: PMDParticle::GetDomainProperties
//
// Purpose:
//               This method returns the properties of the required block when the
//        fields are readed by block (parallel)
//
// This method returns an error code.
//
// Programmer: Mathieu Lobet
// Creation:   Nov 23 2016
//
// scalarDataSetId : index of the scalar data set in the vector scalarDataSets
// blockDim : number of domains to divide the dataset
// blockId : index of the block
// particleBlock : structure containing the properties of the block
//
// Modifications:
//
// ***************************************************************************
int
PMDParticle::GetBlockProperties(int scalarDataSetId,
                                  int blockDim,
                                  int blockId,
                                  particleBlockStruct * particleBlock)
{

#ifdef VERBOSE_MODE
    cerr << "PMDParticle::GetBlockProperties(scalarDataSetId="
         << scalarDataSetId
         << ")"<< endl;
#endif

    // Parameters
    int r;                          // division rest
    int numParticles;               // Number of particles in the dataSet

    // Copy the name of the dataset
    strcpy(particleBlock->dataSetPath,
             this->scalarDataSets[scalarDataSetId].path);

    // Number of particles in the dataset
    numParticles = this->scalarDataSets[scalarDataSetId].numElements;

    // Computation of the number of Particles in this block
    // We divide the particle dataset into blockDim subsets
    particleBlock->numParticles = numParticles / blockDim;
    r = this->numParticles%blockDim;
    if (blockId < r )
    {
        particleBlock->numParticles += 1;
    }

    // Computation of minimum idexes
    if (blockId < r )
    {
        particleBlock->minParticle = blockId*particleBlock->numParticles;
    }
    else
    {
          // the r first blocks share the rest (+1)
        particleBlock->minParticle = r*(particleBlock->numParticles+1)
                                + (blockId - r)*particleBlock->numParticles;
    }

    // Computation of maximum indexes
    particleBlock->maxParticle = particleBlock->minParticle
                                + particleBlock->numParticles -1;


    return 0;

}
