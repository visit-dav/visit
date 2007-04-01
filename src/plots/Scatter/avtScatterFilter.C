// ************************************************************************* //
//                              avtScatterFilter.C                           //
// ************************************************************************* //

#include <avtScatterFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkPolyData.h>

#include <avtExtents.h>

#include <ImproperUseException.h>
#include <DebugStream.h>

//#include <visitstream.h>
//#include <avtDataAttributes.h>

// ****************************************************************************
//  Method: avtScatterFilter constructor
//
//  Arguments:
//
//  Programmer: Brad Whitlock 
//  Creation:   Tue Nov 2 22:36:23 PST 2004
//
//  Modifications:
//
// ****************************************************************************

avtScatterFilter::avtScatterFilter(const std::string &v, const ScatterAttributes &a)
    : avtStreamer(), variableName(v), atts(a)
{
#ifdef THE_FILTER_KNOWS_HOW_TO_ADD_ITS_OWN_VARS
    stringVector vars;

    //
    // Add the secondary variables.
    //
    if(atts.GetVar2Role() != ScatterAttributes::None &&
       atts.GetVar2().size() > 0 &&
       atts.GetVar2() != "default")
    {
        vars.push_back(atts.GetVar2());
    }

    if(atts.GetVar3Role() != ScatterAttributes::None &&
       atts.GetVar3().size() > 0 &&
       atts.GetVar3() != "default")
    {
        vars.push_back(atts.GetVar3());
    }

    if(atts.GetVar4Role() != ScatterAttributes::None &&
       atts.GetVar4().size() > 0 &&
       atts.GetVar4() != "default")
    {
        vars.push_back(atts.GetVar4());
    }

    if(vars.size() > 0)
    {
        SetActiveVariable(vars[0].c_str());
        for(int i = 1; i < vars.size(); ++i)
            AddSecondaryVariable(vars[i].c_str());
    }
#endif
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

        // Get a pointer to the array out of the dataset.
        if(centering == AVT_NODECENT)
            retval = inDS->GetPointData()->GetArray(name.c_str());
        else if (centering == AVT_ZONECENT)
            retval = inDS->GetCellData()->GetArray(name.c_str());
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
//  Method: avtScatterFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the scatter filter.
//
//  Arguments:
//      inDS       The input dataset.
//      dom        The domain number.
//      lab        The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:36:23 PST 2004
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtScatterFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
debug4 << "avtScatterFilter::ExecuteData" << endl;
    avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
 
    // Determine the name of the first varaible.
    std::string var1Name(variableName);
debug4 << "avtScatterFilter::ExecuteData: var1Name = " << var1Name.c_str() << endl;

    // Determine the name of the second varaible.
    std::string var2Name(atts.GetVar2());
    if(var2Name == "default")
        var2Name = var1Name;
debug4 << "avtScatterFilter::ExecuteData: var2Name = " << var2Name.c_str() << endl;

    // Determine the name of the 3rd varaible.
    std::string var3Name(atts.GetVar3());
    if(var3Name == "default")
        var3Name = var1Name;

    // Determine the name of the 4th varaible.
    std::string var4Name(atts.GetVar4());
    if(var4Name == "default")
        var4Name = var1Name;

    bool deleteArray2 = false, deleteArray3 = false, deleteArray4 = false;
    vtkDataArray *arr1 = 0;
    vtkDataArray *arr2 = 0;
    vtkDataArray *arr3 = 0;
    vtkDataArray *arr4 = 0;

    //
    // Determine the centering for var1 and get its data array.
    //
    avtCentering var1Centering = datts.GetCentering(var1Name.c_str());
    if(var1Centering == AVT_NODECENT)
        arr1 = inDS->GetPointData()->GetArray(var1Name.c_str());
    else
        arr1 = inDS->GetCellData()->GetArray(var1Name.c_str());

    //
    // Get the data arrays for the secondary variables.
    //
    if(atts.GetVar2Role() != ScatterAttributes::None)
        arr2 = GetDataArray(inDS, var2Name, var1Centering, deleteArray2);

    if(atts.GetVar3Role() != ScatterAttributes::None)
        arr3 = GetDataArray(inDS, var3Name, var1Centering, deleteArray3);

    if(atts.GetVar4Role() != ScatterAttributes::None)
        arr4 = GetDataArray(inDS, var4Name, var1Centering, deleteArray4);

    //
    // Put the input variables into the appropriate X,Y,Z coordinate role.
    //
    DataInput orderedArrays[5] = {
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.},
        {0, false, false, 0., 0., 0, 0.}};
    int index = int(atts.GetVar1Role());
    orderedArrays[index].data = arr1;
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
    orderedArrays[index].data = arr2;
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
    orderedArrays[index].data = arr3;
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
    orderedArrays[index].data = arr4;
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

    //
    // Make sure that X,Y roles have arrays to use for coordinates.
    //
    if(orderedArrays[0].data == 0 || orderedArrays[1].data == 0)
    {
        if(deleteArray2)
            arr2->Delete();
        if(deleteArray3)
            arr3->Delete();
        if(deleteArray4)
            arr4->Delete();
        EXCEPTION1(ImproperUseException, "At least two variables must play a "
            "role in creating coordinates and the X,Y coordinate roles must be "
            "assigned to a variable.");
    }

    //
    // Create the point mesh from the input data arrays.
    //
    vtkDataSet *outDS = PointMeshFromVariables(&orderedArrays[0],
        &orderedArrays[1], &orderedArrays[2]);

    //
    // If we have a variable that's taking on the color role then add it
    // to the dataset.
    //
    if(orderedArrays[3].data != 0)
    {
        // Add the array to the output dataset.
        if(var1Centering == AVT_NODECENT)
        {
            debug4 << "Adding " << orderedArrays[3].data->GetName()
                   << " as a nodal scalar field." << endl;
//            outDS->GetPointData()->AddArray(orderedArrays[3].data);
//            outDS->GetPointData()->SetActiveVariable(orderedArrays[3].data->GetName());
            outDS->GetPointData()->SetScalars(orderedArrays[3].data);
        }
        else
        {
            debug4 << "Adding " << orderedArrays[3].data->GetName()
                   << " as a zonal scalar field." << endl;
            outDS->GetCellData()->SetScalars(orderedArrays[3].data);
//            outDS->GetCellData()->AddArray(orderedArrays[3].data);
//            outDS->GetCellData()->SetActiveVariable(orderedArrays[3].data->GetName());
        }

        GetOutput()->GetInfo().GetAttributes().SetActiveVariable(
            orderedArrays[3].data->GetName());

#if 1
        if(!deleteArray4)
        {
            // The input dataset owns the data array. Make the output dataset
            // own it instead.
            if(var1Centering == AVT_NODECENT)
                inDS->GetPointData()->RemoveArray(orderedArrays[3].data->GetName());
            else
                inDS->GetCellData()->RemoveArray(orderedArrays[3].data->GetName());
        }
#endif
    }

    if(debug4_real)
        GetOutput()->GetInfo().GetAttributes().Print(debug4_real);

    // Clean up data arrays that we may have had to generate.
    if(deleteArray2)
        arr2->Delete();
    if(deleteArray3)
        arr3->Delete();
    if(deleteArray4)
        arr4->Delete();

    return outDS;
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Sun Mar 13 11:38:06 PST 2005
//    Fix memory leak.
//
//    Jeremy Meredith, Fri Apr  1 16:07:29 PST 2005
//    Fix UMR.
//
// ****************************************************************************

