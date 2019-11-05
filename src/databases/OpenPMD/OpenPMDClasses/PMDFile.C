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
//                                 PMDFile.cpp
// Purposes:
// this file contains the PMDFile class method.
//
// Programmer: Mathieu Lobet
//
// Creation:   Fri Oct 14 2016
//
// ***************************************************************************

#include "PMDFile.h"

#include <stdio.h>

#ifndef TEST
#include <InvalidVariableException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <InvalidTimeStepException.h>
#include <InvalidDBTypeException.h>
#endif

// ***************************************************************************
// Method: PMDFile::PMDFile
//
// Purpose:
//   Constructor: Initialize the object
//
// Notes:     Any special notes for users of the class.
//
// Programmer: Mathieu Lobet
// Creation: Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
PMDFile::PMDFile()
{
	fileId=-1;
	this->version = "";
	strcpy(this->meshesPath,"");
}

// ***************************************************************************
// Method: PMDFile::~PMDFile
//
// Purpose:
//   Destructor of the container PMDFile
//
// Notes:     Any special notes for users of the class.
//
// Programmer: Mathieu Lobet
// Creation: Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
PMDFile::~PMDFile()
{

}

// ***************************************************************************
// Method: PMDFile::OpenFile
//
// Purpose:
//      Open the OpenPMD file
//
// Programmer: Mathieu Lobet
// Creation: Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDFile::OpenFile(char * PMDFilePath)
{
	#ifdef VERBOSE_MODE
	    cerr << "PMDFile::OpenFile" << endl;
	#endif

	hid_t fileAccessPropListID = H5Pcreate(H5P_FILE_ACCESS);

	herr_t err = H5Pset_fclose_degree(fileAccessPropListID, H5F_CLOSE_SEMI);

	// Open the file
	fileId = H5Fopen(PMDFilePath, H5F_ACC_RDONLY, H5P_DEFAULT);

	H5Pclose(fileAccessPropListID);

	// The path is copied in this->filePath if the file was well opened.
	strcpy(this->filePath,PMDFilePath);
}

// ***************************************************************************
// Method: PMDFile::ScanFileAttributes
//
// Purpose:
//      This method scans all attributes at the root of the file
//
// Programmer: Mathieu Lobet
// Creation:   Tue Oct 25 2016
//
// Modifications:
// Nov. 9 2017 - M. Lobet - add buffer + `\0` for a correct reading
//
// ***************************************************************************
void PMDFile::ScanFileAttributes()
{

#ifdef VERBOSE_MODE
	cerr << "PMDFile::ScanFileAttributes" << endl;
#endif

	int 			iAttr;
	char 			attrName[64];
	hsize_t			nbAttr;
	hid_t			groupId;
	hid_t			attrId;
	hid_t 			atype;
	hid_t 			aspace;
	size_t 			size;
	int				nPoints;

	// OpenPMD files always contain a data group at the root
	groupId = H5Gopen(fileId, "/",H5P_DEFAULT);

	// Number of attributes
	nbAttr = H5Aget_num_attrs(groupId);

	// Loop over the attributes
	for (iAttr = 0; iAttr < nbAttr; iAttr++)
	{
		// Opening of the attribute
		attrId = H5Aopen_idx(groupId, (unsigned int)iAttr );

		// Get the name of the attribute
		H5Aget_name(attrId, 64, attrName);
		// the dimensions of the attribute data
		aspace = H5Aget_space(attrId);
		// The type of the attribute
		atype  = H5Aget_type(attrId);
		// Number of elements in the attribute
		nPoints = H5Sget_simple_extent_npoints(aspace);
		// Size of an element (number of char for instance)
		size = H5Tget_size (atype);

		if (strcmp(attrName,"openPMD")==0)
		{
			char *buffer = new char[size + 1];

			// Read attribute
			H5Aread (attrId, atype, buffer);
			buffer[size] = '\0';

			this->version = buffer;

            delete buffer;
		}
		else if (strcmp(attrName,"meshesPath")==0)
		{
			char *buffer = new char[size + 1];

			// Read attribute
			H5Aread (attrId, atype, buffer);
			buffer[size] = '\0';

			strncpy(this->meshesPath,buffer,sizeof(buffer));
			delete buffer;

		}
		else if (strcmp(attrName,"particlesPath")==0)
		{
            char *buffer = new char[size + 1];

			// Read attribute
			H5Aread (attrId, atype, buffer);
			buffer[size] = '\0';

			strncpy(this->particlesPath,buffer,sizeof(buffer));

            delete buffer;
		}
    }

}

