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
//                              avtFacadeFilter.h                            //
// ************************************************************************* //

#ifndef AVT_FACADE_FILTER
#define AVT_FACADE_FILTER

#include <pipeline_exports.h>

#include <avtFilter.h>

// ****************************************************************************
//  Class: avtFacadeFilter
//
//  Purpose:
//      Sometimes a filter is wrapped around another filter or a pipeline of
//      filters.  This can be so that it does a conversion of the attributes or
//      so that it can meet some interface that the original filter does not.
//      This class provides some of the dirty work so that pipelines can be 
//      well-maintained without getting too complicated.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Tue Jun  7 14:48:49 PDT 2005
//    Add methods to access all facaded filters, as well as populating common
//    avtFilter methods (release data, perform restriction).
//
// ****************************************************************************

class PIPELINE_API avtFacadeFilter : virtual public avtFilter
{
  public:
                                   avtFacadeFilter();
    virtual                       ~avtFacadeFilter();

    virtual avtDataObject_p        GetInput(void);
    virtual avtDataObject_p        GetOutput(void);

    virtual avtOriginatingSource  *GetOriginatingSource(void);
    virtual avtQueryableSource    *GetQueryableSource(void);
    virtual bool                   Update(avtContract_p);
    virtual void                   ReleaseData(void);

  protected:
    virtual void                   SetTypedInput(avtDataObject_p);

    virtual int                    GetNumberOfFacadedFilters(void) = 0;
    virtual avtFilter             *GetIthFacadedFilter(int) = 0;

    virtual void                   Execute(void);

    virtual avtContract_p ModifyContract(
                                                  avtContract_p);
    virtual void                   UpdateDataObjectInfo(void);
};


#endif



