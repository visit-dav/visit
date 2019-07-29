// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISITDATA_H
#define VISITDATA_H

#include "visitmesh.h"
#include "visitvariable.h"

class VisItData
{
public:
    VisItData();
    ~VisItData();

    void updateMesh(const VisItMesh&);
    void updateVariable(const VisItVariable&);

    // TODO: Later this may hold a number of meshes and variables, for now just one of each
    VisItMesh*     getMesh()        { return mMesh; }
    VisItVariable* getVariable()    { return mVariable; }
private:
    VisItMesh     *mMesh;
    VisItVariable *mVariable;
};

#endif // VISITDATA_H