// ***************************************************************************
// Method: PMDFile::ScanIterations
//
// Purpose:
//      This method scans the group /data that contains the iteration groups.
//      Each iteration is stored in the member iterations which is a vector
//      of objects PMDIteration.
//      Iteration group attributes are read and store too.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDFile::ScanIterations()
{

	#ifdef VERBOSE_MODE
	    cerr << "PMDFile::ScanIterations" << endl;
	#endif


	hsize_t 		nbIterations; // Number of iterations
	hsize_t			nbAttr;
	hid_t    		groupId;
	hid_t			iterationId;
	hid_t			attrId;
	int 			i;
	int 			iAttr;
	int 			length;
	herr_t 			err;
	char			iterationName[64];
	char 			bufAttrName[64];
	PMDIteration 	iteration;
	double 			val;
	hid_t 			atype;
	hid_t 			aspace;
	H5O_info_t 		objectInfo;

	// OpenPMD files always contain a data group at the root
	groupId = H5Gopen(fileId, "/data",H5P_DEFAULT);

	//H5Gget_num_objs(group_iterations->getId(), &nobj);
	H5Gget_num_objs(groupId, &nbIterations);

	// We scan by "hand" all groups in the group data that corresponds
    // to the different iterations

	// iteration over the iteration group
    for (i = 0; i < nbIterations; i++)
    {

		// Get the object name
		length = H5Gget_objname_by_idx(groupId, (hsize_t)i,
			iterationName, (size_t) 64);

		// Get the type: group, dataset...
		err = H5Oget_info_by_name(groupId, iterationName , &objectInfo,
                                  H5P_DEFAULT);

		// Check that it is a group, we ignore dataset in the data group...
		if (objectInfo.type == H5O_TYPE_GROUP)
		{

		    // Openning of the iteration group
			iterationId = H5Gopen2(groupId, iterationName, H5P_DEFAULT);

			// Save the iteration name
			strcpy(iteration.name,iterationName);

			// Save mesh path
			strcpy(iteration.meshesPath,this->meshesPath);

			// Save particles path
			strcpy(iteration.particlesPath,this->particlesPath);

			// Number of attributes
			nbAttr = H5Aget_num_attrs(iterationId);

			// Loop over the attributes
		    for (iAttr = 0; iAttr < nbAttr; iAttr++)
		    {
				attrId = H5Aopen_idx(iterationId, (unsigned int)iAttr );

				// Get the name of the attribute
				H5Aget_name(attrId, 64, bufAttrName);
				/* the dimensions of the attribute data */
				aspace = H5Aget_space(attrId);
				// The type of the attribute
				atype  = H5Aget_type(attrId);

				if (strcmp(bufAttrName,"dt")==0)
				{
					// Read attribute
					H5Aread (attrId, atype, &val);
					iteration.dt = val;
				}
				else if (strcmp(bufAttrName,"time")==0)
				{
					// Read attribute
					H5Aread (attrId, atype, &val);
					iteration.time = val;
				}
				else if (strcmp(bufAttrName,"timeUnitSI")==0)
				{
					// Read attribute
					H5Aread (attrId, atype, &val);
					iteration.timeUnitSI = val;
				}

				H5Aclose(attrId);
			}

			// Add the iteration in the list of iterations
			iterations.push_back(iteration);
		}
    }

    H5Gclose(groupId);

}