vtkDataSet *
avtScatterFilter::PointMeshFromVariables(DataInput *d1,
    DataInput *d2, DataInput *d3)
{
    vtkPolyData *outDS = vtkPolyData::New();
    int n = d1->data->GetNumberOfTuples();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(n);
    float *coord = (float *)pts->GetVoidPointer(0);
    vtkCellArray *cells = vtkCellArray::New();
    cells->Allocate(n); 
    outDS->SetPoints(pts);
    outDS->SetVerts(cells);
    pts->Delete(); cells->Delete();

    vtkDataArray *arr1 = d1->data;
    vtkDataArray *arr2 = d2->data;
    vtkDataArray *arr3 = d3->data;

    float xMin = arr1->GetTuple1(0);
    float xMax = xMin;
    float yMin = arr2->GetTuple1(0);
    float yMax = yMin;
    float zMin = 0., zMax = 0.;
    int nCells = 0;
    avtDataAttributes &dataAtts = GetOutput()->GetInfo().GetAttributes();
    const float EPSILON = 1.e-9;

    debug4 << "avtScatterFilter::PointMeshFromVariables: arr1 = "
           << arr1->GetName() << ", ntuples=" << arr1->GetNumberOfTuples()
           << endl;
    debug4 << "avtScatterFilter::PointMeshFromVariables: arr2 = "
           << arr2->GetName() << ", ntuples=" << arr2->GetNumberOfTuples()
           << endl;

    //
    // If arr3 == 0 then we're creating a 2D mesh.
    //
    if(arr3 == 0)
    {
        if(d1->useMin || d1->useMax || d2->useMin || d2->useMax)
        {
            float d1min = d1->min;
            float d1max = d1->max;
            float d2min = d2->min;
            float d2max = d2->max;
            for(vtkIdType i = 0; i < n; ++i)
            {
                coord[0] = arr1->GetTuple1(i);
                coord[1] = arr2->GetTuple1(i);
                coord[2] = 0.;

                // Do data min, max so we can set the min,max spatial extents.
                xMin = (xMin < coord[0]) ? xMin : coord[0];
                xMax = (xMax > coord[0]) ? xMax : coord[0];
                yMin = (yMin < coord[1]) ? yMin : coord[1];
                yMax = (yMax > coord[1]) ? yMax : coord[1];

                //
                // Only add values that are in the specified min/max range.
                //
                bool xInRange = true;
                if(d1->useMin)
                    xInRange = (coord[0] >= d1min);
                if(d1->useMax)
                    xInRange &= (coord[0] <= d1max);
                if(xInRange)
                {
                    bool yInRange = true;
                    if(d2->useMin)
                        yInRange = (coord[1] >= d2min);
                    if(d2->useMax)
                        yInRange &= (coord[1] <= d2max);
                    if(yInRange)
                    {
                        coord += 3;
                        cells->InsertNextCell(1, &nCells);
                        ++nCells;
                    }            
                }
            }
            pts->SetNumberOfPoints(nCells);

            // Use the specified limits if they were provided.
            xMin = d1->useMin ? d1min : xMin;
            xMax = d1->useMax ? d1max : xMax;
            yMin = d2->useMin ? d2min : yMin;
            yMax = d2->useMax ? d2max : yMax;
        }
        else
        {
            nCells = n;
            for(vtkIdType i = 0; i < n; ++i, coord += 3)
            {
                coord[0] = arr1->GetTuple1(i);
                coord[1] = arr2->GetTuple1(i);
                coord[2] = 0.;

                // Do data min, max so we can set the min,max spatial extents.
                xMin = (xMin < coord[0]) ? xMin : coord[0];
                xMax = (xMax > coord[0]) ? xMax : coord[0];
                yMin = (yMin < coord[1]) ? yMin : coord[1];
                yMax = (yMax > coord[1]) ? yMax : coord[1];
                cells->InsertNextCell(1, &i);
            }
        }
    }
    else
    {
        debug4 << "avtScatterFilter::PointMeshFromVariables: arr3 = "
               << arr3->GetName() << ", ntuples=" << arr3->GetNumberOfTuples()
               << endl;

        if(d1->useMin || d1->useMax ||
           d2->useMin || d2->useMax ||
           d3->useMin || d3->useMax)
        {
            float d1min = d1->min;
            float d1max = d1->max;
            float d2min = d2->min;
            float d2max = d2->max;
            float d3min = d3->min;
            float d3max = d3->max;
            for(vtkIdType i = 0; i < n; ++i)
            {
                coord[0] = arr1->GetTuple1(i);
                coord[1] = arr2->GetTuple1(i);
                coord[2] = arr3->GetTuple1(i);

                //
                // Only add values that are in the specified min/max range.
                //
                bool xInRange = true;
                if(d1->useMin)
                    xInRange = (coord[0] >= d1min);
                if(d1->useMax)
                    xInRange &= (coord[0] <= d1max);
                if(xInRange)
                {
                    bool yInRange = true;
                    if(d2->useMin)
                        yInRange = (coord[1] >= d2min);
                    if(d2->useMax)
                        yInRange &= (coord[1] <= d2max);
                    if(yInRange)
                    {
                        bool zInRange = true;
                        if(d3->useMin)
                            zInRange = (coord[2] >= d3min);
                        if(d3->useMax)
                            zInRange &= (coord[2] <= d3max);
                        if(zInRange)
                        {
                            coord += 3;
                            cells->InsertNextCell(1, &nCells);
                            ++nCells;
                        }
                    }            
                }

                // Do data min, max so we can set the min,max spatial extents.
                xMin = (xMin < coord[0]) ? xMin : coord[0];
                xMax = (xMax > coord[0]) ? xMax : coord[0];
                yMin = (yMin < coord[1]) ? yMin : coord[1];
                yMax = (yMax > coord[1]) ? yMax : coord[1];
                zMin = (zMin < coord[2]) ? zMin : coord[2];
                zMax = (zMax > coord[2]) ? zMax : coord[2];
            }
            pts->SetNumberOfPoints(nCells);

            // Use the specified limits if they were provided.
            xMin = d1->useMin ? d1min : xMin;
            xMax = d1->useMax ? d1max : xMax;
            yMin = d2->useMin ? d2min : yMin;
            yMax = d2->useMax ? d2max : yMax;
            zMin = d3->useMin ? d3min : zMin;
            zMax = d3->useMax ? d3max : zMax;
        }
        else
        {
            nCells = n;
            for(vtkIdType i = 0; i < n; ++i, coord += 3)
            {
                coord[0] = arr1->GetTuple1(i);
                coord[1] = arr2->GetTuple1(i);
                coord[2] = arr3->GetTuple1(i);

                // Do data min, max so we can set the min,max spatial extents.
                xMin = (xMin < coord[0]) ? xMin : coord[0];
                xMax = (xMax > coord[0]) ? xMax : coord[0];
                yMin = (yMin < coord[1]) ? yMin : coord[1];
                yMax = (yMax > coord[1]) ? yMax : coord[1];
                zMin = (zMin < coord[2]) ? zMin : coord[2];
                zMax = (zMax > coord[2]) ? zMax : coord[2];
                cells->InsertNextCell(1, &i);
            }
        }
    }

    //
    // Scale the coordinate fields.
    //
    int d1scale = d1->scale;
    int d2scale = d2->scale;
    int d3scale = d3->scale;
    if(d1scale > 0 || d2scale > 0 || d3scale > 0)
    {    
        coord = (float *)pts->GetVoidPointer(0);

        // Variables for log scaling
        float small_val = 0.1;  // Less than 1.
        float log_smallval = log10(small_val);

        // Variables for skew scaling.
        float x_range, x_rangeInverse, x_logSkew, x_k;
        if(d1scale == 2)
        {
            x_range = xMax - xMin; 
            x_rangeInverse = 1. / x_range;
            x_logSkew = log(d1->skew);
            x_k = x_range / (d1->skew - 1.);
        }
        float y_range, y_rangeInverse, y_logSkew, y_k;
        if(d2scale == 2)
        {
            y_range = yMax - yMin; 
            y_rangeInverse = 1. / y_range;
            y_logSkew = log(d2->skew);
            y_k = y_range / (d2->skew - 1.);
        }

#if 0
#define LOG10_X(X) log10((X) - xMin + small_val) - log_smallval;
#define LOG10_Y(Y) log10((Y) - yMin + small_val) - log_smallval;
#define LOG10_Z(Z) log10((Z) - zMin + small_val) - log_smallval;
#else
#define LOG10_X(X) log10(X)
#define LOG10_Y(Y) log10(Y)
#define LOG10_Z(Z) log10(Z)
#endif

        if(arr3 == 0)
        {
            for(int i = 0; i < nCells; ++i, coord += 3)
            {
                // Scale X coordinate
                if(d1scale == 1)
                    coord[0] = LOG10_X(coord[0]);
                else if(d1scale == 2)
                {
                    float tmp = (coord[0] - xMin) * x_rangeInverse;
                    coord[0] = x_k * (exp(tmp * x_logSkew) - 1.) + xMin;
                }

                // Scale Y coordinate
                if(d2scale == 1)
                    coord[1] = LOG10_Y(coord[1]);
                else if(d2scale == 2)
                {
                    float tmp = (coord[1] - yMin) * y_rangeInverse;
                    coord[1] = y_k * (exp(tmp * y_logSkew) - 1.) + yMin;
                }
            }
        }
        else
        {
            float z_range, z_rangeInverse, z_logSkew, z_k;
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
                    float tmp = (coord[0] - xMin) * x_rangeInverse;
                    coord[0] = x_k * (exp(tmp * x_logSkew) - 1.) + xMin;
                }

                // Scale Y coordinate
                if(d2scale == 1)
                    coord[1] = LOG10_Y(coord[1]);
                else if(d2scale == 2)
                {
                    float tmp = (coord[1] - yMin) * y_rangeInverse;
                    coord[1] = y_k * (exp(tmp * y_logSkew) - 1.) + yMin;
                }

                // Scale Z Coordinate
                if(d3scale == 1)
                    coord[2] = LOG10_Z(coord[2]);
                else if(d3scale == 2)
                {
                    float tmp = (coord[2] - zMin) * z_rangeInverse;
                    coord[2] = z_k * (exp(tmp * z_logSkew) - 1.) + zMin;
                }
            }
        }

        //
        // Since Log scaling does not preserve min,max values like
        // linear and skew scaling, we must transform the min,max.
        //
        if(d1scale == 1)
        {
            float newXmin = LOG10_X(xMin);
            float newXmax = LOG10_X(xMax);
            xMin = newXmin; xMax = newXmax;
        }

        if(d2scale == 1)
        {
            float newYmin = LOG10_Y(yMin);
            float newYmax = LOG10_Y(yMax);
            yMin = newYmin; yMax = newYmax;
        }

        if(arr3 != 0 && d3scale == 1)
        {
            float newZmin = LOG10_Z(zMin);
            float newZmax = LOG10_Z(zMax);
            zMin = newZmin; zMax = newZmax;
        }
    }

    //
    // Scale the coordinates so they are within a [0,1] cube.
    //
    if(atts.GetScaleCube())
    {
        const float boxSize = 1.f;
        float dX = xMax - xMin;
        float dY = yMax - yMin;
        float x_rangeInverse = (dX == 0.f) ? 1.f : (boxSize / dX);
        float y_rangeInverse = (dY == 0.f) ? 1.f : (boxSize / dY);
        coord = (float *)pts->GetVoidPointer(0);
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
            float dZ = zMax - zMin;
            float z_rangeInverse = (dZ == 0.f) ? 1.f : (boxSize / dZ);
            for(int i = 0; i < nCells; ++i, coord += 3)
            {
                coord[0] = (coord[0] - xMin) * x_rangeInverse;
                coord[1] = (coord[1] - yMin) * y_rangeInverse;
                coord[2] = (coord[2] - zMin) * z_rangeInverse;
            }
        }

        // Min, max of scaled coordinates.
        xMin = yMin = zMin = 0.f;
        xMax = yMax = zMax = 1.f;
    }

    //
    // We need some way to set some labels or something into the axes because
    // the values that will be displayed are not right because we've transformed
    // the points to get a better spatial layout.
    //

