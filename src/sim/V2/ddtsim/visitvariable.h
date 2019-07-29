// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISITVARIABLE_H
#define VISITVARIABLE_H

#include "ddtsim_constants.h"
#include "VisItInterfaceTypes_V2.h"
#include "libsimlibrary.h"
#include "visitmesh.h"

class VisItVariable
{
public:
    VisItVariable();
    VisItVariable(const VisItVariable&);

    bool operator==(const VisItVariable&) const;
    bool isNamed(const char*) const;
    void setName(const char *);
    void setMeshName(const char*);

    void applyVariable(LibsimLibrary&, visit_handle &var);
    void applyMetadata(LibsimLibrary&, visit_handle &metadata);

private:
    char mName[MAX_NAME_LENGTH];    //!< Name of this variable
    char mMeshName[MAX_NAME_LENGTH];//!< Name of the mesh this variable is drawn on
    VisItVarType mVarType;          //!< Type of this variable: scalar, vector, etc
    VisItCenteringType mCentering;  //!< How variable is centered on the mesh: zone or node
    VisItDataType mDataType;        //!< The datatype of this variable: float, int, etc
    int mNumComponents;             //!< The number of array elements (in mData) that comprise a single tuple
    int mNumTuples;                 //!< The number of tuples in the array mData
    void* mData;                    //!< Address of the start of the array to visualise
};

#endif // VISITVARIABLE_H
