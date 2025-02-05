// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ***************************************************************************
//
// file PMDFile.h
//
// Purpose:
//      Definition of the class PMDFile
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// ***************************************************************************

#ifndef PMDFILE_H
#define PMDFILE_H

#include "PMDIteration.h"
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include <hdf5.h>
//#include <visit-hdf5.h>
//#include "H5Cpp.h"

using namespace std;

// ***************************************************************************
// Class: PMDFile
//
// Purpose:
//      The class PMDFile enables to read the structure of an openPMD file.
//      This class constitutes the main one to read openPMD files.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
class PMDFile
{
	public:

		PMDFile();
		~PMDFile();

		/// File path
		char                    filePath[128];
		/// openPMD standard version
		string                  version;
		/// Meshes path
		string 					meshesPath;
		// Particles path
		string 					particlesPath;
		/// Id of the file after opening
		hid_t                   fileId;

		/// vector of PMDIteration objects
		/// that contains the openPMD iterations
		vector <PMDIteration>   iterations;

		void                    OpenFile(char * path);
		void                    ScanFileAttributes();
		void                    ScanIterations();
		void                    ScanFields();
		void                    ScanParticles();
		int                     ReadScalarDataSet(void * array,
		                                int numValues,
		                                void * factor,
		                                H5T_class_t fieldDataClass,
		                                char * path);
		int                     ReadFieldScalarBlock(void * array,
		                                void * factor,
		                                H5T_class_t fieldDataClass,
		                                fieldBlockStruct * fieldBlock);
		int                     ReadParticleScalarBlock(void * array,
		                                                void * factor,
		                                            H5T_class_t dataSetClass,
		                                particleBlockStruct * particleBlock);
		void                    CloseFile();

		int                     GetNumberIterations() const;
		void                    Print();

	protected:


	private:

};

#endif
