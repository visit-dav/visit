// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "visitvariable.h"
#include "ddtsim.h"
#include <string.h>
#include <stdio.h>

VisItVariable::VisItVariable() : mVarType(DDTSIM_VARTYPE_SCALAR), mCentering(DDTSIM_CENTERING_ZONE),
    mDataType(DDTSIM_TYPE_FLOAT), mNumComponents(1), mNumTuples(0), mData(0)
{
    mName[0] = '\0';
    mMeshName[0] = '\0';
}

VisItVariable::VisItVariable(const VisItVariable &other) : mVarType(other.mVarType), mCentering(other.mCentering),
    mDataType(other.mDataType), mNumComponents(other.mNumComponents), mNumTuples(other.mNumTuples), mData(other.mData)
{
    strcpy(mName,other.mName);
    strcpy(mMeshName,other.mMeshName);

    if (strlen(mName)==0)
        DDTSim::error("(ddtsim,VisItVariable) Warning: zero-length name given to copy constructor\n");
    if (strlen(mMeshName)==0)
        DDTSim::error("(ddtsim,VisItVariable) Warning: zero-length mesh name given to copy constructor\n");
}

bool
VisItVariable::operator==(const VisItVariable &other) const
{
    return strcmp(mName,other.mName)==0 &&
            strcmp(mMeshName,other.mMeshName) &&
            mVarType==other.mVarType &&
            mCentering==other.mCentering &&
            mDataType==other.mDataType &&
            mNumComponents==other.mNumComponents &&
            mNumTuples==other.mNumTuples &&
            mData==other.mData;
}

//! Determines if this object has the specified name
bool
VisItVariable::isNamed(const char *name) const
{
    return strcmp(name,mName)==0;
}

//! Set the name of this variable
/*! May be called directly within gdb */
void
VisItVariable::setName(const char *n)
{
    if (strlen(n) > MAX_NAME_LENGTH-1) {
        strncpy(mName,n,MAX_NAME_LENGTH-2);
        mName[MAX_NAME_LENGTH-1] = '\0';
    } else
        strcpy(mName, n);
}

//! Set the name of the mesh for this variable
/*! May be called directly within gdb */
void
VisItVariable::setMeshName(const char *n)
{
    if (strlen(n) > MAX_NAME_LENGTH-1) {
        strncpy(mMeshName,n,MAX_NAME_LENGTH-2);
        mMeshName[MAX_NAME_LENGTH-1] = '\0';
    } else
        strcpy(mMeshName, n);
}

//! Adds variable metadata to an existing visit_handle metadata object
/*! \param libsim reference to the libsim library
    \param metadata an allocated metadata visit_handle */
void
VisItVariable::applyMetadata(LibsimLibrary &libsim, visit_handle &metadata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if (libsim.VariableMetaData_alloc(&h)==VISIT_OKAY)
    {
        int vartype, centering;
        switch(mVarType)
        {
        case DDTSIM_VARTYPE_SCALAR: vartype = VISIT_VARTYPE_SCALAR; break;
        default: DDTSim::error("(ddtsim,VisItVariable) Unknown vartype\n");
        }
        switch(mCentering)
        {
        case DDTSIM_CENTERING_NODE: centering = VISIT_VARCENTERING_NODE; break;
        case DDTSIM_CENTERING_ZONE: centering = VISIT_VARCENTERING_ZONE; break;
        default: DDTSim::error("(ddtsim,VisItVariable) Unknown centering\n");
        }

        libsim.VariableMetaData_setName(h,mName);
        libsim.VariableMetaData_setMeshName(h,mMeshName);
        libsim.VariableMetaData_setType(h,vartype);
        libsim.VariableMetaData_setCentering(h,centering);

        libsim.SimulationMetaData_addVariable(metadata, h);
    }
    else
        DDTSim::error("(ddtsim,VisItVariable) failed to allocate handle for VisIt variable metadata\n");

}

//! Allocates and writes variable information to visit_handle h
/*! \param libsim reference to the libsim library
    \param h an unallocated visit_handle */
void
VisItVariable::applyVariable(LibsimLibrary &libsim, visit_handle &h)
{
    if (libsim.VariableData_alloc(&h)==VISIT_OKAY)
    {
        /* Pass data to VisIt (retaining ownership) */
        switch(mDataType)
        {
        case DDTSIM_TYPE_FLOAT: libsim.VariableData_setDataF(h, VISIT_OWNER_SIM,
                                    mNumComponents, mNumTuples, (float*)mData);
            break;
        case DDTSIM_TYPE_DOUBLE: libsim.VariableData_setDataD(h, VISIT_OWNER_SIM,
                                    mNumComponents, mNumTuples, (double*)mData);
            break;
        case DDTSIM_TYPE_CHAR: libsim.VariableData_setDataC(h, VISIT_OWNER_SIM,
                                    mNumComponents, mNumTuples, (char*)mData);
            break;
        case DDTSIM_TYPE_INT: libsim.VariableData_setDataI(h, VISIT_OWNER_SIM,
                                    mNumComponents, mNumTuples, (int*)mData);
            break;
        default:
            DDTSim::error("(ddtsim,VisItVariable) unknown variable type\n");
            break;
        }

        if (DDTSim::getInstance()->isLogging())
            fprintf(stderr,"(ddtsim) Variable> name=%s, nComps=%i, nTuples=%i\n",mName,mNumComponents,mNumTuples);
    }
    else
        DDTSim::error("(ddtsim,VisItVariable) failed to allocate handle for VisIt variable\n");
}
