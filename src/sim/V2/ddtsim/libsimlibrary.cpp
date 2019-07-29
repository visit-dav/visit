// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "libsimlibrary.h"

#ifndef VISIT_STATIC
LibsimLibrary::LibsimLibrary(const char* lib) :
    mLibsim(lib)
{
    mVisItUpdatePlots = (void (*)()) mLibsim.symbol("VisItUpdatePlots");
    mVisItTimeStepChanged = (void (*)()) mLibsim.symbol("VisItTimeStepChanged");

    mVisItDetectInput = (int (*)(int,int)) mLibsim.symbol("VisItDetectInput");
    mVisItProcessEngineCommand = (int (*)(void)) mLibsim.symbol("VisItProcessEngineCommand");
    
    mVariableData_alloc = (int (*)(visit_handle*)) mLibsim.symbol("VisIt_VariableData_alloc");
    mDomainList_alloc = (int (*)(visit_handle*)) mLibsim.symbol("VisIt_DomainList_alloc");
    
    mVisIt_SimulationMetaData_alloc = (int (*)(visit_handle*)) mLibsim.symbol("VisIt_SimulationMetaData_alloc");
    mVisIt_SimulationMetaData_setMode = (int (*)(visit_handle,int)) mLibsim.symbol("VisIt_SimulationMetaData_setMode");
    mVisIt_SimulationMetaData_setCycleTime = (int (*)(visit_handle,int,double)) mLibsim.symbol("VisIt_SimulationMetaData_setCycleTime");
    mVisIt_SimulationMetaData_addGenericCommand = (int (*)(visit_handle,visit_handle)) mLibsim.symbol("VisIt_SimulationMetaData_addGenericCommand");
    mVisIt_SimulationMetaData_addMesh = (int (*)(visit_handle,visit_handle)) mLibsim.symbol("VisIt_SimulationMetaData_addMesh");
    mVisIt_SimulationMetaData_addMessage = (int (*)(visit_handle,visit_handle)) mLibsim.symbol("VisIt_SimulationMetaData_addMessage");
    mVisIt_SimulationMetaData_addVariable = (int (*)(visit_handle,visit_handle)) mLibsim.symbol("VisIt_SimulationMetaData_addVariable");
    
    mVisIt_MeshMetaData_alloc = (int (*)(visit_handle*)) mLibsim.symbol("VisIt_MeshMetaData_alloc");
    mVisIt_MeshMetaData_setName = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_MeshMetaData_setName");
    mVisIt_MeshMetaData_setMeshType = (int (*)(visit_handle,int)) mLibsim.symbol("VisIt_MeshMetaData_setMeshType");
    mVisIt_MeshMetaData_setTopologicalDimension = (int (*)(visit_handle,int)) mLibsim.symbol("VisIt_MeshMetaData_setTopologicalDimension");
    mVisIt_MeshMetaData_setSpatialDimension = (int (*)(visit_handle,int)) mLibsim.symbol("VisIt_MeshMetaData_setSpatialDimension");
    mVisIt_MeshMetaData_setNumDomains = (int (*)(visit_handle,int)) mLibsim.symbol("VisIt_MeshMetaData_setNumDomains");
    mVisIt_MeshMetaData_setXUnits = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_MeshMetaData_setXUnits");
    mVisIt_MeshMetaData_setYUnits = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_MeshMetaData_setYUnits");
    mVisIt_MeshMetaData_setZUnits = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_MeshMetaData_setZUnits");
        mVisIt_MeshMetaData_setXLabel = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_MeshMetaData_setXLabel");
        mVisIt_MeshMetaData_setYLabel = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_MeshMetaData_setYLabel");
        mVisIt_MeshMetaData_setZLabel = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_MeshMetaData_setZLabel");

    mVisIt_VariableMetaData_alloc = (int (*)(visit_handle*)) mLibsim.symbol("VisIt_VariableMetaData_alloc");
    mVisIt_VariableMetaData_setName = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_VariableMetaData_setName");
    mVisIt_VariableMetaData_setMeshName = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_VariableMetaData_setMeshName");
    mVisIt_VariableMetaData_setUnits = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_VariableMetaData_setUnits");
    mVisIt_VariableMetaData_setType = (int (*)(visit_handle,int)) mLibsim.symbol("VisIt_VariableMetaData_setType");
    mVisIt_VariableMetaData_setCentering = (int (*)(visit_handle,int)) mLibsim.symbol("VisIt_VariableMetaData_setCentering");
    
    mVisIt_CommandMetaData_alloc = (int (*)(visit_handle*)) mLibsim.symbol("VisIt_CommandMetaData_alloc");
    mVisIt_CommandMetaData_setName = (int (*)(visit_handle,const char*)) mLibsim.symbol("VisIt_CommandMetaData_setName");
    mVisIt_CommandMetaData_setEnable = (int (*)(visit_handle,const int)) mLibsim.symbol("VisIt_CommandMetaData_setEnable");

    mDomainList_setDomains = (int (*)(visit_handle,int,visit_handle)) mLibsim.symbol("VisIt_DomainList_setDomains");
    mVariableData_setDataI = (int (*)(visit_handle,int,int,int,int*)) mLibsim.symbol("VisIt_VariableData_setDataI");
    mVariableData_setDataF = (int (*)(visit_handle,int,int,int,float*)) mLibsim.symbol("VisIt_VariableData_setDataF");
    mVariableData_setDataD = (int (*)(visit_handle,int,int,int,double*)) mLibsim.symbol("VisIt_VariableData_setDataD");
    mVariableData_setDataC = (int (*)(visit_handle,int,int,int,char*)) mLibsim.symbol("VisIt_VariableData_setDataC");

    mVisIt_RectilinearMesh_alloc = (int (*)(visit_handle*)) mLibsim.symbol("VisIt_RectilinearMesh_alloc");
    mVisIt_RectilinearMesh_setCoordsXY = (int (*)(visit_handle,visit_handle,visit_handle)) mLibsim.symbol("VisIt_RectilinearMesh_setCoordsXY");
    mVisIt_RectilinearMesh_setCoordsXYZ = (int (*)(visit_handle,visit_handle,visit_handle,visit_handle)) mLibsim.symbol("VisIt_RectilinearMesh_setCoordsXYZ");
    mVisIt_RectilinearMesh_setBaseIndex = (int (*)(visit_handle,int[3])) mLibsim.symbol("VisIt_RectilinearMesh_setBaseIndex");

    mVisIt_PointMesh_alloc = (int (*)(visit_handle*)) mLibsim.symbol("VisIt_PointMesh_alloc");
    mVisIt_PointMesh_setCoordsXY = (int (*)(visit_handle,visit_handle,visit_handle)) mLibsim.symbol("VisIt_PointMesh_setCoordsXY");
    mVisIt_PointMesh_setCoordsXYZ = (int (*)(visit_handle,visit_handle,visit_handle,visit_handle)) mLibsim.symbol("VisIt_PointMesh_setCoordsXYZ");
}
#endif
