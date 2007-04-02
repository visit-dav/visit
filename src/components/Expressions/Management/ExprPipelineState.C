/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         ExprPipelineState.C                               //
// ************************************************************************* //

#include <ExprPipelineState.h>
#include <vector>
#include <avtExpressionFilter.h>

using std::vector;
using std::string;


// ****************************************************************************
//  Method: ExprPipelineState constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
//  Modifications:
//
// ****************************************************************************

ExprPipelineState::ExprPipelineState() 
{
    dataObject=NULL;
}


// ****************************************************************************
//  Method: ExprPipelineState destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Sean Ahearn 
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 22 14:55:09 PDT 2004
//    Delete the filters.
//
// ****************************************************************************

ExprPipelineState::~ExprPipelineState() 
{
    for (int i = 0; i < filters.size(); ++i)
    {
        delete filters[i];
    }
}


// ****************************************************************************
//  Method: ExprPipelineState::PopName
//
//  Purpose:
//    Removes a name from the back of the name stack and returns it. 
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
//  Modifications:
//
// ****************************************************************************

string     
ExprPipelineState::PopName()
{
    string ret = name_stack.back();
    name_stack.pop_back();
    return ret;
}


// ****************************************************************************
//  Method: ExprPipelineState::ReleaseData
//
//  Purpose:
//    Releases the data associated the expression filters.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 22, 2004 
//
//  Modifications:
//
// ****************************************************************************

void 
ExprPipelineState::ReleaseData()
{
    for (int i = 0; i < filters.size(); ++i)
        filters[i]->ReleaseData();
}


// ****************************************************************************
//  Method: ExprPipelineState::Clear
//
//  Purpose:
//      Clears out the pipeline state.
//
//  Programmer: Hank Childs
//  Creation:   December 31, 2004
//
// ****************************************************************************

void
ExprPipelineState::Clear()
{
    for (int i = 0; i < filters.size(); ++i)
        delete filters[i];
    filters.clear();
    name_stack.clear();
    dataObject = NULL;
}