// ***************************************************************************
// Method: PMDFile::ScanFields
//
// Purpose:
//      This method scans the fields in each iteration. For this aim,
//      this method calls the PMDIteration method ScanFields.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDFile::ScanFields()
{
	for (std::vector<PMDIteration>::iterator it = iterations.begin() ;
		 it != iterations.end(); ++it)
	{
	 	it->ScanFields(this->fileId);
	}
}

// ***************************************************************************
// Method: PMDFile::ScanParticles
//
// Purpose:
//      This method scans the particles in each iteration. For this aim,
//      this method calls the PMDIteration method ScanParticles.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDFile::ScanParticles()
{
	for (std::vector<PMDIteration>::iterator it = iterations.begin() ;
         it != iterations.end(); ++it)
	{
	 	it->ScanParticles(this->fileId);
	}
}

// ***************************************************************************
// Method: PMDFile::Print
//
// Purpose:
//      This method prints the structure of the file.
//      For this aim, this method calls the
//      PMDIteration method PMDIteration::PrintInfo().
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDFile::Print()
{

	cout << " File: " << this->filePath << endl;
	cout << " OpenPMD Version: " << this->version << endl;

	cout << endl;
	cout << " Number of iteration: " << this->GetNumberIterations()
             << endl;
	for (std::vector<PMDIteration>::iterator it = iterations.begin() ;
         it != iterations.end(); ++it)
	{
	 	it->PrintInfo();
	}
}

// ***************************************************************************
// Method: PMDFile::GetNumberIterations
//
// Purpose:
//      This method gives the number of iteration in the opened file.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
int PMDFile::GetNumberIterations() const
{
	return iterations.size();
}

// ***************************************************************************
// Method: PMDFile::CloseFile
//
// Purpose:
//      This method closes the opened OpenPMD file.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
void PMDFile::CloseFile()
{
	H5Fclose(fileId);
}

