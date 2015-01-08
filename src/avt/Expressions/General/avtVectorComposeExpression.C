/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                     avtVectorComposeExpression.C                          //
// ************************************************************************* //

#include <avtVectorComposeExpression.h>
#include <avtVariableCache.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkInformation.h>
#include <vtkInformationDoubleVectorKey.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorComposeExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorComposeExpression::avtVectorComposeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorComposeExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorComposeExpression::~avtVectorComposeExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorComposeExpression::GetVariableDimension
//
//  Purpose:
//      Does its best to guess at the variable dimension will be after
//      decomposing a "vector".
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//   Modifications:
//
//    Hank Childs, Thu Feb 26 09:00:06 PST 2004
//    Account for multiple variables.
//
//    Hank Childs, Thu May  6 10:49:32 PDT 2004
//    Make sure that we are getting the dimension for the active variable.
//
// ****************************************************************************

int
avtVectorComposeExpression::GetVariableDimension(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (activeVariable == NULL)
        return 3;
    if (!atts.ValidVariable(activeVariable))
        return 3;

    int inDim = atts.GetVariableDimension(activeVariable);
    if (inDim == 1)
        return 3;
    else if (inDim == 3)
        return 9;

    // Who knows?!?
    return 3;
}


// ****************************************************************************
//  Method: avtVectorComposeExpression::DeriveVariable
//
//  Purpose:
//      Creates a vector variable from components.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Sean Ahern
//  Creation:     Thu Mar  6 19:40:32 America/Los_Angeles 2003
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 11:01:34 PDT 2003
//    Add better support for 2D vectors.
//
//    Hank Childs, Fri Sep 19 13:46:13 PDT 2003
//    Added support for tensors, symmetric tensors.
//
//    Hank Childs, Sun Jan 13 20:11:35 PST 2008
//    Add support for singleton constants.
//
//    Mark C. Miller, Wed Feb 27 10:14:01 PST 2013
//    Enhanced error message about not knowing how to compose 3 Vector for 2D
//    domain to address confusion in creating color vectors.
//
//    Alexander Pletzer, Wed Dec  4 16:40:02 MST 2013
//    Added support for face and edge data
// ****************************************************************************
vtkDataArray *
avtVectorComposeExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    size_t numinputs = varnames.size();

    bool twoDVector = 
            (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2);

    //
    // Our first operand is in the active variable.  We don't know if it's
    // point data or cell data, so check which one is non-NULL.
    //
    vtkDataArray *cell_data1 = in_ds->GetCellData()->GetArray(varnames[0]);
    vtkDataArray *point_data1 = in_ds->GetPointData()->GetArray(varnames[0]);
    vtkDataArray *data1 = NULL, *data2 = NULL, *data3 = NULL;

    avtCentering centering;
    if (cell_data1 != NULL)
    {
        data1 = cell_data1;
        centering = AVT_ZONECENT;
    }
    else
    {
        data1 = point_data1;
        centering = AVT_NODECENT;
    }

    // Get the second variable.
    if (centering == AVT_ZONECENT)
        data2 = in_ds->GetCellData()->GetArray(varnames[1]);
    else
        data2 = in_ds->GetPointData()->GetArray(varnames[1]);

    if (data2 == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first two variables have different centering.");
    }

    if (numinputs == 3)
    {
        // Get the third variable.
        if (centering == AVT_ZONECENT)
            data3 = in_ds->GetCellData()->GetArray(varnames[2]);
        else
            data3 = in_ds->GetPointData()->GetArray(varnames[2]);
    
        if (data3 == NULL)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                   "The first and third variables have different centering.");
        }
    }

    // Build the vector out of components. Face and edge vectors are expected
    // to have a stagger for each component. 
    int isFaceCount = 0;
    int isEdgeCount = 0;

    std::vector<avtVector> offsets(3);
    vtkInformation* data1Info = data1->GetInformation();
    if ( data1Info->Has(avtVariableCache::STAGGER()) ) 
    {
        const char *stagger = data1Info->Get(avtVariableCache::STAGGER());
        if(strcmp(stagger, "face") == 0) isFaceCount++;
        if(strcmp(stagger, "edge") == 0) isEdgeCount++;
    }
    else if ( data1Info->Has(avtVariableCache::OFFSET_3()) ) 
    {
        double* vals = data1Info->Get(avtVariableCache::OFFSET_3());
        offsets[0].x = vals[0];
        offsets[0].y = vals[1];
        offsets[0].z = vals[2];
    }

    vtkInformation* data2Info =data2->GetInformation();
    if ( data2Info->Has(avtVariableCache::STAGGER()) ) 
    {
        const char *stagger = data2Info->Get(avtVariableCache::STAGGER());
        if(strcmp(stagger, "face") == 0) isFaceCount++;
        if(strcmp(stagger, "edge") == 0) isEdgeCount++;
    }
    else if ( data2Info->Has(avtVariableCache::OFFSET_3()) ) 
    {
        double* vals = data2Info->Get(avtVariableCache::OFFSET_3());
        offsets[1].x = vals[0];
        offsets[1].y = vals[1];
        offsets[1].z = vals[2];
    }
  
    if (numinputs == 3) {
      vtkInformation* data3Info = data3->GetInformation();
      if ( data3Info->Has(avtVariableCache::STAGGER()) ) 
      {
          const char *stagger = data3Info->Get(avtVariableCache::STAGGER());
          if(strcmp(stagger, "face") == 0) isFaceCount++;
          if(strcmp(stagger, "edge") == 0) isEdgeCount++;
      }
      else if ( data3Info->Has(avtVariableCache::OFFSET_3()) ) 
      {
          double* vals = data3Info->Get(avtVariableCache::OFFSET_3());
          offsets[2].x = vals[0];
          offsets[2].y = vals[1];
          offsets[2].z = vals[2];
      }
    }

    bool isFace = (isFaceCount == numinputs);
    bool isEdge = (isEdgeCount == numinputs);


    vtkIdType nvals1 = data1->GetNumberOfTuples();
    vtkIdType nvals2 = data2->GetNumberOfTuples();
    vtkIdType nvals3 = 1;
    if (numinputs == 3)
        nvals3 = data3->GetNumberOfTuples();
    
    vtkIdType nvals = nvals1;
    if (nvals == 1)
        nvals  = nvals2;
    if (nvals == 1 && numinputs == 3)
        nvals  = nvals3;

    vtkDataArray *dv = data1->NewInstance();
    if (twoDVector)
    {
        if (numinputs == 2)
        {
            if (data1->GetNumberOfComponents() == 1 &&
                data2->GetNumberOfComponents() == 1)
            {
                //
                // Standard vector case.
                //
                dv->SetNumberOfComponents(3);  // VTK doesn't like 2.
                dv->SetNumberOfTuples(nvals);

                for (vtkIdType i = 0 ; i < nvals ; i++)
                {
                    double val1 = data1->GetTuple1((nvals1>1 ? i : 0));
                    double val2 = data2->GetTuple1((nvals2>1 ? i : 0));
                    dv->SetTuple3(i, val1, val2, 0.);
                }
            }
            else if ((data1->GetNumberOfComponents() == 3) &&
                     (data2->GetNumberOfComponents() == 3))
            {
                //
                // 2D tensor.
                //
                dv->SetNumberOfComponents(9); 
                dv->SetNumberOfTuples(nvals);
                
                for (vtkIdType i = 0 ; i < nvals ; i++)
                {
                    double vals[9];
                    vals[0] = data1->GetComponent((nvals1>1 ? i : 0), 0);
                    vals[1] = data1->GetComponent((nvals2>1 ? i : 0), 1);
                    vals[2] = 0.;
                    vals[3] = data2->GetComponent((nvals1>1 ? i : 0), 0);
                    vals[4] = data2->GetComponent((nvals2>1 ? i : 0), 1);
                    vals[5] = 0.;
                    vals[6] = 0.;
                    vals[7] = 0.;
                    vals[8] = 0.;
                    dv->SetTuple(i, vals);
                }
            }
            else
            {
                char str[1024];
                sprintf(str, "Do not know how to assemble arrays of %d and "
                             "%d into a vector or tensor.", 
                              data1->GetNumberOfComponents(),
                              data2->GetNumberOfComponents());
                EXCEPTION2(ExpressionException, outputVariableName, str);
            }
        }
        else if (numinputs == 3)
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                       "I don't know how to compose "
                       "3 variables to make a field for a 2D dataset. If you are by chance "
                       "attempting to constuct an rgb color vector, try using the special "
                       "expressions designed for this purpose, 'color' or 'color4', under "
                       "Vector expressions submenu.");
 
        }
    }
    else
    {
        if (numinputs == 3)
        {
            if (data1->GetNumberOfComponents() == 1 && 
                data2->GetNumberOfComponents() == 1 && 
                data3->GetNumberOfComponents() == 1)
            {
                //
                // This is your everyday 3D vector combination.
                //
                dv->SetNumberOfComponents(3); 
                dv->SetNumberOfTuples(nvals);
                
                for (vtkIdType i = 0 ; i < nvals ; i++)
                {
                    double val1 = data1->GetTuple1((nvals1>1 ? i : 0));
                    double val2 = data2->GetTuple1((nvals2>1 ? i : 0));
                    double val3 = data3->GetTuple1((nvals3>1 ? i : 0));
                    dv->SetTuple3(i, val1, val2, val3);
                }
            }
            else if (data1->GetNumberOfComponents() == 3 &&
                     data2->GetNumberOfComponents() == 3 &&
                     data3->GetNumberOfComponents() == 3)
            {
                //
                // This is a 3D tensor.  Interpret it as:
                // Data1 = XX, XY, XZ
                // Data2 = YX, YY, YZ
                // Data3 = ZX, ZY, ZZ
                //
                dv->SetNumberOfComponents(9); 
                dv->SetNumberOfTuples(nvals);
                
                for (vtkIdType i = 0 ; i < nvals ; i++)
                {
                    double entry[9];
                    data1->GetTuple((nvals1>1 ? i : 0), entry);
                    data2->GetTuple((nvals2>1 ? i : 0), entry+3);
                    data3->GetTuple((nvals3>1 ? i : 0), entry+6);
                    dv->SetTuple(i, entry);
                }
            }
            else
            {
                EXCEPTION2(ExpressionException, outputVariableName, 
                           "The only interpretation "
                           "VisIt can make of 3 variables for a 3D dataset is "
                           "a vector or a tensor.  But these inputs don't have"
                           " the right number of components to make either.");
            }
        }
        else 
        {
            EXCEPTION2(ExpressionException, outputVariableName, 
                        "You must specify three vectors "
                        "to compose a field for a 3D dataset.");
        }
    }

    //If offset information existed on the input dataset, set it on the output
    vtkInformation* dvInfo = dv->GetInformation();
    if (isFace) {
      dvInfo->Set(avtVariableCache::STAGGER(), "face");
    }
    else if (isEdge) {
      dvInfo->Set(avtVariableCache::STAGGER(), "edge");
    }
    else if ((offsets[0].x != 0) || (offsets[0].y != 0) || (offsets[0].z != 0) ||
        (offsets[1].x != 0) || (offsets[1].y != 0) || (offsets[1].z != 0) ||
        (offsets[2].x != 0) || (offsets[2].y != 0) || (offsets[2].z != 0)) {
      
      dvInfo->Set(avtVariableCache::OFFSET_3_COMPONENT_0(), offsets[0].x, offsets[0].y, offsets[0].z);
      dvInfo->Set(avtVariableCache::OFFSET_3_COMPONENT_1(), offsets[1].x, offsets[1].y, offsets[1].z);
      dvInfo->Set(avtVariableCache::OFFSET_3_COMPONENT_2(), offsets[2].x, offsets[2].y, offsets[2].z);
    }

    return dv;
}
