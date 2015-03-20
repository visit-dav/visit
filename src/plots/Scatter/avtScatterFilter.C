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
//                              avtScatterFilter.C                           //
// ************************************************************************* //

#include <avtScatterFilter.h>
#include <avtParallel.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkPolyData.h>

#include <avtMetaData.h>
#include <avtExtents.h>
#include <avtIntervalTree.h>

#include <ImproperUseException.h>
#include <InvalidLimitsException.h>
#include <DebugStream.h>

//#include <visitstream.h>
//#include <avtDataAttributes.h>

#include <string>
#include <vector>

#define SCATTER_FLOAT_MIN -1e+37
#define SCATTER_FLOAT_MAX  1e+37

#define BEGIN_X_RANGE \
                bool xInRange = true; \
                if(d1->useMin) \
                    xInRange = (coord[0] >= d1min); \
                if(d1->useMax) \
                    xInRange &= (coord[0] <= d1max); \
                if(xInRange) {
#define END_X_RANGE }

#define BEGIN_Y_RANGE \
                bool yInRange = true; \
                if(d2->useMin) \
                    yInRange = (coord[1] >= d2min); \
                if(d2->useMax) \
                    yInRange &= (coord[1] <= d2max); \
                if(yInRange) { 

#define END_Y_RANGE }

#define BEGIN_Z_RANGE \
                bool zInRange = true; \
                if(d3->useMin) \
                    zInRange = (coord[2] >= d3min); \
                if(d3->useMax) \
                    zInRange &= (coord[2] <= d3max); \
                if(zInRange) { 

#define END_Z_RANGE }

// ****************************************************************************
//  Method: avtScatterFilter constructor
//
//  Arguments:
//
//  Programmer: Brad Whitlock 
//  Creation:   Tue Nov 2 22:36:23 PST 2004
//
//  Modifications:
//    Brad Whitlock, Mon Jul 18 11:07:35 PDT 2005
//    Added extents arrays and flags.
//
//    Cyrus Harrison, Thu Aug 19 13:33:53 PDT 2010
//    Removed unused code.
//
//    Kathleen Biagas, Thu Mar  1 14:49:50 MST 2012
//    Added keepNodeZone.
//
// ****************************************************************************

avtScatterFilter::avtScatterFilter(const ScatterAttributes &a)
    : avtDataTreeIterator(),atts(a)
{
    needXExtents = false;
    xExtents[0] = 0., xExtents[1] = 1.;

    needYExtents = false;
    yExtents[0] = 0., yExtents[1] = 1.;

    needZExtents = false;
    zExtents[0] = 0., zExtents[1] = 1.;

    needColorExtents = false;
    colorExtents[0] = 0., colorExtents[1] = 0.;

    keepNodeZone = false;
}


// ****************************************************************************
//  Method: avtScatterFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:36:23 PST 2004
//
//  Modifications:
//
// ****************************************************************************

avtScatterFilter::~avtScatterFilter()
{
}

// ****************************************************************************
// Method: avtScatterFilter::PreExecute
//
// Purpose: 
//   Executes before the ExecuteData method so we can get the x,y,z extents
//   for the variables that are involved in creating the plot.
//
// Note:       We use this method to calculate min/max values, if needed.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 18 11:08:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtScatterFilter::PreExecute(void)
{
    avtDataTreeIterator::PreExecute();

    const char *vars[5] = {0,0,0,0,0};
    PopulateNames(vars);

    if (needXExtents)
    {
        GetDataExtents(xExtents, vars[0]);
        needXExtents = false;
        debug1 << "avtScatterFilter::PreExecute: Calculated xExtents=" <<
             xExtents[0] << ", " << xExtents[1] << endl;
    }

    if (needYExtents)
    {
        GetDataExtents(yExtents, vars[1]);
        needYExtents = false;
        debug1 << "avtScatterFilter::PreExecute: Calculated yExtents=" <<
             yExtents[0] << ", " << yExtents[1] << endl;
    }

    if (needZExtents)
    {
        GetDataExtents(zExtents, vars[2]);
        needZExtents = false;
        debug1 << "avtScatterFilter::PreExecute: Calculated zExtents=" <<
             zExtents[0] << ", " << zExtents[1] << endl;
    }

    if (needColorExtents)
    {
        GetDataExtents(colorExtents, vars[3]);
        needColorExtents = false;
        debug1 << "avtScatterFilter::PreExecute: Calculated colorExtents=" <<
             colorExtents[0] << ", " << colorExtents[1] << endl;
    }

    thisProcsSpatialExtents = std::vector<double>(6,0.0);
}

// ****************************************************************************
//  Method: avtScatterFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the scatter filter.
//
//  Arguments:
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:36:23 PST 2004
//
//  Modifications:
//    Brad Whitlock, Tue Jul 19 10:40:18 PDT 2005
//    Moved some code to other routines and made the color variable always
//    be node-centered so the glyphers can handle it.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
//
//    Brad Whitlock, Fri Jul 16 14:24:25 PDT 2010
//    Work around curve variable centering being unknown.
//
//    Cyrus Harrison, Tue Aug 17 11:51:28 PDT 2010
//    Moved logic that modifies the output data atts to PostExecute.
//
//    Cyrus Harrison, Thu Aug 19 13:34:28 PDT 2010
//    Obtain var1 from atts.
//
//    Kathleen Biagas, Thu Mar  1 14:49:50 MST 2012
//    Keep track of original node numbers.
//
//    Eric Brugger, Tue Aug 19 11:13:44 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Kathleen Biagas, Fri Mar 20 11:44:28 PDT 2015
//    Make 'PointMeshFromVariables' a templated method to support double
//    precision.
//
// ****************************************************************************

avtDataRepresentation *
avtScatterFilter::ExecuteData(avtDataRepresentation *inDR)
{
    debug4 << "avtScatterFilter::ExecuteData" << endl;

    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();

    // Determine the name of the first variable.
    std::string var1Name(atts.GetVar1());

    // Determine the name of the second variable.
    std::string var2Name(atts.GetVar2());
    if(var2Name == "default")
        var2Name = var1Name;

    // Determine the name of the 3rd variable.
    std::string var3Name(atts.GetVar3());
    if(var3Name == "default")
        var3Name = var1Name;

    // Determine the name of the 4th variable.
    std::string var4Name(atts.GetVar4());
    if(var4Name == "default")
        var4Name = var1Name;

    //
    // Determine the centering for var1 and get its data array.
    //
    bool deleteArray2 = false, deleteArray3 = false, deleteArray4 = false;
    vtkDataArray *arr[4] = {0,0,0,0};
    avtCentering var1Centering = datts.GetCentering(var1Name.c_str());
    if(var1Centering == AVT_NODECENT)
        arr[0] = inDS->GetPointData()->GetArray(var1Name.c_str());
    else if(var1Centering == AVT_ZONECENT)
        arr[0] = inDS->GetCellData()->GetArray(var1Name.c_str());
    else
    {
        // Assume unknown centering.
        arr[0] = inDS->GetPointData()->GetArray(var1Name.c_str());
        if(arr[0] != 0)
            var1Centering = AVT_NODECENT;
        else
        {
            arr[0] = inDS->GetCellData()->GetArray(var1Name.c_str());
            var1Centering = AVT_ZONECENT;
        }
    }

    //
    // Get the data arrays for the secondary variables.
    //
    if(atts.GetVar2Role() != ScatterAttributes::None)
        arr[1] = GetDataArray(inDS, var2Name, var1Centering, deleteArray2);

    if(atts.GetVar3Role() != ScatterAttributes::None)
        arr[2] = GetDataArray(inDS, var3Name, var1Centering, deleteArray3);

    if(atts.GetVar4Role() != ScatterAttributes::None)
        arr[3] = GetDataArray(inDS, var4Name, var1Centering, deleteArray4);

    //
    // Put the input variables into the appropriate X,Y,Z coordinate role.
    //
    DataInput orderedArrays[5] = {
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.}};
    PopulateDataInputs(orderedArrays, arr);

    //
    // Make sure that X,Y roles have arrays to use for coordinates.
    //
    if(orderedArrays[0].data == 0 || orderedArrays[1].data == 0)
    {
        if(deleteArray2)
            arr[1]->Delete();
        if(deleteArray3)
            arr[2]->Delete();
        if(deleteArray4)
            arr[3]->Delete();
        EXCEPTION1(ImproperUseException, "At least two variables must play a "
            "role in creating coordinates and the X,Y coordinate roles must be "
            "assigned to a variable.");
    }

    bool createdData = false;
    vtkDataSet *outDS = 0;
    TRY
    {
        //
        // Create the point mesh from the input data arrays.
        //
        if ((orderedArrays[0].data &&
             orderedArrays[0].data->GetDataType() == VTK_DOUBLE)
         || (orderedArrays[1].data &&
             orderedArrays[1].data->GetDataType() == VTK_DOUBLE)
         || (orderedArrays[2].data &&
             orderedArrays[2].data->GetDataType() == VTK_DOUBLE))
        {
            outDS = PointMeshFromVariables<double>(&orderedArrays[0],
                &orderedArrays[1], &orderedArrays[2], &orderedArrays[3],
                createdData,
                inDS->GetPointData()->GetArray("avtOriginalNodeNumbers"),
                VTK_DOUBLE);
        }
        else
        {
            outDS = PointMeshFromVariables<float>(&orderedArrays[0],
                &orderedArrays[1], &orderedArrays[2], &orderedArrays[3],
                createdData,
                inDS->GetPointData()->GetArray("avtOriginalNodeNumbers"),
                VTK_FLOAT);
        }

        //
        // If we have a variable that's taking on the color role then add it
        // to the dataset.
        //
        if(outDS != 0 && orderedArrays[3].data != 0)
        {
            if(!createdData)
            {
                // Cell centering and Node centering are the same for a point
                // mesh but let's always do node centering because the glyph
                // mapper does not like it when we give it cell centered data
                // and it never uses the colors in that case.
                debug4 << "Adding " << orderedArrays[3].data->GetName()
                       << " as a nodal scalar field." << endl;
                outDS->GetPointData()->SetScalars(orderedArrays[3].data);
            }

            if(!createdData && !deleteArray4)
            {
                // The input dataset owns the data array. Make the output dataset
                // own it instead.
                if(var1Centering == AVT_NODECENT)
                    inDS->GetPointData()->RemoveArray(orderedArrays[3].data->GetName());
                else
                    inDS->GetCellData()->RemoveArray(orderedArrays[3].data->GetName());
            }
        }

        if(DebugStream::Level4())
            GetOutput()->GetInfo().GetAttributes().Print(DebugStream::Stream4());

        // Clean up data arrays that we may have had to generate.
        if(deleteArray2)
            arr[1]->Delete();
        if(deleteArray3)
            arr[2]->Delete();
        if(deleteArray4)
            arr[3]->Delete();
    }
    CATCH(InvalidLimitsException)
    {
        // Clean up data arrays that we may have had to generate.
        if(deleteArray2)
            arr[1]->Delete();
        if(deleteArray3)
            arr[2]->Delete();
        if(deleteArray4)
            arr[3]->Delete();

        RETHROW;
    }
    ENDTRY

    avtDataRepresentation *outDR = new avtDataRepresentation(outDS,
        inDR->GetDomain(), inDR->GetLabel());

    if (outDS != NULL)
        outDS->Delete();

    return outDR;
}

// ****************************************************************************
// Method: avtScatterFilter::PostExecute
//
// Purpose:
//   Executes after all domains are processed. Sets thisProcs spatial extents
//   and if a color var is selected, sets this var as the active variable for
//   display in the legend.
//
// Note: Refactored from ExecuteDataset to prevent parallel hang with when
//  there are more processors than chunks to process.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Aug 17 11:43:37 PDT 2010
//
// Modifications:
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
//    Hank Childs, Tue Dec 21 11:26:45 PST 2010
//    Set the active variable to be the color variable, where before it only
//    worked if the 4th variable was the color variable.
//
// ****************************************************************************

void
avtScatterFilter::PostExecute(void)
{
    avtDataTreeIterator::PostExecute();
    avtDataAttributes &out_datts = GetOutput()->GetInfo().GetAttributes();

    std::string color_var = "";
    if (atts.GetVar1Role() == ScatterAttributes::Color)
        color_var = atts.GetVar1();
    if (atts.GetVar2Role() == ScatterAttributes::Color)
        color_var = atts.GetVar2();
    if (atts.GetVar3Role() == ScatterAttributes::Color)
        color_var = atts.GetVar3();
    if (atts.GetVar4Role() == ScatterAttributes::Color)
        color_var = atts.GetVar4();
    if (color_var == "default")
        color_var = pipelineVariable;

    if (color_var != "")
    {
        //
        // Remove all of the variables that are not the color variable.
        //
        int nvars = out_datts.GetNumberOfVariables();
        int delIndex = 0;
        for(int ivar = 0; ivar < nvars; ++ivar)
        {
            std::string current_var(out_datts.GetVariableName(delIndex));
            if(current_var == color_var)
            {
                ++delIndex;
                out_datts.SetCentering(AVT_NODECENT, color_var.c_str());
            }
            else
                out_datts.RemoveVariable(current_var);
        }

        // Set the new active var and its extents
        out_datts.SetActiveVariable(color_var.c_str());
        out_datts.GetThisProcsOriginalDataExtents()->Set(colorExtents);
    }

    // correctly set the proper thisProcs spatial extents
    out_datts.GetThisProcsOriginalSpatialExtents()->Clear();
    out_datts.GetOriginalSpatialExtents()->Clear();

    if (NeedSpatialExtents())
    {
        // make sure all procs have the proper spatial extents
        // (unify here b/c processors that didn't have chunks may have invalid
        //  extents)
        UnifyMinMax(&thisProcsSpatialExtents[0],6);
        out_datts.GetThisProcsOriginalSpatialExtents()->Set(&thisProcsSpatialExtents[0]);

        debug4 << "avtScatterFilter::PostExecute() Final ThisProcs Spatial Extents: "
               << "xExtents = ["
               << thisProcsSpatialExtents[0] << ", "
               << thisProcsSpatialExtents[1]<< "] "
               << "yExtents = ["
               << thisProcsSpatialExtents[2] << ", "
               << thisProcsSpatialExtents[3]<< "] "
               << "zExtents = ["
               << thisProcsSpatialExtents[4] << ", "
               << thisProcsSpatialExtents[5]<< "]" << endl;
    }
}


// ****************************************************************************
//  Method: avtScatterFilter::PointMeshFromVariables
//
//  Purpose:
//      Sends the specified input and output through the scatter filter.
//
//  Arguments:
//      d1 : Information about the variable in the X coordinate role.
//      d2 : Information about the variable in the Y coordinate role.
//      d3 : Information about the variable in the Z coordinate role.
//
//  Returns:       The output point mesh dataset.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:36:23 PST 2004
//
//  Modifications:
//    Hank Childs, Sun Mar 13 11:38:06 PST 2005
//    Fix memory leak.
//
//    Jeremy Meredith, Fri Apr  1 16:07:29 PST 2005
//    Fix UMR.
//
//    Brad Whitlock, Mon Jul 18 11:58:42 PDT 2005
//    I made the extents get calculated in PreExecute so we have the extents
//    from all domains.
//
//    Hank Childs, Thu Sep 14 09:16:23 PDT 2006
//    Fix indexing bug and initialization bugs pointed out by Matt Wheeler.
//
//    Cyrus Harrison, Tue Aug 17 11:51:28 PDT 2010
//    Moved logic that modifies the output data atts to PostExecute.
//    Removed extents logic that was #ifdef-ed out.
//
//    Kathleen Biagas, Thu Mar  1 14:49:50 MST 2012
//    Keep track of original node numbers.
//
//    Kathleen Biagas, Fri Mar 20 11:44:28 PDT 2015
//    Make this method templated to support double precision.
//
// ****************************************************************************

template <typename T>
vtkDataSet *
avtScatterFilter::PointMeshFromVariables(DataInput *d1,
    DataInput *d2, DataInput *d3, DataInput *d4, bool &createdData,
    vtkDataArray *origNodes, int vtktype)
{
    const char *mName = "avtScatterFilter::PointMeshFromVariables: ";
    vtkPolyData *outDS = vtkPolyData::New();
    int n = d1->data->GetNumberOfTuples();
    vtkPoints *pts = vtkPoints::New(vtktype);
    pts->SetNumberOfPoints(n);
    T *coord = (T *)pts->GetVoidPointer(0);
    vtkCellArray *cells = vtkCellArray::New();
    cells->Allocate(n); 
    outDS->SetPoints(pts);
    outDS->SetVerts(cells);
    pts->Delete(); cells->Delete();

    vtkDataArray *arr1 = d1->data; // X
    vtkDataArray *arr2 = d2->data; // Y
    vtkDataArray *arr3 = d3->data; // Z
    vtkDataArray *arr4 = d4->data; // Color

    vtkDataArray *newOrigNodes = NULL;
    if (origNodes != NULL)
    {
        newOrigNodes  = origNodes->NewInstance();
        newOrigNodes->SetNumberOfComponents(origNodes->GetNumberOfComponents());
        newOrigNodes->Allocate(origNodes->GetNumberOfTuples());
        newOrigNodes->SetName(origNodes->GetName());
    }

    // Indicate that we're not creating data, though that could change.
    createdData = false;

    // Initially set the min/max values to the calculated extents. Note that
    // the extents here will not have been calculated if we are using user-
    // specified extents but we take care of that later.
    T xMin = T(xExtents[0]);
    T xMax = T(xExtents[1]);
    T yMin = T(yExtents[0]);
    T yMax = T(yExtents[1]);
    T zMin = T(zExtents[0]);
    T zMax = T(zExtents[1]);
    debug4 << mName << "xExtents = [" << xMin << ", " << xMax << "] "
           << "yExtents = [" << yMin << ", " << yMax << "] "
           << "zExtents = [" << zMin << ", " << zMax << "]" << endl;

    vtkIdType nCells = 0;

    debug4 << mName << "arr1 = " << arr1->GetName()
           << ", ntuples=" << arr1->GetNumberOfTuples() << endl;
    debug4 << mName << "arr2 = " << arr2->GetName()
           << ", ntuples=" << arr2->GetNumberOfTuples() << endl;

    T d1min = d1->min;
    T d1max = d1->max;
    T d2min = d2->min;
    T d2max = d2->max;
    T d3min = d3->min;
    T d3max = d3->max;

    //
    // If arr3 == 0 then we're creating a 2D mesh.
    //
    if(arr3 == 0)
    {
        if(d1->useMin || d1->useMax || d2->useMin || d2->useMax)
        {
            if(arr4 == 0)
            {
                // Not coloring the plot
                for(vtkIdType i = 0; i < n; ++i)
                {
                    coord[0] = arr1->GetTuple1(i);
                    coord[1] = arr2->GetTuple1(i);
                    coord[2] = 0.;
                    BEGIN_X_RANGE
                        BEGIN_Y_RANGE
                            coord += 3;
                            cells->InsertNextCell(1, &nCells);
                            ++nCells;
                            if (origNodes)
                                newOrigNodes->InsertNextTuple(i, origNodes);
                        END_Y_RANGE            
                    END_X_RANGE
                }
            }
            else
            {
                // Coloring the plot.
                debug4 << mName << "arr4 = " << arr4->GetName()
                       << ", ntuples=" << arr4->GetNumberOfTuples() << endl;

                vtkDataArray *newColorData = 
                    vtkDataArray::CreateDataArray(arr4->GetDataType());
                newColorData->SetNumberOfComponents(
                    arr4->GetNumberOfComponents());
                newColorData->Allocate(n);
                newColorData->SetName(arr4->GetName());

                for(vtkIdType i = 0; i < n; ++i)
                {
                    coord[0] = arr1->GetTuple1(i);
                    coord[1] = arr2->GetTuple1(i);
                    coord[2] = 0.;
                    BEGIN_X_RANGE
                        BEGIN_Y_RANGE
                            coord += 3;
                            cells->InsertNextCell(1, &nCells);
                            newColorData->InsertNextTuple(arr4->GetTuple(i));
                            ++nCells;
                            if (origNodes)
                                newOrigNodes->InsertNextTuple(i, origNodes);
                        END_Y_RANGE            
                    END_X_RANGE
                }

                createdData = true;
                newColorData->Squeeze();
                outDS->GetPointData()->SetScalars(newColorData);
                newColorData->Delete();
            }
            pts->SetNumberOfPoints(nCells);
        }
        else
        {
            nCells = n;
            for(vtkIdType i = 0; i < n; ++i, coord += 3)
            {
                coord[0] = arr1->GetTuple1(i);
                coord[1] = arr2->GetTuple1(i);
                coord[2] = 0.;
                cells->InsertNextCell(1, &i);
                if (origNodes)
                    newOrigNodes->InsertNextTuple(i, origNodes);
            }
        }
    }
    else
    {
        debug4 << mName << "arr3 = " << arr3->GetName()
               << ", ntuples=" << arr3->GetNumberOfTuples() << endl;

        if(d1->useMin || d1->useMax ||
           d2->useMin || d2->useMax ||
           d3->useMin || d3->useMax)
        {
            if(arr4 == 0)
            {
                // Not coloring the plot.
                for(vtkIdType i = 0; i < n; ++i)
                {
                    coord[0] = arr1->GetTuple1(i);
                    coord[1] = arr2->GetTuple1(i);
                    coord[2] = arr3->GetTuple1(i);

                    //
                    // Only add values that are in the specified min/max range.
                    //
                    BEGIN_X_RANGE
                        BEGIN_Y_RANGE
                            BEGIN_Z_RANGE
                                coord += 3;
                                cells->InsertNextCell(1, &nCells);
                                ++nCells;
                                if (origNodes)
                                    newOrigNodes->InsertNextTuple(i, origNodes);
                            END_Z_RANGE
                        END_Y_RANGE            
                    END_X_RANGE
                }
            }
            else
            {
                // Coloring the plot.
                debug4 << mName << "arr4 = " << arr4->GetName()
                       << ", ntuples=" << arr4->GetNumberOfTuples() << endl;

                vtkDataArray *newColorData = 
                    vtkDataArray::CreateDataArray(arr4->GetDataType());
                newColorData->SetNumberOfComponents(
                    arr4->GetNumberOfComponents());
                newColorData->Allocate(n);
                newColorData->SetName(arr4->GetName());

                for(vtkIdType i = 0; i < n; ++i)
                {
                    coord[0] = arr1->GetTuple1(i);
                    coord[1] = arr2->GetTuple1(i);
                    coord[2] = arr3->GetTuple1(i);

                    //
                    // Only add values that are in the specified min/max range.
                    //
                    BEGIN_X_RANGE
                        BEGIN_Y_RANGE
                            BEGIN_Z_RANGE
                                coord += 3;
                                cells->InsertNextCell(1, &nCells);
                                newColorData->InsertNextTuple(arr4->GetTuple(i));
                                ++nCells;
                                if (origNodes)
                                    newOrigNodes->InsertNextTuple(i, origNodes);
                            END_Z_RANGE
                        END_Y_RANGE            
                    END_X_RANGE
                }

                createdData = true;
                newColorData->Squeeze();
                outDS->GetPointData()->SetScalars(newColorData);
                newColorData->Delete();
            }

            pts->SetNumberOfPoints(nCells);
        }
        else
        {
            nCells = n;
            for(vtkIdType i = 0; i < n; ++i, coord += 3)
            {
                coord[0] = arr1->GetTuple1(i);
                coord[1] = arr2->GetTuple1(i);
                coord[2] = arr3->GetTuple1(i);
                cells->InsertNextCell(1, &i);
                if (origNodes)
                    newOrigNodes->InsertNextTuple(i, origNodes);
            }
        }
    }
    if (origNodes)
    {
        outDS->GetPointData()->AddArray(newOrigNodes);
    }

    //
    // Scale the coordinate fields.
    //
    int d1scale = d1->scale;
    int d2scale = d2->scale;
    int d3scale = d3->scale;
    if(d1scale > 0 || d2scale > 0 || d3scale > 0)
    {    
        coord = (T *)pts->GetVoidPointer(0);

        // Variables for skew scaling.
        T x_range = 0, x_rangeInverse = 0, x_logSkew = 0, x_k = 0;
        if(d1scale == 2)
        {
            x_range = xMax - xMin; 
            x_rangeInverse = 1. / x_range;
            x_logSkew = log(d1->skew);
            x_k = x_range / (d1->skew - 1.);
        }
        T y_range = 0, y_rangeInverse = 0, y_logSkew = 0, y_k = 0;
        if(d2scale == 2)
        {
            y_range = yMax - yMin; 
            y_rangeInverse = 1. / y_range;
            y_logSkew = log(d2->skew);
            y_k = y_range / (d2->skew - 1.);
        }

#define LOG10_X(X) log10(X)
#define LOG10_Y(Y) log10(Y)
#define LOG10_Z(Z) log10(Z)

        if(arr3 == 0)
        {
            for(int i = 0; i < nCells; ++i, coord += 3)
            {
                // Scale X coordinate
                if(d1scale == 1)
                    coord[0] = LOG10_X(coord[0]);
                else if(d1scale == 2)
                {
                    T tmp = (coord[0] - xMin) * x_rangeInverse;
                    coord[0] = x_k * (exp(tmp * x_logSkew) - 1.) + xMin;
                }

                // Scale Y coordinate
                if(d2scale == 1)
                    coord[1] = LOG10_Y(coord[1]);
                else if(d2scale == 2)
                {
                    T tmp = (coord[1] - yMin) * y_rangeInverse;
                    coord[1] = y_k * (exp(tmp * y_logSkew) - 1.) + yMin;
                }
            }
        }
        else
        {
            T z_range = 0, z_rangeInverse = 0, z_logSkew = 0, z_k = 0;
            if(d3->scale == 2)
            {
                z_range = zMax - zMin; 
                z_rangeInverse = 1. / z_range;
                z_logSkew = log(d3->skew);
                z_k = z_range / (d3->skew - 1.);
            }

            for(int i = 0; i < nCells; ++i, coord += 3)
            {
                // Scale X coordinate
                if(d1scale == 1)
                    coord[0] = LOG10_X(coord[0]);
                else if(d1scale == 2)
                {
                    T tmp = (coord[0] - xMin) * x_rangeInverse;
                    coord[0] = x_k * (exp(tmp * x_logSkew) - 1.) + xMin;
                }

                // Scale Y coordinate
                if(d2scale == 1)
                    coord[1] = LOG10_Y(coord[1]);
                else if(d2scale == 2)
                {
                    T tmp = (coord[1] - yMin) * y_rangeInverse;
                    coord[1] = y_k * (exp(tmp * y_logSkew) - 1.) + yMin;
                }

                // Scale Z Coordinate
                if(d3scale == 1)
                    coord[2] = LOG10_Z(coord[2]);
                else if(d3scale == 2)
                {
                    T tmp = (coord[2] - zMin) * z_rangeInverse;
                    coord[2] = z_k * (exp(tmp * z_logSkew) - 1.) + zMin;
                }
            }
        }

        //
        // Since Log scaling does not preserve min,max values like
        // linear and skew scaling, we must transform the min,max.
        //
        bool badLimits = false;
        if(d1scale == 1)
        {
            T uxMin = d1->useMin ? d1min : xMin;
            T uxMax = d1->useMax ? d1max : xMax;
            badLimits |= (uxMin <= 0. || uxMax <= 0.);
            xMin = LOG10_X(uxMin);
            xMax = LOG10_X(uxMax);
        }

        if(d2scale == 1)
        {
            T uyMin = d2->useMin ? d2min : yMin;
            T uyMax = d2->useMax ? d2max : yMax;
            badLimits |= (uyMin <= 0. || uyMax <= 0.);
            yMin = LOG10_Y(uyMin);
            yMax = LOG10_Y(uyMax);
        }

        if(arr3 != 0 && d3scale == 1)
        {
            T uzMin = d3->useMin ? d3min : zMin;
            T uzMax = d3->useMax ? d3max : zMax;
            badLimits |= (uzMin <= 0. || uzMax <= 0.);
            zMin = LOG10_Z(uzMin);
            zMax = LOG10_Z(uzMax);
        }

        if(badLimits)
        {
            outDS->Delete();
            EXCEPTION1(InvalidLimitsException, true);
        }
    }

    //
    // Scale the coordinates so they are within a [0,1] cube.
    //
    if(atts.GetScaleCube())
    {
        const T boxSize = 1.;
        T dX = xMax - xMin;
        T dY = yMax - yMin;
        T x_rangeInverse = (dX == 0.) ? 1. : (boxSize / dX);
        T y_rangeInverse = (dY == 0.) ? 1. : (boxSize / dY);
        coord = (T *)pts->GetVoidPointer(0);
        if(arr3 == 0)
        {
            for(int i = 0; i < nCells; ++i, coord += 3)
            {
                coord[0] = (coord[0] - xMin) * x_rangeInverse;
                coord[1] = (coord[1] - yMin) * y_rangeInverse;
            }
        }
        else
        {
            T dZ = zMax - zMin;
            T z_rangeInverse = (dZ == 0.) ? 1. : (boxSize / dZ);
            for(int i = 0; i < nCells; ++i, coord += 3)
            {
                coord[0] = (coord[0] - xMin) * x_rangeInverse;
                coord[1] = (coord[1] - yMin) * y_rangeInverse;
                coord[2] = (coord[2] - zMin) * z_rangeInverse;
            }
        }

        // Min, max of scaled coordinates.
        xMin = yMin = zMin = 0.;
        xMax = yMax = zMax = 1.;
    }

    //
    // We need some way to set some labels or something into the axes because
    // the values that will be displayed are not right because we've transformed
    // the points to get a better spatial layout.
    //

    //
    // Set the final spatial extents value.
    //
    thisProcsSpatialExtents[0] = xMin;
    thisProcsSpatialExtents[1] = xMax;
    thisProcsSpatialExtents[2] = yMin;
    thisProcsSpatialExtents[3] = yMax;
    thisProcsSpatialExtents[4] = zMin;
    thisProcsSpatialExtents[5] = zMax;

    return outDS;
}

// ****************************************************************************
//  Method: avtScatterFilter::GetDataArray
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:36:23 PST 2004
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan  7 13:30:30 PST 2005
//    Moved retrieval of cenering out of TRY-CATCH block, and test for
//    ValidVariable before retrieving centering.
//
//    Brad Whitlock, Fri Jul 16 14:25:22 PDT 2010
//    I added code to work around the centering coming back as UNKNOWN as it
//    now seems to be with curves.
//
// ****************************************************************************

vtkDataArray *
avtScatterFilter::GetDataArray(vtkDataSet *inDS, const std::string &name,
    avtCentering targetCentering, bool &deleteArray)
{
    vtkDataArray *retval = 0;

    avtCentering centering = AVT_UNKNOWN_CENT;
    if (GetInput()->GetInfo().GetAttributes().ValidVariable(name.c_str()))
    {
        // Get the variable's centering.
        centering = GetInput()->GetInfo().GetAttributes().
            GetCentering(name.c_str());

        debug4 << name << " is a valid variable with";

        // Get a pointer to the array out of the dataset.
        if(centering == AVT_NODECENT)
        {
            debug4 << " node centering" << endl;
            retval = inDS->GetPointData()->GetArray(name.c_str());
        }
        else if (centering == AVT_ZONECENT)
        {
            debug4 << " zone centering" << endl;
            retval = inDS->GetCellData()->GetArray(name.c_str());
        }
        else
        {
            debug4 << " unknown centering";
            retval = inDS->GetPointData()->GetArray(name.c_str());
            if(retval != 0)
            {
                debug4 << "  -- wait, it's node centered" << endl;
                centering = AVT_NODECENT;
            }
            else
            {
                retval = inDS->GetCellData()->GetArray(name.c_str());
                centering = AVT_ZONECENT;
                debug4 << "  -- wait, it's zone centered" << endl;
            }
        }
    }
    else
    {
        debug4 << name << " is not a valid variable." << endl;
    }

    TRY
    {
        //
        // If we have a data array and its centering is not what we want
        // then create a new data array that has the opposite centering.
        //
        if(targetCentering != centering && retval != 0)
        {
            debug4 << "The variable centerings do not match. Recentering..." << endl;

            // Create a recentered copy of the data array.
            retval = Recenter(inDS, retval, targetCentering);
            deleteArray = true;
        }
    }
    CATCH(VisItException)
    {
        // nothing.
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: avtScatterFilter::PopulateDataInputs
//
// Purpose: 
//   Maps the state object's various variables into a DataInput array so we 
//   know more easily which variable performs the X role, the Y role, etc.
//
// Arguments:
//   orderedArrays : The array that we're populating.
//   arr           : The array of data arrays to associate with the variables.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 18 11:50:08 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtScatterFilter::PopulateDataInputs(DataInput *orderedArrays, vtkDataArray **arr) const
{
    int index = int(atts.GetVar1Role());
    orderedArrays[index].data = arr[0];
    orderedArrays[index].useMin = atts.GetVar1MinFlag();
    orderedArrays[index].useMax = atts.GetVar1MaxFlag();
    orderedArrays[index].min = atts.GetVar1Min();
    orderedArrays[index].max = atts.GetVar1Max();
    orderedArrays[index].scale = int(atts.GetVar1Scaling());
    orderedArrays[index].skew = atts.GetVar1SkewFactor();
    // Revert to Linear scaling with bad skew factor.
    if (orderedArrays[index].scale == 2 &&
        (orderedArrays[index].skew <= 0 || orderedArrays[index].skew == 1.)) 
    {
        debug4 << "Bad skew factor for var1: " << atts.GetVar1SkewFactor()
               << endl;
        orderedArrays[index].scale = 0;
    }

    index = int(atts.GetVar2Role());
    orderedArrays[index].data = arr[1];
    orderedArrays[index].useMin = atts.GetVar2MinFlag();
    orderedArrays[index].useMax = atts.GetVar2MaxFlag();
    orderedArrays[index].min = atts.GetVar2Min();
    orderedArrays[index].max = atts.GetVar2Max();
    orderedArrays[index].scale = int(atts.GetVar2Scaling());
    orderedArrays[index].skew = atts.GetVar2SkewFactor();
    // Revert to Linear scaling with bad skew factor.
    if (orderedArrays[index].scale == 2 &&
        (orderedArrays[index].skew <= 0 || orderedArrays[index].skew == 1.)) 
    {
        debug4 << "Bad skew factor for var2: " << atts.GetVar2SkewFactor()
               << endl;
        orderedArrays[index].scale = 0;
    }

    index = int(atts.GetVar3Role());
    orderedArrays[index].data = arr[2];
    orderedArrays[index].useMin = atts.GetVar3MinFlag();
    orderedArrays[index].useMax = atts.GetVar3MaxFlag();
    orderedArrays[index].min = atts.GetVar3Min();
    orderedArrays[index].max = atts.GetVar3Max();
    orderedArrays[index].scale = int(atts.GetVar3Scaling());
    orderedArrays[index].skew = atts.GetVar3SkewFactor();
    // Revert to Linear scaling with bad skew factor.
    if (orderedArrays[index].scale == 2 &&
        (orderedArrays[index].skew <= 0 || orderedArrays[index].skew == 1.)) 
    {
        debug4 << "Bad skew factor for var3: " << atts.GetVar3SkewFactor()
               << endl;
        orderedArrays[index].scale = 0;
    }

    index = int(atts.GetVar4Role());
    orderedArrays[index].data = arr[3];
    orderedArrays[index].useMin = atts.GetVar4MinFlag();
    orderedArrays[index].useMax = atts.GetVar4MaxFlag();
    orderedArrays[index].min = atts.GetVar4Min();
    orderedArrays[index].max = atts.GetVar4Max();
    orderedArrays[index].scale = int(atts.GetVar4Scaling());
    orderedArrays[index].skew = atts.GetVar4SkewFactor();
    // Revert to Linear scaling with bad skew factor.
    if (orderedArrays[index].scale == 2 &&
        (orderedArrays[index].skew <= 0 || orderedArrays[index].skew == 1.)) 
    {
        debug4 << "Bad skew factor for var4: " << atts.GetVar4SkewFactor()
               << endl;
        orderedArrays[index].scale = 0;
    }
}

// ****************************************************************************
// Method: avtScatterFilter::PopulateNames
//
// Purpose: 
//   Populates an array of names taking into account variable roles.
//
// Arguments:
//   names : The array to populate.
//
// Note:       The names array must be at least 5 elements long.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 18 13:56:46 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtScatterFilter::PopulateNames(const char **names) const
{
    names[int(atts.GetVar1Role())] = atts.GetVar1().c_str();

    if(atts.GetVar2() == "default")
        names[int(atts.GetVar2Role())] = atts.GetVar1().c_str();
    else
        names[int(atts.GetVar2Role())] = atts.GetVar2().c_str();

    if(atts.GetVar3() == "default")
        names[int(atts.GetVar3Role())] = atts.GetVar1().c_str();
    else
        names[int(atts.GetVar3Role())] = atts.GetVar3().c_str();

    if(atts.GetVar4() == "default")
        names[int(atts.GetVar4Role())] = atts.GetVar1().c_str();
    else
        names[int(atts.GetVar4Role())] = atts.GetVar4().c_str();
}

// ****************************************************************************
//  Method: avtScatterFilter::UpdateDataObjectInfo
//
//  Purpose:
//    Indicates that topological dimension of the output is 1.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:36:23 PST 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan  7 13:30:30 PST 2005
//    Removed TRY-CATCH blocks in favor of testing for ValidVariable.
//
//    Kathleen Biagas, Thu Mar  1 14:49:50 MST 2012
//    Keep  original nodes/zones when necessary.
//
// ****************************************************************************

void
avtScatterFilter::UpdateDataObjectInfo(void)
{
debug4 << "avtScatterFilter::UpdateDataObjectInfo" << endl;
    avtDataAttributes &dataAtts = GetOutput()->GetInfo().GetAttributes();
    std::string v1Units(""), v2Units(""), v3Units(""), v4Units("");

    if(atts.GetVar1Role() != ScatterAttributes::None)
    {
        if (dataAtts.ValidVariable(atts.GetVar1().c_str()))
        {
            v1Units = dataAtts.GetVariableUnits(atts.GetVar1().c_str());
        }
    }

    if(atts.GetVar2Role() != ScatterAttributes::None)
    {
        if (dataAtts.ValidVariable(atts.GetVar2().c_str()))
        {
            v2Units = dataAtts.GetVariableUnits(atts.GetVar2().c_str());
        }
    }

    if(atts.GetVar3Role() != ScatterAttributes::None)
    {
        if (dataAtts.ValidVariable(atts.GetVar3().c_str()))
        {
            v3Units = dataAtts.GetVariableUnits(atts.GetVar3().c_str());
        }
    }

    if(atts.GetVar4Role() != ScatterAttributes::None)
    {
        if (dataAtts.ValidVariable(atts.GetVar4().c_str()))
        {
            v4Units = dataAtts.GetVariableUnits(atts.GetVar4().c_str());
        }
    }

    //
    // Set the axis units and labels according to the roles of the variables
    // that were used to create the mesh.
    //
    const char *labels[5] = {0,0,0,0,0};
    const char *units[5] = {0,0,0,0,0};
    labels[int(atts.GetVar1Role())] = atts.GetVar1().c_str();
    units[int(atts.GetVar1Role())] = v1Units.c_str();
    labels[int(atts.GetVar2Role())] = atts.GetVar2().c_str();
    units[int(atts.GetVar2Role())] = v2Units.c_str();
    labels[int(atts.GetVar3Role())] = atts.GetVar3().c_str();
    units[int(atts.GetVar3Role())] = v3Units.c_str();
    labels[int(atts.GetVar4Role())] = atts.GetVar4().c_str();
    units[int(atts.GetVar4Role())] = v4Units.c_str();
    dataAtts.SetXLabel((labels[0] == 0) ? "" : labels[0]);
    dataAtts.SetXUnits((units[0] == 0) ? "" : units[0]);
    dataAtts.SetYLabel((labels[1] == 0) ? "" : labels[1]);
    dataAtts.SetYUnits((units[1] == 0) ? "" : units[1]);
    dataAtts.SetZLabel((labels[2] == 0) ? "" : labels[2]);
    dataAtts.SetZUnits((units[2] == 0) ? "" : units[2]);

    // Set the spatial dimension
    dataAtts.SetSpatialDimension(CountSpatialDimensions());
    dataAtts.SetTopologicalDimension(0);

    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
    GetOutput()->GetInfo().GetValidity().SetPointsWereTransformed(true);
    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
}

int
avtScatterFilter::CountSpatialDimensions() const
{
    int dims = 0;
    if(atts.GetVar1Role() != ScatterAttributes::None &&
       atts.GetVar1Role() != ScatterAttributes::Color)
        ++dims;
    if(atts.GetVar2Role() != ScatterAttributes::None &&
       atts.GetVar2Role() != ScatterAttributes::Color)
        ++dims;
    if(atts.GetVar3Role() != ScatterAttributes::None &&
       atts.GetVar3Role() != ScatterAttributes::Color)
        ++dims;
    if(atts.GetVar4Role() != ScatterAttributes::None &&
       atts.GetVar4Role() != ScatterAttributes::Color)
        ++dims;

    return (dims > 3) ? 3 : dims;
}


vtkDataArray *
avtScatterFilter::Recenter(vtkDataSet *ds, vtkDataArray *arr, 
                           avtCentering cent) const
{
    vtkDataSet *ds2 = ds->NewInstance();
    ds2->CopyStructure(ds);

    vtkDataArray *outv = NULL;
    if (cent == AVT_ZONECENT)
    {
        if (ds2->GetNumberOfPoints() != arr->GetNumberOfTuples())
        {
            ds2->Delete();
            EXCEPTION1(ImproperUseException, "Asked to re-center a nodal "
                       "variable that is not nodal.");
        }

        ds2->GetPointData()->SetScalars(arr);

        vtkPointDataToCellData *pd2cd = vtkPointDataToCellData::New();
        pd2cd->SetInputData(ds2);
        pd2cd->Update();
        outv = pd2cd->GetOutput()->GetCellData()->GetScalars();
        outv->Register(NULL);
        pd2cd->Delete();
    }
    else
    {
        if (ds2->GetNumberOfCells() != arr->GetNumberOfTuples())
        {
            ds2->Delete();
            EXCEPTION1(ImproperUseException, "Asked to re-center a zonal "
                       "variable that is not zonal.");
        }

        ds2->GetCellData()->SetScalars(arr);

        vtkCellDataToPointData *cd2pd = vtkCellDataToPointData::New();
        cd2pd->SetInputData(ds2);
        cd2pd->Update();
        outv = cd2pd->GetOutput()->GetPointData()->GetScalars();
        outv->Register(NULL);
        cd2pd->Delete();
    }

    ds2->Delete();
    return outv;
}

// ****************************************************************************
// Method: avtScatterPlot::NeedSpatialExtents
//
// Purpose: 
//   Determines whether spatial extents are needed.
//
// Returns:    True if spatial extents are needed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 19 12:27:24 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
avtScatterFilter::NeedSpatialExtents() const
{
    //
    // We always need spatial min/max values unless we're doing
    // Linear scaling with no min/max values set and no scale to cube.
    //
    bool needSpatialExtents;
    if(atts.GetScaleCube())
        needSpatialExtents = true;
    else
    {
        //
        // Assign variables according to their roles. var[0] == X,
        // var[1] == Y, and var[2] == Z.
        //
        const char *names[5] = {0,0,0,0,0};
        PopulateNames(names);
        DataInput var[5] = {
            {0, false, false, 0., 0., 0, 0.},
            {0, false, false, 0., 0., 0, 0.},
            {0, false, false, 0., 0., 0, 0.},
            {0, false, false, 0., 0., 0, 0.},
            {0, false, false, 0., 0., 0, 0.}};
        vtkDataArray *arr[4] = {0,0,0,0};
        PopulateDataInputs(var, arr);

        bool var1LinearScaleNoMinMax = (var[0].scale == 0) &&
                                       (!var[0].useMin && !var[0].useMax);
        bool var2LinearScaleNoMinMax = (var[1].scale == 0) &&
                                       (!var[1].useMin && !var[1].useMax);
        if(names[2] == 0)
        {
             needSpatialExtents = !(var1LinearScaleNoMinMax &&
                                    var2LinearScaleNoMinMax);
        }
        else
        {
            bool var3LinearScaleNoMinMax = (var[2].scale == 0) &&
                                           (!var[2].useMin && !var[2].useMax);
            needSpatialExtents = !(var1LinearScaleNoMinMax &&
                                   var2LinearScaleNoMinMax &&
                                   var3LinearScaleNoMinMax);
        }
    }

    debug4 << "avtScatterFilter::NeedSpatialExtents = "
           << (needSpatialExtents?"True":"False") << endl;

    return needSpatialExtents;
}

// ****************************************************************************
// Method: avtScatterFilter::ModifyContract
//
// Purpose: 
//   Returns an altered pipeline specification and determines which variables
//   must have their min/max values calculated.
//
// Arguments:
//
// Returns:    A new pipeline specification.
//
// Note:       This method is used here to determine which variables need to
//             have their min/max values calculated. If any need to have their
//             min/max values calculated then we turn off DLB.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 18 11:42:11 PDT 2005
//
// Modifications:
//   Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//   API change for avtIntervalTree.
//
//   Hank Childs, Tue Feb 19 19:45:43 PST 2008
//   Rename "dynamic" to "streaming", since we really care about whether we
//   are streaming, not about whether we are doing dynamic load balancing.
//   And the two are no longer synonymous.
//
//   Kathleen Biagas, Thu Mar  1 14:51:45 PST 2012
//   Request Original Nodes when necessary.
//
// ****************************************************************************

avtContract_p
avtScatterFilter::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = new avtContract(spec);

    //
    // Assign variables according to their roles. var[0] == X, var[1] == Y, 
    // and var[2] == Z.
    //
    const char *names[5] = {0,0,0,0,0};
    PopulateNames(names);
    DataInput var[5] = {
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.}};
    vtkDataArray *arr[4] = {0,0,0,0};
    PopulateDataInputs(var, arr);

    //
    // If we need spatial extents, try and get them now.
    //
    if(NeedSpatialExtents())
    {
        if (TryDataExtents(xExtents, names[0]))
        {
            needXExtents = false;
            debug1 << "avtScatterFilter::ModifyContract: xExtents="
                   << xExtents[0] << ", " << xExtents[1] << endl;
        }
        else
            needXExtents = true;

        // If we did not specify both min and max then get the extents.
        if (TryDataExtents(yExtents, names[1]))
        {
            needYExtents = false;
            debug1 << "avtScatterFilter::ModifyContract: yExtents="
                   << yExtents[0] << ", " << yExtents[1] << endl;
        }
        else
            needYExtents = true;

        if(names[2] != 0)
        {
            // If we did not specify both min and max then get the extents.
            if (TryDataExtents(zExtents, names[2]))
            {
                needZExtents = false;
                debug1 << "avtScatterFilter::ModifyContract: zExtents="
                       << zExtents[0] << ", " << zExtents[1] << endl;
            }
            else
                needZExtents = true;
        }
    }

    // If we're coloring by a variable, get the color variable's extents.
    if(names[3] != 0)
    {
        // If we did not specify both min and max then get the extents.
        if(!(var[3].useMin && var[3].useMax))
        {
            if (TryDataExtents(colorExtents, names[3]))
            {
                needColorExtents = false;
                debug1 << "avtScatterFilter::ModifyContract: colorExtents="
                       << colorExtents[0] << ", " << colorExtents[1] << endl;
            }
            else
                needColorExtents = true;
        }
    }

    // If we need any extents that were not available, no DLB.
    if(needXExtents || needYExtents || needZExtents || needColorExtents)
        rv->NoStreaming();

    // Pick which domains should be considered using the interval tree.
    for(int i = 0; i < 4; ++i)
    {
        if(names[i] != 0 && (var[i].useMin || var[i].useMax))
        {
            avtIntervalTree *it = GetMetaData()->GetDataExtents(names[i]);
            if(it != NULL)
            {
                double minval = var[i].useMin ? var[i].min : SCATTER_FLOAT_MIN;
                double maxval = var[i].useMax ? var[i].max : SCATTER_FLOAT_MAX;
                std::vector<int> dl;
                it->GetElementsListFromRange(&minval, &maxval, dl);
                rv->GetDataRequest()->GetRestriction()->RestrictDomains(dl);
            }
        }
    }

    if (rv->GetDataRequest()->MayRequireNodes() ||
        rv->GetDataRequest()->MayRequireZones())
    {
        keepNodeZone = true;
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }
    else
        keepNodeZone = false;


    return rv;
}