// ***************************************************************************
// Method: PMDFile::ReadScalarDataSet
//
// Purpose
//      This method reads the specified scalar dataset given by path
//      and returns the resulting array.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Arguments:
//      array : pointer to the array that will receive the content
//            of the dataset
//      numValues : number of element to read
//      factor : multiplication factor applied to all elements of array
//      fieldDataClass : Dataset type (H5T_FLOAT...)
//      path : path to the data set in the OpenPMD file
//
// Returns:  <0 on failure, 0 on success.
//
// Modifications:
//      Mathieu Lobet, Tue Dec 13 2016
//      I added double dataset and double multiplication factor.
// ***************************************************************************
int
PMDFile::ReadScalarDataSet(void * array,
                           int numValues,
                           void * factor,
                           H5T_class_t dataClass,
                           char * path)
{

#ifdef VERBOSE_MODE
    cerr << "PMDFile::ReadScalarDataSet("
         << "numValues=" << numValues << ","
         << "path=" << path << ")"
         << endl;
#endif

	int 	ndims;
    int     dataSize;
    hid_t   datasetId;
    hid_t   datasetType;
    hid_t   datasetSpace;
    hsize_t datasetStorageSize;

    // Open the corresponding dataset
    if ((datasetId = H5Dopen(this->fileId,path,H5P_DEFAULT))<0)
    {
        char error[1024];
#ifndef TEST
        snprintf(error, 1024, "Problem when opening the dataset %d",
                 int(datasetId));
        EXCEPTION2(InvalidFilesException, (const char *) path,error);
        cerr << " Problem when opening the dataset: " << path << endl;
#endif
        return -1;
    }
    else
    {

        // Data space
        datasetSpace = H5Dget_space(datasetId);
        // Data type
        datasetType  = H5Dget_type(datasetId);
        // Data size
        dataSize = H5Tget_size(datasetType);
        // Storage size
        datasetStorageSize = H5Dget_storage_size(datasetId);
        // Dimension from the data space
        ndims        = H5Sget_simple_extent_ndims(datasetSpace);

        // Check the class of the dataset
        if (dataClass==H5T_FLOAT)
        {

            // Correct number of values in the dataset
            if (numValues == int(datasetStorageSize/dataSize))
            {

                if (H5Dread(datasetId, datasetType, H5S_ALL, H5S_ALL,
                            H5P_DEFAULT, array) < 0)
                {
#ifndef TEST
                    EXCEPTION1(InvalidVariableException, path);
#endif
                    cerr << " Problem when reading the dataset: "
                         << path << endl;
                    return -4;
                }

                // ___ Application of the factor to the data _________________

                if (dataSize == 4)
                {
                    float factorTmp = *(float*) (factor);
    		        if (factorTmp != 1)
    		        {
                        float * arrayTmp = (float*) (array);
                        //cerr << " Application of the factor: "
                        //     << factorTmp << endl;

    		        	for (int i=0;i<numValues;i++)
    		        	{
    		        		arrayTmp[i] *= factorTmp;
    		        	}
    		        }
                }
                else if (dataSize == 8)
                {

									  // Factor is still a float
                    float factorTmp = *(float*) (factor);
                    if (factorTmp != 1)
                    {
                        double * arrayTmp = (double*) (array);
                        /*cerr << " Application of the factor: "
                             << factorTmp << endl;*/

                        for (int i=0;i<numValues;i++)
                        {
                            arrayTmp[i] *= (double)(factorTmp);
                        }
                    }
                }
                //cerr << " End Application of the factor" << endl;


            }
            else
            {
                char error[1024];
                snprintf(error, 1024,
                         "Invalid size for the current dataset (%d %ld)",
                         numValues,long(datasetStorageSize));

#ifndef TEST
                EXCEPTION2(InvalidFilesException, (const char *) path,error);
#endif
            	cerr << " Invalid size for the current dataset:" << numValues
                     << " " << long(datasetStorageSize) << endl;
                return -3;
            }
        }
        else
        {
#ifndef TEST
            EXCEPTION2(InvalidFilesException,
                       (const char *) path,
                        "The current dataset is not of a valid class.");
#endif
            cerr << "The current dataset, " << path
                 << ", is not a valid class: " << dataClass << endl;
            return -2;
        }

        //H5Dclose(datasetId);
        //H5Tclose(datasetType);
        //H5Sclose(datasetSpace);

    }
    //cerr << " End ReadScalarDataSet" << endl;
   	return 0;
}

