// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//	  an openPMD file.
//	  When an openPMD file is read, a PMDIteration is created
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
	string	name;
	/// Mesh path in the iteration group
	string	meshesPath;
	/// Particles path in the iteration group
	string	particlesPath;
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
