/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                          avtExpressionStreamer.C                          //
// ************************************************************************* //

#include <avtExpressionStreamer.h>

#include <math.h>
#include <float.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

#include <avtExtents.h>
#include <avtCommonDataFunctions.h>

#include <avtExprNode.h>

#include <DebugStream.h>
#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtExpressionStreamer constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtExpressionStreamer::avtExpressionStreamer()
{
    ;
}


// ****************************************************************************
//  Method: avtExpressionStreamer destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtExpressionStreamer::~avtExpressionStreamer()
{
    ;
}


// ****************************************************************************
//  Method: avtExpressionStreamer::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//
//  Returns:      The output dataset.
//
//  Programmer:   Hank Childs
//  Creation:     June 7, 2002
//
//  Modifications:
//
//    Sean Ahern, Tue Jun 11 16:35:48 PDT 2002
//    Modified to work with both point and cell data.  Note, this doesn't
//    yet handle filters that convert from point to cell, or vice versa.
//
//    Akira Haddox, Thu Aug 15 16:41:44 PDT 2002
//    Modified to work with vector expressions.
//
//    Akira Haddox, Mon Aug 19 16:39:07 PDT 2002 
//    Modified to use IsPointVariable to determine variable type.
//
//    Hank Childs, Mon Sep  2 18:53:40 PDT 2002
//    Do not use enhanced connectivity points when calculating ranges.
//
//    Hank Childs, Tue Nov 19 08:50:42 PST 2002
//    Use the dimension of the returned type instead of the virtual function
//    call to decide if the variable is a vector.  Did not remove virtual
//    function to determine this, since it may be used before execution (by the
//    GUI, for example to decide if something is a vector,scalar, etc).
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002
//    Use NewInstance instead of MakeObject, new vtk api. 
//
//    Hank Childs, Thu Jan 23 11:19:14 PST 2003
//    Set only the cumulative extents.
//
//    Hank Childs, Mon Sep 22 08:34:14 PDT 2003
//    Add support for tensors.
//
//    Hank Childs, Thu Jan 29 16:22:06 PST 2004
//    Fix some odd centering cases that can come up when a variable can get
//    misidentified.
//
//    Hank Childs, Wed Feb 25 14:48:31 PST 2004
//    Make sure that the extents get associated with the correct variable.
//
//    Kathleen Bonnell, Thu Mar 11 11:24:40 PST 2004 
//    DataExtents now always only 2 components.  Allow for Tensor var's range
//    to be computed. 
//
//    Hank Childs, Fri Aug 27 16:54:45 PDT 2004
//    Rename ghost data arrays.
//
//    Hank Childs, Mon Dec 27 10:28:51 PST 2004
//    Moved from avtExpressionFilter to avtExpressionStreamer.
//
//    Hank Childs, Mon Jan  3 09:15:40 PST 2005
//    Check to see if the variable already exists.  If so, no need to 
//    re-execute.
//
//    Hank Childs, Thu Jul 28 09:06:00 PDT 2005
//    Fix UMR that can occur with array variables.
//
//    Hank Childs, Tue Aug 30 13:46:19 PDT 2005
//    Move code for calculating cumulative extents to avtExpressionFilter.
//
//    Hank Childs, Tue Nov 15 11:41:28 PST 2005
//    Make a comment to debug logs when we can "bypass".
//
//    Hank Childs, Sun Feb 19 10:26:49 PST 2006
//    Fix a crash.
//
//    Hank Childs, Fri Jun  9 14:18:11 PDT 2006
//    Remove unused variable.
//
// ****************************************************************************

vtkDataSet *
avtExpressionStreamer::ExecuteData(vtkDataSet *in_ds, int index,
                                 std::string label)
{
    //
    // Sometimes we are asked to calculate a variable twice.  The easiest way
    // to catch this is to see if we already have the requested variable and
    // not re-derive it if we do.
    //
    vtkDataArray *dat = NULL;
    dat = in_ds->GetPointData()->GetArray(outputVariableName);
    if (dat == NULL)
        dat = in_ds->GetCellData()->GetArray(outputVariableName);
    if (dat != NULL)
    {
        debug1 << "NOTE: variable " << outputVariableName 
               << " already exists and it is not being recalculated." << endl;
        dat->Register(NULL);  // At the end of the routine, we will free this.
    }

    //
    // Start off by having the derived type calculate the derived variable.
    //
    currentDomainsLabel = label;
    currentDomainsIndex = index;
    if (dat == NULL)
    {
        dat = DeriveVariable(in_ds);
        if (dat == NULL)
        {
            EXCEPTION1(ExpressionException, "an unknown error occurred while " 
                  "trying to calculate your expression.  Please contact a "
                  "VisIt developer.");
        }
        dat->SetName(outputVariableName);
    }

    int vardim = dat->GetNumberOfComponents();

    //
    // Now make a copy of the input and add the derived variable as its output.
    //
    vtkDataSet *rv = (vtkDataSet *) in_ds->NewInstance();
    rv->ShallowCopy(in_ds);
    int npts   = rv->GetNumberOfPoints();
    int ncells = rv->GetNumberOfCells();
    int ntups  = dat->GetNumberOfTuples();
    bool isPoint = false;
    if ((ntups == ncells) && (ntups != npts))
    {
        isPoint = false;
    }
    else if ((ntups == npts) && (ntups != ncells))
    {
        isPoint = true;
    }
    else if ((ntups == npts) && (ntups == ncells))
    {
        isPoint = IsPointVariable();
    }
    else
    {
        debug1 << "Number of tuples cannot be point or cell variable."
               << endl;
        debug1 << "Var = " << dat->GetName() << endl;
        debug1 << "Ntuples = " << ntups << endl;
        debug1 << "Ncells = " << ncells << endl;
        debug1 << "Npts = " << npts << endl;
        return rv;
    }

    if (isPoint)
    {
        rv->GetPointData()->AddArray(dat);
        if (vardim == 1)
            rv->GetPointData()->SetActiveScalars(outputVariableName);
        else if (vardim == 3)
            rv->GetPointData()->SetActiveVectors(outputVariableName);
        else if (vardim == 9)
            rv->GetPointData()->SetActiveTensors(outputVariableName);
    }
    else
    {
        rv->GetCellData()->AddArray(dat);
        if (vardim == 1)
            rv->GetCellData()->SetActiveScalars(outputVariableName);
        else if (vardim == 3)
            rv->GetCellData()->SetActiveVectors(outputVariableName);
        else if (vardim == 9)
            rv->GetCellData()->SetActiveTensors(outputVariableName);
    }

    //
    // Make sure that we don't have any memory leaks.
    //
    dat->Delete();
    ManageMemory(rv);
    rv->Delete();

    return rv;
}