// ***************************************************************************
// Method: PMDFile::ReadFieldScalarBlock
//
// Purpose
//      This method reads a block of data from a field dataset specified
//      by fieldBlock.
//
// Programmer: Mathieu Lobet
// Creation:   Mon Nov 14 2016
//
// Arguments:
//      array output array
//      factor multiply factor
//      dataSetClass Dataset type (H5T_FLOAT...)
//      ieldBlock field block properties
//
// Returns:  <0 on failure, 0 on success.
//
// Modifications:
//      Mathieu Lobet, Tue Dec 13 2016
//      I added the parallel reading of 2D datasets.
//      I added double dataset and double multiplication factor.
//
// ***************************************************************************
int
PMDFile::ReadFieldScalarBlock(void * array,
                              void * factor,
                              H5T_class_t fieldDataClass,
                              fieldBlockStruct * fieldBlock)
{

    int     ndims;
    int     err;
    int     dataSize;
    hid_t   datasetId;
    hid_t   datasetType;
    hid_t   datasetSpace;
    hsize_t datasetStorageSize;

    //cerr  << "PMDFile::ReadFieldScalarBlock" << endl;

    // Open the corresponding dataset
    if ((datasetId = H5Dopen(this->fileId,fieldBlock->dataSetPath,
        H5P_DEFAULT))<0)
    {
        char error[1024];
        snprintf(error, 1024, "Problem when opening the dataset %d",
                 int(datasetId));
#ifndef TEST
        EXCEPTION2(InvalidFilesException, (const char *)
                   fieldBlock->dataSetPath,error);
#endif
        cerr << " Problem when opening the dataset: "
             << fieldBlock->dataSetPath << endl;
        return -1;
    }
    else
    {

        // Data space
        datasetSpace = H5Dget_space(datasetId);
        // Data type
        datasetType  = H5Dget_type(datasetId);
        // Data size
        dataSize = H5Tget_size(datasetType);
        // Storage size
        datasetStorageSize = H5Dget_storage_size(datasetId);
        // Dimension from the data space
        ndims        = H5Sget_simple_extent_ndims(datasetSpace);

        // Check the class of the dataset
        if (fieldDataClass == H5T_FLOAT)
        {

            // ___ Read the dataset __________________________________________
            // 3D dataset
            if (ndims==3)
            {

                // Parameters for the hyperslab
                hsize_t start[3];
                hsize_t block[3];
                hsize_t stride[3];
                hsize_t count[3];
                hid_t   memspace;

                // Fill the parameters for the hyperslab
                // using the fieldBlock properties
                start[0] = fieldBlock->minNode[0];
                start[1] = fieldBlock->minNode[1];
                start[2] = fieldBlock->minNode[2];
                block[0] = 1;   block[1] = 1;   block[2] = 1;
                stride[0] = 1;  stride[1] = 1;  stride[2] = 1;
                count[0]  = fieldBlock->nbNodes[0];
                count[1]  = fieldBlock->nbNodes[1];
                count[2] = fieldBlock->nbNodes[2];

                //Define hyperslab in the dataset.
                err = H5Sselect_hyperslab(datasetSpace, H5S_SELECT_SET,
                                          start, stride, count, block);

                if (err!=0)
                {
                    cerr << " Problem when defining "
                    " the hyperslab in the dataset" << endl;
                    return -3;
                }

                // Create memory dataspace.
                // Dimension sizes of the dataset in memory when we read
                // selection from the dataset on the disk
                hsize_t mdim[] = {static_cast<hsize_t>(fieldBlock->nbNodes[0]),
                                  static_cast<hsize_t>(fieldBlock->nbNodes[1]),
                                  static_cast<hsize_t>(fieldBlock->nbNodes[2])};

                // Define the memory dataspace.
                memspace = H5Screate_simple (fieldBlock->ndims, mdim, NULL);

                start[0] = 0;   start[1] = 0;   start[2] = 0;
                block[0] = 1;   block[1] = 1;   block[2] = 1;
                stride[0] = 1;  stride[1] = 1;  stride[2] = 1;
                count[0]  = fieldBlock->nbNodes[0];
                count[1]  = fieldBlock->nbNodes[1];
                count[2] = fieldBlock->nbNodes[2];

                // Define memory hyperslab.
                err = H5Sselect_hyperslab (memspace, H5S_SELECT_SET,
                                           start, stride, count, block);

                if (H5Dread(datasetId, datasetType, memspace, datasetSpace,
                    H5P_DEFAULT, array) < 0)
                {
#ifndef TEST
                    EXCEPTION1(InvalidVariableException,
                               fieldBlock->dataSetPath);
#endif
                    cerr << " Problem when reading the dataset: "
                         << fieldBlock->dataSetPath << endl;
                    return -4;
                }
            }
            // 2D Dataset
            else if (ndims==2)
            {

                // Parameters for the hyperslab
                hsize_t start[2];
                hsize_t block[2];
                hsize_t stride[2];
                hsize_t count[2];
                hid_t   memspace;

                // Fill the parameters for the hyperslab
                // using the fieldBlock properties
                start[0] = fieldBlock->minNode[0];
                start[1] = fieldBlock->minNode[1];
                block[0] = 1;   block[1] = 1;
                stride[0] = 1;  stride[1] = 1;
                count[0]  = fieldBlock->nbNodes[0];
                count[1]  = fieldBlock->nbNodes[1];

                //Define hyperslab in the dataset.
                err = H5Sselect_hyperslab(datasetSpace, H5S_SELECT_SET,
                                          start, stride, count, block);

                if (err!=0)
                {
                    cerr << " Problem when defining the "
                            "hyperslab in the dataset" << endl;
                    return -3;
                }

                // Create memory dataspace.
                // Dimension sizes of the dataset in memory when
                // we read selection from the dataset on the disk
                hsize_t mdim[] = { static_cast<hsize_t>(fieldBlock->nbNodes[0]),
                                   static_cast<hsize_t>(fieldBlock->nbNodes[1])};

                // Define the memory dataspace.
                memspace = H5Screate_simple(fieldBlock->ndims, mdim, NULL);

                start[0] = 0;   start[1] = 0;
                block[0] = 1;   block[1] = 1;
                stride[0] = 1;  stride[1] = 1;
                count[0]  = fieldBlock->nbNodes[0];
                count[1]  = fieldBlock->nbNodes[1];

                // Define memory hyperslab.
                err = H5Sselect_hyperslab (memspace, H5S_SELECT_SET,
                                           start, stride, count, block);

                // Reading of the dataset
                if (H5Dread(datasetId, datasetType, memspace,
                            datasetSpace, H5P_DEFAULT, array) < 0)
                {
#ifndef TEST
                    EXCEPTION1(InvalidVariableException,
                               fieldBlock->dataSetPath);
#endif
                    cerr << " Problem when reading the dataset: "
                         << fieldBlock->dataSetPath
                         << endl;
                    return -4;
                }

            }

            // ___ Application of the factor to the data _____________________

            if (dataSize == 4)
            {
                float factorTmp = *(float*) (factor);
                if (factorTmp != 1)
                {
                    float * arrayTmp = (float*) (array);
                    for (int i=0;i<fieldBlock->nbTotalNodes;i++)
                    {
                        arrayTmp[i] *= factorTmp;
                    }
                }
            }
            else if (dataSize == 8)
            {
                // factor is still a float
                float factorTmp = *(float*) (factor);
                if (factorTmp != 1)
                {
                    double * arrayTmp = (double*) (array);
                    for (int i=0;i<fieldBlock->nbTotalNodes;i++)
                    {
                        arrayTmp[i] *= factorTmp;
                    }
                }
            }

        }
        else
        {
#ifndef TEST
            EXCEPTION2(InvalidFilesException,
                       (const char *) fieldBlock->dataSetPath,
                       "The current dataset is not of a valid class.");
#endif
            cerr << "The current dataset, " << fieldBlock->dataSetPath
                 << ", is not a valid class: " << fieldDataClass << endl;
            return -2;
        }

        H5Dclose(datasetId);
        H5Sclose(datasetSpace);
        H5Tclose(datasetType);

    }
    //cerr << " End ReadFieldScalarBlock" << endl;

    return 0;
}

