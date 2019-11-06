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
//      The class PMDFile enables to read the structure of an OpenPMD file.
//      This class constitutes the main one to read OpenPMD files.
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
		/// OpenPMD version
		string                  version;
		/// Meshes path
		char 					meshesPath[64];
		// Particles path
		char 					particlesPath[64];
		/// Id of the file after opening
		hid_t                   fileId;

		/// vector of PMDIteration objects
		/// that contains the OpenPMD iterations
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
