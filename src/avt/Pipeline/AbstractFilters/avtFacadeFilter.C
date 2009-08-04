/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                             avtFacadeFilter.C                             //
// ************************************************************************* //

#include <avtFacadeFilter.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtFacadeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtFacadeFilter::avtFacadeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtFacadeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtFacadeFilter::~avtFacadeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtFacadeFilter::GetInput
//
//  Purpose:
//      Gets the input for this filter.  Since this is a facade, it goes to the
//      first input in the pipeline and gets its filter.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 14:55:40 PDT 2005
//    Modify method of accessing facaded filters to meet new interface.
//
// ****************************************************************************

avtDataObject_p
avtFacadeFilter::GetInput(void)
{
    return GetIthFacadedFilter(0)->GetInput();
}

// ****************************************************************************
//  Method: avtFacadeFilter::GetInput
//
//  Purpose:
//      Gets the input for this filter.  Since this is a facade, it goes to the
//      first input in the pipeline and gets its filter.
//
//  Programmer: Tom Fogal
//  Creation:   June 23, 2009
//
// ****************************************************************************

const avtDataObject_p
avtFacadeFilter::GetInput(void) const
{
    return GetIthFacadedFilter(0)->GetInput();
}


// ****************************************************************************
//  Method: avtFacadeFilter::GetOutput
//
//  Purpose:
//      Gets the output for this filter.  Since this is a facade, it goes to 
//      the last input in the pipeline and gets its filter.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 14:55:40 PDT 2005
//    Modify method of accessing facaded filters to meet new interface.
//
// ****************************************************************************

avtDataObject_p
avtFacadeFilter::GetOutput(void)
{
    return GetIthFacadedFilter(GetNumberOfFacadedFilters()-1)->GetOutput();
}


// ****************************************************************************
//  Method: avtFacadeFilter::GetOriginatingSource
//
//  Purpose:
//      Gets the terminating source of the pipeline.  Since this is a facade,
//      it goes to the first filter in the pipeline and gets its terminating
//      source.  This can probably be achieved by using any filter in the
//      pipeline.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 14:55:40 PDT 2005
//    Modify method of accessing facaded filters to meet new interface.
//
// ****************************************************************************

avtOriginatingSource *
avtFacadeFilter::GetOriginatingSource(void)
{
    return GetIthFacadedFilter(GetNumberOfFacadedFilters()-1)->
                                                        GetOriginatingSource();
}


// ****************************************************************************
//  Method: avtFacadeFilter::GetQueryableSource
//
//  Purpose:
//      Gets the queryable source of the pipeline.  Since this is a facade,
//      it goes to the first filter in the pipeline and gets its queryable
//      source.  This can probably be achieved by using any filter in the
//      pipeline.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 14:55:40 PDT 2005
//    Modify method of accessing facaded filters to meet new interface.
//
// ****************************************************************************

avtQueryableSource *
avtFacadeFilter::GetQueryableSource(void)
{
    return GetIthFacadedFilter(GetNumberOfFacadedFilters()-1)->
                                                          GetQueryableSource();
}


// ****************************************************************************
//  Method: avtFacadeFilter::Update
//
//  Purpose:
//      Responds as if it is a normal filter doing an Update.  This can be
//      achieved by making the last filter do an Update.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 14:55:40 PDT 2005
//    Modify method of accessing facaded filters to meet new interface.
//
// ****************************************************************************

bool
avtFacadeFilter::Update(avtContract_p spec)
{
    return GetIthFacadedFilter(GetNumberOfFacadedFilters()-1)->Update(spec);
}


// ****************************************************************************
//  Method: avtFacadeFilter::SetTypedInput
//
//  Purpose:
//      Sets the input of the facade.  This passes the input on to the first
//      filter in the pipeline.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 14:55:40 PDT 2005
//    Modify method of accessing facaded filters to meet new interface.
//
// ****************************************************************************

void
avtFacadeFilter::SetTypedInput(avtDataObject_p input)
{
    GetIthFacadedFilter(0)->SetInput(input);
}


// ****************************************************************************
//  Method: avtFacadeFilter::Execute
//
//  Purpose:
//      This must be defined so that the derived types are concrete.  Execute
//      should never be called, since the Update/Execute cycle should go
//      through the filters that are facaded.  If it is called, an exception
//      should be called.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

void
avtFacadeFilter::Execute(void)
{
    EXCEPTION0(ImproperUseException);
}


// ****************************************************************************
//  Method: avtFacadeFilter::ModifyContract
//
//  Purpose:
//      Calls perform restriction on all of the filters it is facading.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2005
//
// ****************************************************************************

avtContract_p
avtFacadeFilter::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = spec;

    for (int i = GetNumberOfFacadedFilters()-1 ; i >= 0 ; i--)
        rv = GetIthFacadedFilter(i)->ModifyContract(rv);

    return rv;
}


// ****************************************************************************
//  Method: avtFacadeFilter::ReleaseData
//
//  Purpose:
//      Calls release data on all of the filters it is facading.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2005
//
// ****************************************************************************

void
avtFacadeFilter::ReleaseData(void)
{
    for (int i = 0 ; i < GetNumberOfFacadedFilters() ; i++)
        GetIthFacadedFilter(i)->ReleaseData();
}


// ****************************************************************************
//  Method: avtFacadeFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Calls refashion data object info on each of the facaded filters.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2005
//
// ****************************************************************************

void
avtFacadeFilter::UpdateDataObjectInfo(void)
{
    for (int i = 0 ; i < GetNumberOfFacadedFilters() ; i++)
        GetIthFacadedFilter(i)->UpdateDataObjectInfo();
}


