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
//                       avtAverageMeanCurvatureQuery.C                      //
// ************************************************************************* //

#include <avtAverageMeanCurvatureQuery.h>

#include <avtCurvatureExpression.h>

#include <DebugStream.h>
#include <NonQueryableInputException.h>


using     std::string;


// ****************************************************************************
//  Method: avtAverageMeanCurvatureQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtAverageMeanCurvatureQuery::avtAverageMeanCurvatureQuery() 
    : avtWeightedVariableSummationQuery()
{
    curvature = new avtCurvatureExpression;
    curvature->DoGaussCurvature(false);
    curvature->SetOutputVariableName("curvature");
}


// ****************************************************************************
//  Method: avtAverageMeanCurvatureQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtAverageMeanCurvatureQuery::~avtAverageMeanCurvatureQuery()
{
    delete curvature;
}


// ****************************************************************************
//  Method: avtAverageMeanCurvatureQuery::CreateVariable
//
//  Purpose:
//     Creates the variable for the summation.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtDataObject_p
avtAverageMeanCurvatureQuery::CreateVariable(avtDataObject_p inData)
{
    curvature->SetInput(inData);
    return curvature->GetOutput();
}


// ****************************************************************************
//  Method: avtAverageMeanCurvatureQuery::VerifyInput
//
//  Purpose:
//      Make sure we are operating on a surface.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

void
avtAverageMeanCurvatureQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtWeightedVariableSummationQuery::VerifyInput();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 2)
    {
        EXCEPTION1(NonQueryableInputException, "The average mean curvature "
                   "query can only operate on surfaces.");
    }
}

