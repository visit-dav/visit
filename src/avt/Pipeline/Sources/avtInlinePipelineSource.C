// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtInlinePipelineSource.C                        //
// ************************************************************************* //

#include <avtInlinePipelineSource.h>


// ****************************************************************************
//  Method: avtInlinePipelineSource constructor
//
//  Arguments:
//      dob     A data object that is part of the real pipeline.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2001
//
// ****************************************************************************

avtInlinePipelineSource::avtInlinePipelineSource(avtDataObject_p dob)
{
    realPipelineSource = dob->GetOriginatingSource();
}


// ****************************************************************************
//  Method: avtInlinePipelineSource destructor
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2001
//
// ****************************************************************************

avtInlinePipelineSource::~avtInlinePipelineSource()
{
    realPipelineSource = NULL;
}


// ****************************************************************************
//  Method: avtInlinePipelineSource::GetFullDataRequest
//
//  Purpose:
//      Gets the full data specification from the real terminating source.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2001
//
// ****************************************************************************

avtDataRequest_p
avtInlinePipelineSource::GetFullDataRequest(void)
{
    return realPipelineSource->GetFullDataRequest();
}


// ****************************************************************************
//  Method: avtInlinePipelineSource::FetchMeshAuxiliaryData
//
//  Purpose:
//      Calls the FetchMeshAuxiliaryData routine of the real pipeline source.
//
//  Arguments:
//      dataType  The type of data about the mesh (extents, etc).
//      args      The arguments describing the data.
//      spec      The data we are interested in.
//      output    The requested data as a void *.
//
//  Programmer:   Hank Childs
//  Creation:     September 11, 2001
//
// ****************************************************************************

void
avtInlinePipelineSource::FetchMeshAuxiliaryData(const char *dataType,
                  void *args, avtDataRequest_p spec, VoidRefList &output)
{
    realPipelineSource->FetchMeshAuxiliaryData(dataType, args, spec, output);
}


// ****************************************************************************
//  Method: avtInlinePipelineSource::FetchVariableAuxiliaryData
//
//  Purpose:
//      Calls the FetchVariableAuxiliaryData routine of the real pipeline
//      source.
//
//  Arguments:
//      dataType  The type of data about the variable (extents, etc).
//      args      The arguments describing the data.
//      spec      The data we are interested in.
//      output    The requested data as a void *.
//
//  Programmer:   Hank Childs
//  Creation:     September 11, 2001
//
// ****************************************************************************

void
avtInlinePipelineSource::FetchVariableAuxiliaryData(const char *dataType,
                  void *args, avtDataRequest_p spec, VoidRefList &output)
{
    realPipelineSource->FetchVariableAuxiliaryData(dataType, args,spec,output);
}


// ****************************************************************************
//  Method: avtInlinePipelineSource::FetchMaterialAuxiliaryData
//
//  Purpose:
//      Calls the FetchMaterialAuxiliaryData routine of the real pipeline
//      source.
//
//  Arguments:
//      dataType  The type of data about the material.
//      args      The arguments describing the data.
//      spec      The data we are interested in.
//      output    The requested data as a void *.
//
//  Programmer:   Hank Childs
//  Creation:     September 11, 2001
//
// ****************************************************************************

void
avtInlinePipelineSource::FetchMaterialAuxiliaryData(const char *dataType,
                  void *args, avtDataRequest_p spec, VoidRefList &output)
{
    realPipelineSource->FetchMaterialAuxiliaryData(dataType, args,spec,output);
}


// ****************************************************************************
//  Method: avtInlinePipelineSource::FetchSpeciesAuxiliaryData
//
//  Purpose:
//      Calls the FetchSpeciesAuxiliaryData routine of the real pipeline
//      source.
//
//  Arguments:
//      dataType  The type of data about the species.
//      args      The arguments describing the data.
//      spec      The data we are interested in.
//      output    The requested data as a void *.
//
//  Programmer:   Jeremy Meredith
//  Creation:     June  8, 2004
//
// ****************************************************************************

void
avtInlinePipelineSource::FetchSpeciesAuxiliaryData(const char *dataType,
                  void *args, avtDataRequest_p spec, VoidRefList &output)
{
    realPipelineSource->FetchSpeciesAuxiliaryData(dataType, args,spec,output);
}


