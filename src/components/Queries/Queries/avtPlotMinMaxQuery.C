// ************************************************************************* //
//                           avtPlotMinMaxQuery.C                            //
// ************************************************************************* //

#include <avtPlotMinMaxQuery.h>

#include <snprintf.h>

#include <vector>
#include <float.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkVisItUtility.h>

#include <avtCondenseDatasetFilter.h>
#include <avtMatrix.h>
#include <avtParallel.h>
#include <avtQueryableSource.h>
#include <avtTerminatingSource.h>
#include <avtSourceFromAVTDataset.h>
#include <avtVector.h>

#include <NonQueryableInputException.h>
#include <DebugStream.h>

#ifdef PARALLEL
#include <mpi.h>
#include <BufferConnection.h>
#endif

using std::vector;
using std::string;

float ComputeMajorEigenvalue(float *);

// ****************************************************************************
//  Method: avtPlotMinMaxQuery::avtPlotMinMaxQuery
//
//  Purpose:
//      Construct an avtPlotMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 27, 2003
//
//  Modifications:
//
// ****************************************************************************

avtPlotMinMaxQuery::avtPlotMinMaxQuery()
{
    dimension = 3;
    topoDim = 2;
    blockOrigin = 0;
    cellOrigin = 0;
    invTransform = NULL;
    condense = new avtCondenseDatasetFilter;
    condense->KeepAVTandVTK(true);
    singleDomain = true;
}

// ****************************************************************************
//  Method: avtPlotMinMaxQuery::~avtPlotMinMaxQuery
//
//  Purpose:
//      Destruct an avtPlotMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 27, 2003 
//
//  Modifications:
//
// ****************************************************************************

avtPlotMinMaxQuery::~avtPlotMinMaxQuery()
{
    if (condense != NULL)
    {
        delete condense;
        condense = NULL;
    }
}


// ****************************************************************************
//  Method: avtPlotMinMaxQuery::VerifyInput
//
//  Purpose:
//    Verify a new input.  Overrides base class in order to allow vectors
//    (topo dim == 0) to be queried.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 19, 2003 
//
// ****************************************************************************

void
avtPlotMinMaxQuery::VerifyInput()
{
    if (!GetInput()->GetInfo().GetValidity().GetQueryable())
    {
        EXCEPTION0(NonQueryableInputException);
    }
}


// ****************************************************************************
//  Method: avtPlotMinMaxQuery::Execute
//
//  Purpose:
//    Retrieves var information from the dataset, based on a domain and zone.
//
//  Arguments:
//    ds          The input dataset.
//    dom         The domain number.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 27, 2003
//
//  Modifications:
//
// ****************************************************************************