#if 1
    //
    // Clear the cumulative true spatial extents and the true spatial extents.
    //
    dataAtts.GetCumulativeTrueSpatialExtents()->Clear();
    dataAtts.GetTrueSpatialExtents()->Clear();
#else
    //
    // Set the spatial extents.
    //
    if(arr3 == 0) 
    {
        avtExtents newSE(2);
        float dX = xMax - xMin;
        float dY = yMax - yMin;
        if(dX < EPSILON && dY < EPSILON)
        {
            xMax += 1.;
            yMax += 1.;
        }
        else if(dX < EPSILON)
        {
            xMax += dY;
        }
        else if(dY < EPSILON)
            yMax += dX;
        const double de[] = {xMin, xMax, yMin, yMax};
        newSE.Set(de);
        *se = newSE;
    }
    else
    {
        avtExtents *se = GetOutput()->GetInfo().GetAttributes().GetCumulativeTrueSpatialExtents();
        avtExtents newSE(3);
        float dX = xMax - xMin;
        float dY = yMax - yMin;
        float dZ = zMax - zMin;
        if(dX < EPSILON && dY < EPSILON && dZ < EPSILON)
        {
            xMax += 1.;
            yMax += 1.;
            zMax += 1.;
        }
        else
        {
            float halfDist = 0.5 * sqrt(dX*dX + dY*dY + dZ*dZ);
            if(dX < EPSILON)
                xMax += halfDist;
            if(dY < EPSILON)
                yMax += halfDist;
            if(dZ < EPSILON)
                zMax += halfDist;
        }
        const double de[] = {xMin, xMax, yMin, yMax, zMin, zMax};
        newSE.Set(de);
        *se = newSE;
    }
