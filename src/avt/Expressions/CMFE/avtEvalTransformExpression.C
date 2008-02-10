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
//                          avtEvalTransformExpression.h                     //
// ************************************************************************* //

#include <avtEvalTransformExpression.h>

#include <vtkMatrix4x4.h>

#include <avtCustomTransform.h>
#include <avtSourceFromAVTDataset.h>


// ****************************************************************************
//  Method: avtEvalTransformExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtEvalTransformExpression::avtEvalTransformExpression()
{
}


// ****************************************************************************
//  Method: avtEvalTransformExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtEvalTransformExpression::~avtEvalTransformExpression()
{
}


// ****************************************************************************
//  Method: avtEvalTransformExpression::TransformData
//
//  Purpose:
//      Transforms the data based on the input parameters.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtDataObject_p
avtEvalTransformExpression::TransformData(avtDataObject_p input)
{
    vtkMatrix4x4 *mat = vtkMatrix4x4::New();
    mat->SetElement(0, 0, inputParameters[0]);
    mat->SetElement(0, 1, inputParameters[1]);
    mat->SetElement(0, 2, inputParameters[2]);
    mat->SetElement(1, 0, inputParameters[3]);
    mat->SetElement(1, 1, inputParameters[4]);
    mat->SetElement(1, 2, inputParameters[5]);
    mat->SetElement(2, 0, inputParameters[6]);
    mat->SetElement(2, 1, inputParameters[7]);
    mat->SetElement(2, 2, inputParameters[8]);

    avtDataset_p ds;
    CopyTo(ds, input);
    avtSourceFromAVTDataset termsrc(ds);

    avtCustomTransform transform;
    transform.SetMatrix(mat);
    transform.SetInput(termsrc.GetOutput());

    avtDataObject_p output = transform.GetOutput();
    output->Update(GetGeneralContract());

    mat->Delete();

    return output;
}