void 
avtPlotMinMaxQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL)
    {
        return;
    }

    bool nodeCentered;
    vtkDataArray *data = NULL;
    string var = queryAtts.GetVariables()[0];
    int varType = queryAtts.GetVarTypes()[0];
    scalarCurve = false;
    if ((data = ds->GetPointData()->GetArray(var.c_str())) != NULL)
    {
        nodeCentered = true;
        elementName = "node";
    }
    else if ((data = ds->GetCellData()->GetArray(var.c_str())) != NULL)
    {
        nodeCentered = false;
        elementName = "zone";
    }
    else if (varType == QueryAttributes::Curve) 
    {
        data = vtkVisItUtility::GetPoints(ds)->GetData();
        nodeCentered = true;
        elementName = "node";
    }
    else if ((varType == QueryAttributes::Scalar) && (topoDim == 1))
    {
        //
        //  This allows Lineouts to be queried for minMax.
        //
        data = vtkVisItUtility::GetPoints(ds)->GetData();
        nodeCentered = true;
        elementName = "node";
        scalarCurve = true;
    }
    else 
    {
        debug5 << "avtPlotMinMaxQuery could not find a vtkDataArray"
               << " associated with var " << var.c_str() << endl;
        return;    
    }

    float val;
    float *x;
    bool haveMin = false;
    bool haveMax = false;
    for (int elNum = 0; elNum < data->GetNumberOfTuples(); elNum++)
    {
        switch(varType)
        {
            case QueryAttributes::Scalar :
                if (!scalarCurve)
                    val = data->GetComponent(elNum, 0);
                else 
                    val = data->GetComponent(elNum, 1);
                break; 
            case QueryAttributes::Vector :
                x = data->GetTuple3(elNum);
                val = sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
                break; 
            case QueryAttributes::Tensor :
            case QueryAttributes::Symmetric_Tensor :
                x = data->GetTuple9(elNum);
                val = ComputeMajorEigenvalue(x);
                break; 
            case QueryAttributes::Curve :
                val = data->GetComponent(elNum, 1);
                break; 
            default:
                val = data->GetComponent(elNum, 0);
                break; 
        }
        if (val < minVal)
        {
             minElementNum = elNum;
             minVal = val;
             haveMin = true;
        } 
        if (val > maxVal)
        {
            maxElementNum = elNum;
            maxVal = val;
            haveMax = true;
        } 
    }

    if (haveMin)
    {
        if (nodeCentered)
        {
            GetNodeCoord(ds, minElementNum, minCoord);
            //
            // Indicate that the db needs to supply the correct
            // node number.
            //
            if (!scalarCurve)
                minElementNum = -1;
        } 
        else
        {
            GetCellCoord(ds, minElementNum, minCoord);
            vtkDataArray *origCells = 
                     ds->GetCellData()->GetArray("avtOriginalCellNumbers"); 
            if (origCells)
            {
                int comp = origCells->GetNumberOfComponents() -1;
                minElementNum = (int)
                        origCells->GetComponent(minElementNum, comp);
            }
            else 
            {
                //
                // Indicate that the db needs to supply the correct
                // cell number.
                //
                if (!scalarCurve)
                    minElementNum = -1;
            }
        } 
    }
    if (haveMax)
    {
        if (nodeCentered)
        {
            GetNodeCoord(ds, maxElementNum, maxCoord);
            if (!scalarCurve)
                maxElementNum = -1;
        } 
        else
        {
            GetCellCoord(ds, maxElementNum, maxCoord);
                vtkDataArray *origCells = 
                     ds->GetCellData()->GetArray("avtOriginalCellNumbers"); 
            if (origCells)
            {
                int comp = origCells->GetNumberOfComponents() -1;
                maxElementNum = (int)
                        origCells->GetComponent(maxElementNum, comp);
            }
            else 
            {
                //
                // Indicate that the db needs to supply the correct
                // cell number.
                //
                if (!scalarCurve)
                    maxElementNum = -1;
            }
        } 
    }
    if (haveMin)
        minDomain = (dom < 0 ? 0 : dom);
    if (haveMax)
        maxDomain = (dom < 0 ? 0 : dom);
}


