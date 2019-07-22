// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISITMESH_H
#define VISITMESH_H

#include "ddtsim_constants.h"
#include <VisItInterfaceTypes_V2.h>
#include "libsimlibrary.h"
#include "visitsim.h"

//! Class representing a single mesh to be visualised with VisIt
/*!
 * Most of the data members are statically allocated, and so can be safely set via gdb.
 * The arrays defining the coordinates of the mesh need to be allocated, this is done by calling
 * generate(). VisItMesh objects that have had their dynamic arrays generated should not be
 * used or deleted via gdb, such operations must be done in the signal handler.
 */
class VisItMesh
{
public:
    VisItMesh();
    VisItMesh(const VisItMesh&);
    ~VisItMesh();

    bool operator==(const VisItMesh&) const;
    bool isNamed(const char *) const;
    void setName(const char *);

    void setLabelX(const char *);
    void setLabelY(const char *);
    void setLabelZ(const char *);

    bool isGenerated() const;           //!< Determines if the dyanamic arrays have been generated
    void generate();                    //!< Generate the dyanmic arrays

    void applyMesh(LibsimLibrary&, visit_handle &mesh);
    void applyPointMesh(LibsimLibrary&, visit_handle &mesh);
    void applyRectilinearMesh(LibsimLibrary&, visit_handle &mesh);
    void applyMetadata(LibsimLibrary&, VisItSim&, visit_handle &metadata);

private:
    void setString(const char*,char*);

    VisItMeshType mType;//!< i.e. rectilinear or point
    bool mGenerated;    //!< true if the dynamic arrays have been allocated
    char mName[MAX_NAME_LENGTH];    //!< Name of the mesh. Hardcoded max-size
    char mLabelX[MAX_NAME_LENGTH];  //!< Label for X axis. Hardcoded max-size
    char mLabelY[MAX_NAME_LENGTH];  //!< Label for Y axis. Hardcoded max-size
    char mLabelZ[MAX_NAME_LENGTH];  //!< Label for Z axis. Hardcoded max-size
    int mBaseIndex[3];  //!< Offset coordinates [x,y,z]
    int mDimension[3];  //!< Size of each [x,y,z] dimension
    int mIncrement[3];  //!< Distance between points on each dimension [x,y,z]
    int mAxisOrder[3];  //!< Axis-order in which point meshes should be populated. i.e. mAxisOrder[0]=1 means the x coordinate (0) should be increment second (1). Only used in point meshes.
    float* mCoords[3];  //!< Pointers to arrays of coordinate values [x,y,z]. May be axis marks along each axis in a rectilinear mesh, or the (x,y,z) coordinates of each point in a point mesh
    int mNumPoints;     //!< Number of points in a point mesh. Set by generate().
};

#endif // VISITMESH_H
