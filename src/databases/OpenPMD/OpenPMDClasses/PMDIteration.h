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
*	this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*	this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*	documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*	be used to endorse or promote products derived from this software without
*	specific prior written permission.
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

#ifndef PMDITERATION_H
#define PMDITERATION_H

#include "PMDField.h"
#include "PMDParticle.h"
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
// Class: PMDIteration
//
// Purpose:
//	  This class enables to manage the different iterations of
//	  an OpenPMD file.
//	  When an OpenPMD file is read, a PMDIteration is created
//	  for each iteration.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//
// ***************************************************************************
class PMDIteration
{
	public:
	PMDIteration();
	~PMDIteration();

	void	ScanFields(hid_t fileId);
	void	ScanParticles(hid_t fileId);
	void	PrintInfo();
	bool	HasFieldOfName(char * fieldName);

	// Iteration attributes
	/// Iteration name
	char	name[20];
	/// Mesh path in the iteration group
	char	meshesPath[64];
	/// Particles path in the iteration group
	char	particlesPath[64];
	/// Iteration time step
	float  	dt;
	/// Iteration corresponding time
	float 	time;
	/// factor to convert the time in SI units
	float 	timeUnitSI;

	/// Vector of field objects from the datasets
	vector <PMDField> fields;

	/// Vector of field group structures
	vector <fieldGroupStruct> fieldGroups;

	/// Vector of particle objects
	vector <PMDParticle> particles;

	protected:

};

#endif