// ***************************************************************************
// Method: PMDFile::ReadParticleScalarBlock
//
// Purpose:
//      This method reads a block of data from a particle dataset
//      specified by particleBlock.
//
// Programmer: Mathieu Lobet
// Creation:   Mon Nov 14 2016
//
// Arguments:
//      array : array that will contain the data from the dataset
//      factor : multiplication factor
//      dataSetClass : data set type (H5T_FLOAT...)
//      particleBlock : structure that contains useful information
//                      for the block division
//
// Modifications:
//      Mathieu Lobet, Tue Dec 13 2016
//      I added double dataset and double multiplication factor.
//
// ***************************************************************************
int PMDFile::ReadParticleScalarBlock(void * array,
                                     void * factor,
                                     H5T_class_t dataSetClass,
                                     particleBlockStruct * particleBlock)
{
    int     ndims;
    int     err;
    int     dataSize;
    hid_t   datasetId;
    hid_t   datasetType;
    hid_t   datasetSpace;
    hsize_t datasetStorageSize;

    //cerr  << "PMDFile::ReadParticleScalarBlock" << endl;

    // Open the corresponding dataset
    if ((datasetId = H5Dopen(this->fileId,particleBlock->dataSetPath,
                             H5P_DEFAULT))<0)
    {
        cerr << " Problem when opening the dataset: "
             << particleBlock->dataSetPath << endl;
        return -1;
    }
    else
    {
        // Parameters for the hyperslab
        hsize_t start[1];
        hsize_t block[1];
        hsize_t stride[1];
        hsize_t count[1];
        hid_t   memspace;

        // Data space
        datasetSpace = H5Dget_space(datasetId);
        // Data type
        datasetType  = H5Dget_type(datasetId);
        // Data size
        dataSize = H5Tget_size(datasetType);
        // Storage size
        datasetStorageSize = H5Dget_storage_size(datasetId);
        // Dimension from the data space
        ndims        = H5Sget_simple_extent_ndims(datasetSpace);

        // Check the class of the dataset
        if ((H5Tget_class(datasetType) == dataSetClass)
            &&((dataSetClass==H5T_FLOAT)))
        {

            // Fill the parameters for the hyperslab
            // using the particleBlock properties
            start[0] = particleBlock->minParticle;
            block[0] = 1;
            stride[0] = 1;
            count[0]  = particleBlock->numParticles;

            //Define hyperslab in the dataset.
            err = H5Sselect_hyperslab(datasetSpace,
                                      H5S_SELECT_SET,
                                      start,
                                      stride,
                                      count,
                                      block);

            if (err!=0)
            {
                cerr << " Problem when defining the hyperslab in the dataset"
                     << endl;
                return -3;
            }

            // Create memory dataspace.
            // Dimension sizes of the dataset in memory when we read
            // selection from the dataset on the disk
            hsize_t mdim[] = { static_cast<hsize_t>(particleBlock->numParticles)};

            // Define the memory dataspace.
            memspace = H5Screate_simple (1, mdim, NULL);

            start[0] = 0;
            block[0] = 1;
            stride[0] = 1;
            count[0]  = particleBlock->numParticles;

            // Define memory hyperslab.
            err = H5Sselect_hyperslab (memspace, H5S_SELECT_SET,
                                       start, stride, count, block);

            if (H5Dread(datasetId, datasetType, memspace, datasetSpace,
                        H5P_DEFAULT, array) < 0)
            {
#ifndef TEST
                EXCEPTION1(InvalidVariableException,
                           particleBlock->dataSetPath);
#endif
                cerr << " Problem when reading the dataset: "
                     << particleBlock->dataSetPath << endl;
                return -4;
            }

            // ___ Application of the factor to the data _____________________

            if (dataSize==4)
            {
                float factorTmp = *(float*) (factor);
                if (factorTmp != 1)
                {
                    float * arrayTmp = (float*) (array);
                    for (int i=0;i<particleBlock->numParticles;i++)
                    {
                        arrayTmp[i] *= factorTmp;
                    }
                }
            }
            else if (dataSize==8)
            {
							  // factor is still a float
                float factorTmp = *(float*) (factor);
                if (factorTmp != 1)
                {
                    double * arrayTmp = (double*) (array);
                    for (int i=0;i<particleBlock->numParticles;i++)
                    {
                        arrayTmp[i] *= (double)(factorTmp);
                    }
                }
            }
						else
						{
								cerr << " Error in PMDFile::ReadParticleScalarBlock"
										 << endl;
								cerr << " DataSize is not recognized"
										 << endl;
						}

        }
        else
        {
#ifndef TEST
            EXCEPTION2(InvalidFilesException,
                       (const char *) particleBlock->dataSetPath,
                       "The current dataset is not a float dataset");
#endif
            cerr << "The current dataset, " << particleBlock->dataSetPath
                 << ", is not of the specified class:" << H5T_FLOAT
                 << endl;
            return -2;
        }

        H5Dclose(datasetId);
        H5Sclose(datasetSpace);
        H5Tclose(datasetType);
    }

    return 0;

}
