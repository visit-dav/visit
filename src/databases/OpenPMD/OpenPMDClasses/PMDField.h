// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ***************************************************************************
//
// file PMDField.h
//
// Purpose:
//      PMDField class description
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// ***************************************************************************

#ifndef PMDFIELD_H
#define PMDFIELD_H

#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <cmath>
#include <iostream>

#include <hdf5.h>
//#include <visit-hdf5.h>
//#include "H5Cpp.h"

using namespace std;

/// Structure to store field block properties
struct fieldBlockStruct
{
    /// Block dimension
    int     ndims;
    /// number of nodes
    int     nbNodes[3];
    /// minimum node index
    int     minNode[3];
    /// maximum node index
    int     maxNode[3];
    /// Total number of nodes
    int     nbTotalNodes;
    /// Path to the dataSet
    char    dataSetPath[128];
};

/// Field group structure
/// This structure represents the group in the openPMD file
struct fieldGroupStruct
{
    /// name
    char name[64];
    /// List of id in the field vector
    vector <int> fieldIds;
    /// Geometry fo the group
    string geometry;
    /// Components if cartesian geometry
    int cartesianComponents[3];
    /// If geometry is thetaMode, we keep in memory
    /// where are the components. This will be used to compute
    /// Ex, Ey, Ez if they do not exist
    int thetaComponents[3];
};

// ***************************************************************************
// Class: PMDField
//
// Purpose:
//      This class represents the field datasets in the openPMD file.
//
// Programmer: Mathieu Lobet
// Creation:   Fri Oct 14 2016
//
// Modifications:
//      Mathieu Lobet, Tue Dec 13 2016
//      I added the parameter dataSize and dataClass
// ***************************************************************************
class PMDField
{
	public:
            	PMDField();
            	~PMDField();

        // Field attributes
        /// name of the field
        char    name[64];
        /// path to the dataset in the hdf5 file
        char    datasetPath[128];
        /// path to the group containing this dataset
        char    groupPath[128];
        /// number of dimensions
        int     ndims;
        /// number of nodes in each direction
        int     nbNodes[3];
        /// Number of nodes to discretize in theta for Axisymmetric mode
        int     thetaNbNodes;
        /// Number of modes for the theta mode
        int     nbModes;
        /// Signe of the operation of the imaginary part for the modes
        /// with the theta geometry
        int     thetaImSign;
        /// Grid spacing in each direction (max 3)
        double  gridSpacing[3];
        /// Origin of the grid
        double  gridGlobalOffset[3];
        /// Shift of the grid
        double  gridPosition[3];
        /// Units for the axis
        double  unitSI;
        /// Units for the grid
        double  gridUnitSI;
        /// Geometry of the grid
        string    geometry;
        /// Labels
        string    axisLabels[3];
        /// Units
        string    unitsLabel;
        /// Field boundary conditions
        string    fieldBoundary[6];
        /// Field boundary conditions
        string    fieldBoundaryParameters[6];
        /// Data order (C or Fortran)
        string    dataOrder;
        /// Data size in number of bytes (4,8)
        int     dataSize;
        /// Data Class (H5T_FLOAT, H5T_INTEGER...)
        H5T_class_t dataClass;

        void    ScanAttributes(hid_t object_id);
        void    SetGridDimensions(hid_t dataset_id);
        int     GetNumValues () const;
        int     GetBlockProperties (int blockDim,
                                    int blockId,
                                    fieldBlockStruct * fieldBlock)
                                    const;
        int     ComputeArrayThetaMode(void * dataSetArray,
                                      void * finalDataArray);
    protected:

    private:
        void    SetGridSpacing(char * name,
                               hid_t attr_id,
                               hid_t attr_type,
                               hid_t attr_space);
        void    SetGridGlobalOffset(char * name,
                                    hid_t attr_id,
                                    hid_t attr_type,
                                    hid_t attr_space);
        void    SetGridPosition(char * name,
                                hid_t attr_id,
                                hid_t attr_type,
                                hid_t attr_space);
        void    SetUnitSI(char * name,
                          hid_t attr_id,
                          hid_t attr_type,
                          hid_t attr_space);
        void    SetGridUnitSI(char * name,
                              hid_t attr_id,
                              hid_t attr_type,
                              hid_t attr_space);
        void    SetGeometry(char * name,
                            hid_t attr_id,
                            hid_t attr_type,
                            hid_t attr_space);
        void    SetAxisLabels(char * name,
                              hid_t attr_id,
                              hid_t attr_type,
                              hid_t attr_space);
        void    SetUnitDimension(char * name,
                                 hid_t attr_id,
                                 hid_t attr_type,
                                 hid_t attr_space);
        void    SetFieldBoundary(char * name,
                                 hid_t attr_id,
                                 hid_t attr_type,
                                 hid_t attr_space);
        void    SetFieldBoundaryParameters(char * name,
                                  hid_t attr_id,
                                  hid_t attr_type,
                                  hid_t attr_space);
        void    SetDataOrder(char * name,
                             hid_t attr_id,
                             hid_t attr_type,
                             hid_t attr_space);

        int    SetGeometryParameters(char * name,
                                             hid_t attrId,
                                             hid_t attrType,
                                             hid_t attrSpace);
};

#endif