#endif

    ManageMemory(outDS);
    outDS->Delete();

    return outDS;
}

// ****************************************************************************
//  Method: avtScatterFilter::RefashionDataObjectInfo
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
// ****************************************************************************

void
avtScatterFilter::RefashionDataObjectInfo(void)
{
debug4 << "avtScatterFilter::RefashionDataObjectInfo" << endl;
    avtDataAttributes &dataAtts = GetOutput()->GetInfo().GetAttributes();
    std::string v1Units(""), v2Units(""), v3Units(""), v4Units("");

    if(atts.GetVar1Role() != ScatterAttributes::None)
    {    
        if (dataAtts.ValidVariable(variableName.c_str()))
        {
            v1Units = dataAtts.GetVariableUnits(variableName.c_str());
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
    labels[int(atts.GetVar1Role())] = variableName.c_str();
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
        pd2cd->SetInput(ds2);
        vtkDataSet *ds3 = pd2cd->GetOutput();
        ds3->Update();
        outv = ds3->GetCellData()->GetScalars();
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
        cd2pd->SetInput(ds2);
        vtkDataSet *ds3 = cd2pd->GetOutput();
        ds3->Update();
        outv = ds3->GetPointData()->GetScalars();
        outv->Register(NULL);
        cd2pd->Delete();
    }

    ds2->Delete();
    return outv;
}

#if 0
// ****************************************************************************
//  Method: avtScatterFilter::PerformRestriction
//
//  Purpose:
//    Turn on Zone numbers flag so that the database does not make
//    any assumptions. 
//
//  Programmer: Brad Whitlock
//  Creation:   March 25, 2002
//
// ****************************************************************************
 
avtPipelineSpecification_p
avtScatterFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec);
    rv->GetDataSpecification()->TurnZoneNumbersOn();
    return rv;
}


#endif
