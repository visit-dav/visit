// ************************************************************************* //
//                             avtGradientFilter.C                           //
// ************************************************************************* //

#include <avtGradientFilter.h>

#include <math.h>
#include <vector>

#include <vtkCellDataToPointData.h>
#include <vtkCellDerivatives.h> 
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtGradientFilter constructor
//
//  Programmer: Akira Haddox <Transferred by Hank Childs>
//  Creation:   August 18, 2002
//
// ****************************************************************************

avtGradientFilter::avtGradientFilter()
{
    myFilter = vtkCellDataToPointData::New();
}


// ****************************************************************************
//  Method: avtGradientFilter destructor
//
//  Programmer: Akira Haddox <Transferred by Hank Childs>
//  Creation:   August 18, 2002
//
// ****************************************************************************

avtGradientFilter::~avtGradientFilter()
{
    if (myFilter != NULL)
    {
        myFilter->Delete();
        myFilter = NULL;
    }
}


// ****************************************************************************
//  Method: avtGradientFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Matthew Haddox 
//  Creation:     July 30, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Nov 13 13:55:52 PST 2002
//    Fixed memory mismanagement issue that caused a crash on successive calls
//    and fixed memory leak.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Use NewInstance instead of MakeObject, new vtkapi. 
// 
// ****************************************************************************

vtkDataArray *
avtGradientFilter::DeriveVariable(vtkDataSet *in_ds)
{
    vtkDataArray *scalarValues = in_ds->GetPointData()->GetScalars(); 
    bool deleteSVar = false;
    
    if (!scalarValues)
    {
        if (!(in_ds->GetCellData()->GetScalars()))
        {
            return NULL; // No data
        }

        myFilter->SetInput(in_ds);
        
        vtkDataSet *ds = (vtkDataSet*)in_ds->NewInstance();
        myFilter->SetOutput(ds);
        ds->Update();
        ds->SetSource(NULL);

        scalarValues = ds->GetPointData()->GetScalars();
        scalarValues->Register(NULL); // so we don't lose it
        deleteSVar = true;
        ds->Delete();
    }
    
    int nPoints = in_ds->GetNumberOfPoints();

    vtkDataArray *results = vtkFloatArray::New();
    results->SetNumberOfComponents(3);
    results->SetNumberOfTuples(nPoints);        
    
    for (int nodeId = 0 ; nodeId < nPoints; nodeId++)
    {
        float xDELTA=1e6, yDELTA=1e6, zDELTA=1e6;
        
        float *nodeCoords = in_ds->GetPoint(nodeId);
        float  nodeValue  = scalarValues->GetComponent(nodeId, 0);
        
        vtkIdList *neighborCellIds = vtkIdList::New();
        vtkIdList *myNodeId = vtkIdList::New();

        myNodeId->SetNumberOfIds(1);
        myNodeId->SetId(0, nodeId);
        
        in_ds->GetCellNeighbors(-1, myNodeId, neighborCellIds);

        myNodeId->Delete();

        int nCells=neighborCellIds->GetNumberOfIds();

        // Find appropriate deltas
        for (int ci = 0 ; ci < nCells ; ci++)
        {
            float *bounds = in_ds->GetCell(neighborCellIds->GetId(ci))
                                                                 ->GetBounds();
            if (bounds[1]-bounds[0] < xDELTA*5)
            {
                xDELTA = (bounds[1]-bounds[0]) / 5.0;
            }
            if (bounds[3]-bounds[2] < yDELTA*5)
            {
                yDELTA = (bounds[3]-bounds[2]) / 5.0;
            }
            if (bounds[5]-bounds[4] < zDELTA*5)
            {
                zDELTA = (bounds[5]-bounds[4]) / 5.0;
            }
        }
        
        float xComponent, yComponent, zComponent;
        
        xComponent=EvaluateComponent(nodeCoords[0],nodeCoords[1],nodeCoords[2],
                xDELTA, 0, 0, nodeValue, in_ds, scalarValues, neighborCellIds);
        yComponent=EvaluateComponent(nodeCoords[0],nodeCoords[1],nodeCoords[2],
                0, yDELTA, 0, nodeValue, in_ds, scalarValues, neighborCellIds);
        zComponent=EvaluateComponent(nodeCoords[0],nodeCoords[1],nodeCoords[2],
                0, 0, zDELTA, nodeValue, in_ds, scalarValues, neighborCellIds);

        results->SetTuple3(nodeId, xComponent, yComponent, zComponent);
    }
    
    if (deleteSVar)
        scalarValues->Delete();
    
    return results;
}