// ****************************************************************************
//  Method: avtPlotMinMaxQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 27, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtPlotMinMaxQuery::PostExecute(void)
{
    int hasMin, hasMax; 

    int ts = queryAtts.GetTimeStep(); 
    string var = queryAtts.GetVariables()[0];

    hasMin = (((minVal != FLT_MAX) && 
                  ThisProcessorHasMinimumValue(minVal)) ? 1 : 0);
    hasMax = (((maxVal != -FLT_MAX) && 
                  ThisProcessorHasMaximumValue(maxVal)) ? 1 : 0);

    if (hasMin)
    {
        if (invTransform != NULL)
        {
            avtVector v1(minCoord[0], minCoord[1], minCoord[2]);
            v1 = (*invTransform) * v1;
            minCoord[0] = v1.x;
            minCoord[1] = v1.y;
            minCoord[2] = v1.z;
        }
        if (!scalarCurve)
        src->FindElementForPoint(var.c_str(), ts, minDomain, 
                     elementName.c_str(), minCoord, minElementNum);
        CreateMinMessage();
    }
    if (hasMax)
    {
        if (invTransform != NULL)
        {
            avtVector v1(maxCoord[0], maxCoord[1], maxCoord[2]);
            v1 = (*invTransform) * v1;
            maxCoord[0] = v1.x;
            maxCoord[1] = v1.y;
            maxCoord[2] = v1.z;
        }
        if (!scalarCurve)
        src->FindElementForPoint(var.c_str(), ts, maxDomain, 
                     elementName.c_str(), maxCoord, maxElementNum);
        CreateMaxMessage();
    }

#ifdef PARALLEL
    int myRank, numProcs;
    int size, i;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);


    if (myRank == 0)
    {
        for (i = 1; i < numProcs; i++)
        {
            MPI_Status stat, stat2;
            MPI_Recv(&hasMin, 1, MPI_INT, MPI_ANY_SOURCE,
                MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if (hasMin)
            {
                MPI_Recv(&size, 1, MPI_INT, stat.MPI_SOURCE, MPI_ANY_TAG,
                             MPI_COMM_WORLD, &stat2);
                char *buf = new char[size];
                MPI_Recv(buf, size, MPI_CHAR, stat.MPI_SOURCE, MPI_ANY_TAG,
                             MPI_COMM_WORLD, &stat2);
                minMsg = buf;
                delete [] buf;
            }
            MPI_Recv(&hasMax, 1, MPI_INT, MPI_ANY_SOURCE,
                         MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if (hasMax)
            {
                MPI_Recv(&size, 1, MPI_INT, stat.MPI_SOURCE, MPI_ANY_TAG,
                             MPI_COMM_WORLD, &stat2);
                char *buf = new char[size];
                MPI_Recv(buf, size, MPI_CHAR, stat.MPI_SOURCE, MPI_ANY_TAG,
                             MPI_COMM_WORLD, &stat2);
                maxMsg = buf;
                delete [] buf;
            }
        } // for all procs
    }
    else
    {
        MPI_Send(&hasMin, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
        if (hasMin)
        {
            size = minMsg.size();
            char *buf = new char[size];
            SNPRINTF(buf, size, minMsg.c_str());
            MPI_Send(&size, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
            MPI_Send(buf, size, MPI_CHAR, 0, myRank, MPI_COMM_WORLD);
            delete [] buf;
        }
        MPI_Send(&hasMax, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
        if (hasMax)
        {
            size = maxMsg.size();
            char *buf = new char[size];
            SNPRINTF(buf, size, maxMsg.c_str());
            MPI_Send(&size, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
            MPI_Send(buf, size, MPI_CHAR, 0, myRank, MPI_COMM_WORLD);
            delete [] buf;
        }
        return;
    }
#endif

    CreateResultMessage();
}


// ****************************************************************************
//  Method: avtPlotMinMaxQuery::PreExecute
//
//  Purpose:
//    This is called before any of the domains are executed.
//    Retrieves the correct spatial dimension, and resets certain values. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 27, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtPlotMinMaxQuery::PreExecute()
{
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
    dimension = data.GetSpatialDimension();
    topoDim = data.GetTopologicalDimension();
    blockOrigin = data.GetBlockOrigin();
    cellOrigin = data.GetCellOrigin();

    minVal = FLT_MAX;
    maxVal = -FLT_MAX;
    minElementNum = -1;
    maxElementNum = -1;
    minDomain = -1;
    maxDomain = -1;
    minCoord[0] = 0; // x
    minCoord[1] = 0; // y
    minCoord[2] = 0; // z
    maxCoord[0] = 0; // x
    maxCoord[1] = 0; // y
    maxCoord[2] = 0; // z
    minMsg = "No Information Found";
    maxMsg = "No Information Found";
    elementName = "";
}


// ****************************************************************************
//  Method: avtPlotMinMaxQuery::ApplyFilters
//
//  Purpose:
//    Retrieves the termnating source to use as input. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 27, 2003
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtPlotMinMaxQuery::ApplyFilters(avtDataObject_p inData)
{
    avtPipelineSpecification_p pspec = 
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();
    avtDataSpecification_p dspec = pspec->GetDataSpecification();

    src = inData->GetQueryableSource();
    intVector dlist;
    avtDataSpecification_p dspec2 = 
        inData->GetTerminatingSource()->GetFullDataSpecification();

    dspec2->GetSIL().GetDomainList(dlist);
    if (dlist.size() == 1 && dspec2->UsesAllDomains())
        singleDomain = true;
    else 
        singleDomain = false;

    avtDataAttributes &inAtts = inData->GetInfo().GetAttributes();
    if (inAtts.HasTransform() && inAtts.GetCanUseTransform())
    {
        invTransform = inAtts.GetTransform();
    }
    else 
    {
        invTransform = NULL;
    }

    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p obj = termsrc.GetOutput();
    condense->SetInput(obj);
    avtDataObject_p objOut = condense->GetOutput();
    objOut->Update(pspec);
    return objOut;

}


// ****************************************************************************
//  Method: avtPlotMinMaxQuery::GetNodeCoord
//
//  Purpose:
//    Retrieves the coordinate for the specified node id. 
//
//  Arguments:
//    ds        The dataset from which to retrieve the coordinate.
//    id        The id for the node.
//    coord     A place to store the node's coordinates.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtPlotMinMaxQuery::GetNodeCoord(vtkDataSet *ds, const int id, float coord[3])
{
    float *fp = ds->GetPoint(id);
    coord[0] = fp[0];
    coord[1] = fp[1];
    coord[2] = fp[2];
}


// ****************************************************************************
//  Method: avtPlotMinMaxQuery::GetCellCoord
//
//  Purpose:
//    Retrieves the coordinate (cell center) for the specified cell id.
//
//  Arguments:
//    ds        The dataset from which to retrieve the coordinate.
//    id        The id for the node.
//    coord     A place to store the cell's coordinates.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtPlotMinMaxQuery::GetCellCoord(vtkDataSet *ds, const int id, float coord[3])
{
    vtkCell *cell = ds->GetCell(id);
    float parametricCenter[3];
    float *weights = new float[cell->GetNumberOfPoints()];
    int subId = cell->GetParametricCenter(parametricCenter);
    cell->EvaluateLocation(subId, parametricCenter, coord, weights);
    delete [] weights;
}


// ****************************************************************************
//  Method: avtPlotMinMaxQuery::CreateMinMessage
//
//  Purpose:
//    Creates a string containing information about the minimum value. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtPlotMinMaxQuery::CreateMinMessage()
{
    char buff[256]; 
    if (strcmp(elementName.c_str(), "zone") == 0)
        minElementNum += cellOrigin;
 
    minMsg = queryAtts.GetVariables()[0] + " -- Min = ";
    SNPRINTF(buff, 256, "%f (%s %d ", minVal, 
             elementName.c_str(), minElementNum);
    minMsg += buff; 

    if (!singleDomain)
    {
        SNPRINTF(buff, 256, "in domain %d ", minDomain+blockOrigin);
        minMsg += buff;
    }

    if (queryAtts.GetVarTypes()[0] == QueryAttributes::Curve || scalarCurve)
    { 
        SNPRINTF(buff, 256, "at coord <%f>)", minCoord[0]);
    }
    else if (dimension == 2)
    {
        SNPRINTF(buff, 256, "at coord <%f, %f>)", minCoord[0], 
                 minCoord[1]);
    }
    else
    {
        SNPRINTF(buff, 256, "at coord <%f, %f, %f>)", minCoord[0], 
                 minCoord[1], minCoord[2]);
    }
    minMsg += buff;
}

// ****************************************************************************
//  Method: avtPlotMinMaxQuery::CreateMaxMessage
//
//  Purpose:
//    Creates a string containing information about the maximum value. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtPlotMinMaxQuery::CreateMaxMessage()
{
    char buff[256]; 
    if (strcmp(elementName.c_str(), "zone") == 0)
        maxElementNum += cellOrigin;
 
    maxMsg = queryAtts.GetVariables()[0] + " -- Max = ";
    SNPRINTF(buff, 256, "%f (%s %d ", maxVal, elementName.c_str(), 
             maxElementNum);
    maxMsg += buff; 

    if (!singleDomain)
    {
        SNPRINTF(buff, 256, "in domain %d ", maxDomain+blockOrigin);
        maxMsg += buff;
    }

    if (queryAtts.GetVarTypes()[0] == QueryAttributes::Curve || scalarCurve)
    { 
        SNPRINTF(buff, 256, "at coord <%f>)", maxCoord[0]);
    }
    else if (dimension == 2)
    {
        SNPRINTF(buff, 256, "at coord <%f, %f>)", maxCoord[0], 
                 maxCoord[1]);
    }
    else
    {
        SNPRINTF(buff, 256, "at coord <%f, %f, %f>)", maxCoord[0], 
                 maxCoord[1], maxCoord[2]);
    }
    maxMsg += buff;
}

// ****************************************************************************
//  Method: avtPlotMinMaxQuery::CreateMaxMessage
//
//  Purpose:
//    Concatenates the Min and Max messages.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtPlotMinMaxQuery::CreateResultMessage()
{
    string msg = "\n";
    doubleVector vals;
    msg += minMsg + "\n" + maxMsg + "\n\n";
    vals.push_back(minVal);
    vals.push_back(maxVal);

    SetResultMessage(msg);
    SetResultValues(vals);
}

float
ComputeMajorEigenvalue(float *vals)
{
    float *input[3];
    float row1[3];
    float row2[3];
    float row3[3];
    input[0] = row1;
    input[1] = row2;
    input[2] = row3;
    input[0][0] = vals[0];
    input[0][1] = vals[1];
    input[0][2] = vals[2];
    input[1][0] = vals[3];
    input[1][1] = vals[4];
    input[1][2] = vals[5];
    input[2][0] = vals[6];
    input[2][1] = vals[7];
    input[2][2] = vals[8];
    float *eigenvecs[3];
    float outrow1[3];
    float outrow2[3];
    float outrow3[3];
    eigenvecs[0] = outrow1;
    eigenvecs[1] = outrow2;
    eigenvecs[2] = outrow3;
    float eigenvals[3];
    vtkMath::Jacobi(input, eigenvals, eigenvecs);
    return eigenvals[0];
}
