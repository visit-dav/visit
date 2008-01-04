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
//                             avtDO_REPL_STRING.C                           //
// ************************************************************************* //

#include <avtDO_REPL_STRING.h>

#include <avtDO_REPL_STRINGSource.h>


// ****************************************************************************
//  Method: avtDO_REPL_STRING constructor
//
//  Arguments:
//      src     An data object source that is the upstream object.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRING::avtDO_REPL_STRING(avtDataObjectSource *src)
    : avtDataObject(src)
{
    ;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDO_REPL_STRING::~avtDO_REPL_STRING()
{
    ;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING::InstantiateWriter
//
//  Purpose:
//      Instantiates a writer that is appropriate for an avtDO_REPL_STRING.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

/*
avtDataObjectWriter *
avtDO_REPL_STRING::InstantiateWriter(void)
{
    return new avtDO_REPL_STRINGWriter;
}
 */


// ****************************************************************************
//  Method: avtDO_REPL_STRING::ReleaseData
//
//  Purpose:
//      Free the data associated with this DO_REPL_STRING.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

void
avtDO_REPL_STRING::ReleaseData(void)
{
    YOU MUST DEFINE WHAT IT MEANS TO RELEASE DATA FOR YOUR DATA OBJECT.
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING::Instance
//
//  Purpose:
//      Creates an instance of an avtDO_REPL_STRING.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

avtDataObject *
avtDO_REPL_STRING::Instance(void)
{
    avtDataObjectSource *src = NULL;
    avtDO_REPL_STRING *new_do = new avtDO_REPL_STRING(src);
    return new_do;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING::DerivedCopy
//
//  Purpose:
//      Copy over the image.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

void
avtDO_REPL_STRING::DerivedCopy(avtDataObject *dob)
{
    avtDO_REPL_STRING *typed_dob = (avtDO_REPL_STRING *) dob;
    YOU MUST IMPLEMENT HOW TO COPY THE DATA MEMBERS FROM typed_dob INTO
    "this" OBJECT.
    AS IN:
    data_member1 = typed_dob->data_member1;
    ...
    data_memberN = typed_dob->data_memberN;
}


// ****************************************************************************
//  Method: avtDO_REPL_STRING::GetNumberOfCells
//
//  Purpose:
//      Estimates the number of cells needed to render this object.  This is
//      used for determining if VisIt should go into its scalable rendering
//      mode.
//
//  Programmer: AUTHOR_REPL_STRING
//  Creation:   DATE_REPL_STRING
//
// ****************************************************************************

int
avtDO_REPL_STRING::GetNumberOfCells(bool onlyWantPolygonCount) const
{
   YOU MUST IMPLEMENT THIS.  IF YOU WILL NEVER BE RENDERING YOUR DATA OBJECT
   (BECAUSE YOU WILL ALWAYS BE CONVERTING TO AN avtDataset), THEN JUST RETURN 0.
}



