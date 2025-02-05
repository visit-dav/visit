// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ***************************************************************************
//
// file PMDParticle.h
//
// PMDParticle Class definition
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// ***************************************************************************

#ifndef PMDPARTICLE_H
#define PMDPARTICLE_H

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

/// Structure to store particle block properties
struct particleBlockStruct
{
    /// Block dimension
    int     numParticles;
    /// minimum node index
    int     minParticle;
    /// maximum node index
    int     maxParticle;
    /// Path to the dataSet
    char    dataSetPath[128];
};

// ***************************************************************************
// Class: PMDParticle
//
// Purpose:
//      This class represents the Particle groups in the openPMD files.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//      Mathieu Lobet, Tue Dec 13 2016
//      I added the parameter dataSize and dataClass
// ***************************************************************************
class PMDParticle
{
	public:

        /// Structure to store scalar vtk metadata
        struct scalarDataSet
        {
            /// Scalar data name
            char    name[64];
            /// Scalar data path in the openPMD file
            char    path[128];
            /// Units computed from the openPMD file
            string    unitLabel;
            /// Factor for SI units
            double  unitSI;
            /// Weighting Power
            double  weigthingPower;
            /// Time offset read from openPMD files
            double  timeOffset;
            /// Macro-weighted read from openPMD file
            int     macroWeighted;
            /// Data size in number of bytes (4,8)
            int     dataSize;
            /// Number of elements (particles)
            int     numElements;
            /// Data Class (H5T_FLOAT, H5T_INTEGER...)
            H5T_class_t dataClass;
        };

        /// Structure to store vector vtk metadata
        struct vectorDataSet
        {
            /// Vector data name
            char    name[64];
            /// Vector data path in the openPMD file
            char    path[128];
            /// Units computed from the openPMD file
            string    unitLabel;
            /// Corresponding scalarDataSet objects in scalarDataSets
            /// for each vector component
            int     dataSetId[3];
            /// Time offset read from openPMD files
            double  timeOffset;
            /// Weighting Power
            double  weigthingPower;
        };

        // Constructor and destructor
            	   PMDParticle();
            	   ~PMDParticle();

        // Particle methods
        void        ScanParticleGroup(hid_t particleGroupId);
        void        ScanCharge(hid_t particleGroupId, char * objectName);
        void        ScanMass(hid_t particleGroupId, char * objectName);
        void        ScanPositions(hid_t particleGroupId, char * objectName);
        void        ScanMomenta(hid_t particleGroupId, char * objectName);
        void        ScanDataSet(hid_t particleGroupId,char * objectName);
        void        ScanGroup(hid_t particleGroupId,char * objectName);
        int         GetBlockProperties(int scalarDataSetId,
                                       int blockDim,
                                       int blockId,
                                       particleBlockStruct * particleBlock);
        int         GetNumScalarDatasets();
        int         GetNumVectorDatasets();

        // Particle attributes
        /// Particle group name
        char        name[64];
        /// Particle path
        char        path[64];
        /// Number of particles (shape in attribute of mass or charge)
        /// Then, the number of particles may depend on the dataset
        long        numParticles;
        /// Particle mass (when constant for all particles)
        double      charge;
        /// Particle mass (when constant for all particles)
        double      mass;
        ///         position dimension
        int         numDimsPositions;
        ///         momentum dimension
        int         numDimsMomenta;
        /// This flag is true when the momentum is available
        bool        momentumAvailable;
        /// ScalarDataSet index in scalarDataSets for the x positions
        int         positionsId[3];

        /// Contain all the scalar for this particle group
        vector <scalarDataSet> scalarDataSets;
        /// Contain all the vectors for this particle group
        vector <vectorDataSet> vectorDataSets;

    protected:


    private:

        void        SetScalarAttributes(hid_t objectId,
                                        scalarDataSet * scalar);
        void        SetVectorAttributes(hid_t objectId,
                                        vectorDataSet * vectorObject);
        string      SetUnitDimension(char* name, hid_t attrId,
                                     hid_t attrType,
                                     hid_t attrSpace);
        double      SetUnitSI(char * name, hid_t attrId,
                              hid_t attrType, hid_t attrSpace);

};

#endif
