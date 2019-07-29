// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "visitdata.h"

VisItData::VisItData() : mMesh(0), mVariable(0)
{
}

VisItData::~VisItData()
{
    if (mMesh)
        delete mMesh;
    if (mVariable)
        delete mVariable;
}

//! Replaces the current mesh with the specified mesh
/*! The mesh coordinates are generated for the new current mesh. No action is taken if the
 *  specified mesh holds the same information as the current one. */
void
VisItData::updateMesh(const VisItMesh &mesh)
{
    if (!mMesh || !(*mMesh==mesh))
    {
        delete mMesh;
        mMesh = new VisItMesh(mesh);
        mMesh->generate();
    }
}

//! Replaces the current variable info with that specified
/*! No action is taken if the specified mesh holds the same information as the current one */
void
VisItData::updateVariable(const VisItVariable &var)
{
    if (!mVariable || !(*mVariable==var))
    {
        delete mVariable;
        mVariable = new VisItVariable(var);
    }
}