// ****************************************************************************
//  Method: avtGradientFilter::EvaluateComponent
//
//  Purpose:
//      Calculate the directional derivative from the given point in the given
//      deltas.
//
//  Arguments:
//    x,y,z           The point.
//    dx, dy, dz      The direction, in delta scale: It is indended for (2/3)
//                    of them to be 0.
//    value           The value at the point.
//    in_ds           The dataset.
//    scalarValues    The values for the dataset.
//    neighborCells   The cells to look in.
//
//  Returns:      The value at that point.
//
//  Programmer:   Matthew Haddox 
//  Creation:     July 30, 2002
//
// ****************************************************************************


float avtGradientFilter::EvaluateComponent(float x, float y, float z,
            float dx, float dy, float dz, float value, vtkDataSet *in_ds,
            vtkDataArray *scalarValues, vtkIdList *neighborCells)
{
    int deltaMultiplier = 2;
    float upper;
    bool  success = true;

    upper = EvaluateValue(x+dx, y+dy, z+dz, in_ds, scalarValues, neighborCells,
                        success);
    if (!success)
    {
        upper=value;
        --deltaMultiplier;
    }

    float lower;
    success = true;
    lower = EvaluateValue(x-dx, y-dy, z-dz, in_ds, scalarValues, neighborCells,
                          success);

    if (!success)
    {
        lower=value;
        --deltaMultiplier;
    }

    if (!deltaMultiplier)
        return 0;

    return (upper-lower)/ (double(deltaMultiplier)*(dx+dy+dz));
}


// ****************************************************************************
//  Method: avtGradientFilter::EvaluateValue
//
//  Purpose:
//      Calculate the value at a point.
//
//  Arguments:
//    x,y,z           The point.
//    in_ds           The dataset.
//    scalarValues    The values for the dataset.
//    neighborCells   The cells to look in.
//    success         Set to false if there is a problem.
//
//  Returns:      The value at that point.
//
//  Programmer:   Matthew Haddox 
//  Creation:     July 30, 2002
//
// ****************************************************************************


float avtGradientFilter::EvaluateValue(float x, float y, float z, 
                               vtkDataSet *in_ds, vtkDataArray *scalarValues,
                               vtkIdList *neighborCells, bool &success)
{
    // Find which cell contains this point
    float coords[3] = {x,y,z};
    int   junk2;
    float junk3[3];
    float junk4;
    float weights[8];  // This needs to be the max number of points a cell has

    float *abnormalWeights = NULL; // In case of more than 8 points
    
    int cellId;
    vtkCell *c;
    for (cellId = 0 ; cellId < neighborCells->GetNumberOfIds() ; cellId++)
    {
        c = in_ds->GetCell(neighborCells->GetId(cellId));
    
        if (c->GetNumberOfPoints() > 8)
        {
            abnormalWeights = new float[c->GetNumberOfPoints()];
            if (c->EvaluatePosition(coords, NULL, junk2, junk3, junk4, 
                                    abnormalWeights) == 1)
            {
                break;
            }
            
            delete[] abnormalWeights;
            abnormalWeights=NULL;
        }
        
        else if (c->EvaluatePosition(coords, NULL, junk2, junk3, junk4, 
                                     weights) == 1)
        {
            break;
        }
    }

    if (cellId == neighborCells->GetNumberOfIds())
    {
        success = false;
        return 0.;
    }
    
    float value = 0.;
    if (abnormalWeights)
    {
        for (int k=0 ; k < c->GetNumberOfPoints() ; k++)
        {
            int pt = c->GetPointId(k);
            value += abnormalWeights[k] * scalarValues->GetComponent(pt,0);
        }
        delete [] abnormalWeights;
    }
    else
    {
        for (int k = 0 ; k < c->GetNumberOfPoints() ; k++)
        {
            int pt = c->GetPointId(k);
            value += weights[k] * scalarValues->GetComponent(pt, 0);
        }
    }
    return value;    
} 


