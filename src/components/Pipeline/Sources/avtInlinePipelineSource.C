/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
    realPipelineSource = dob->GetTerminatingSource();
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
//  Method: avtInlinePipelineSource::GetFullDataSpecification
//
//  Purpose:
//      Gets the full data specification from the real terminating source.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2001
//
// ****************************************************************************

avtDataSpecification_p
avtInlinePipelineSource::GetFullDataSpecification(void)
{
    return realPipelineSource->GetFullDataSpecification();
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
                  void *args, avtDataSpecification_p spec, VoidRefList &output)
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
                  void *args, avtDataSpecification_p spec, VoidRefList &output)
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
                  void *args, avtDataSpecification_p spec, VoidRefList &output)
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
                  void *args, avtDataSpecification_p spec, VoidRefList &output)
{
    realPipelineSource->FetchSpeciesAuxiliaryData(dataType, args,spec,output);
}


