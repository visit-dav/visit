// ************************************************************************* //
//                              avtMinMaxQuery.C                             //
// ************************************************************************* //

#include <avtMinMaxQuery.h>

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
#include <vtkUnsignedCharArray.h>
#include <vtkVisItUtility.h>

#include <avtCommonDataFunctions.h>
#include <avtMatrix.h>
#include <avtParallel.h>
#include <avtQueryableSource.h>
#include <avtTerminatingSource.h>
#include <avtVector.h>

#include <NonQueryableInputException.h>
#include <DebugStream.h>

#ifdef PARALLEL
#include <mpi.h>
#include <BufferConnection.h>
#endif

using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtMinMaxQuery::avtMinMaxQuery
//
//  Purpose:
//      Construct an avtMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 27, 2003
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004
//    Added args, which control whether we do the Min or Max (or both).
//
// ****************************************************************************

avtMinMaxQuery::avtMinMaxQuery(bool domin, bool domax)
{
    dimension = 3;
    topoDim = 2;
    blockOrigin = 0;
    cellOrigin = 0;
    invTransform = NULL;
    singleDomain = true;
    doMin = domin;
    doMax = domax;
}

// ****************************************************************************
//  Method: avtMinMaxQuery::~avtMinMaxQuery
//
//  Purpose:
//      Destruct an avtMinMaxQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 27, 2003 
//
//  Modifications:
//
// ****************************************************************************

avtMinMaxQuery::~avtMinMaxQuery()
{
}


// ****************************************************************************
//  Method: avtMinMaxQuery::VerifyInput
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
avtMinMaxQuery::VerifyInput()
{
    if (!GetInput()->GetInfo().GetValidity().GetQueryable())
    {
        EXCEPTION0(NonQueryableInputException);
    }
}


// ****************************************************************************
//  Method: avtMinMaxQuery::Execute
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
//    Kathleen Bonnell, Mon Dec 22 14:48:57 PST 2003
//    Test for ghost zones. Changed min/max val check to <= or >= so that
//    serial and parallel versions will always return the same results.
//
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004 
//    Only check min/or max if they are set to be done. 
//
//    Kathleen Bonnell, Thu May  6 17:36:43 PDT 2004 
//    If working with OriginalData, or zones have been preserved, use the
//    zone number found here, rather than querying the database for it.
//
// ****************************************************************************

void 
avtMinMaxQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL)
    {
        return;
    }

    vtkUnsignedCharArray *ghosts = 
        (vtkUnsignedCharArray*)ds->GetCellData()->GetArray("vtkGhostLevels");
    bool nodeCentered;
    vtkDataArray *data = NULL;
    string var = queryAtts.GetVariables()[0];
    int varType = queryAtts.GetVarTypes()[0];
    scalarCurve = false;
    bool checkGhost = false;
    if ((data = ds->GetPointData()->GetArray(var.c_str())) != NULL)
    {
        nodeCentered = true;
        elementName = "node";
    }
    else if ((data = ds->GetCellData()->GetArray(var.c_str())) != NULL)
    {
        nodeCentered = false;
        elementName = "zone";
        checkGhost = ghosts != NULL;
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
        debug5 << "avtMinMaxQuery could not find a vtkDataArray"
               << " associated with var " << var.c_str() << endl;
        return;    
    }

    float val;
    float *x;
    bool haveMin = false;
    bool haveMax = false;
    bool zonesPreserved = GetInput()->GetInfo().GetValidity().GetZonesPreserved();
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
                val = MajorEigenvalue(x);
                break; 
            case QueryAttributes::Curve :
                val = data->GetComponent(elNum, 1);
                break; 
            default:
                val = data->GetComponent(elNum, 0);
                break; 
        }
        bool ghost = false;
        if (checkGhost)
        {
            ghost = (ghosts->GetValue(elNum) > 0);
        }
        if (doMin && val <= minVal && !ghost)
        {
             minElementNum = elNum;
             minVal = val;
             haveMin = true;
        } 
        if (doMax && val >= maxVal && !ghost)
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
            if (!scalarCurve && !OriginalData())
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
                if (!scalarCurve && !OriginalData() && !zonesPreserved) 
                    minElementNum = -1;
            }
        } 
    }
    if (haveMax)
    {
        if (nodeCentered)
        {
            GetNodeCoord(ds, maxElementNum, maxCoord);
            if (!scalarCurve && !OriginalData())
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
                if (!scalarCurve && !OriginalData() && !zonesPreserved)
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
//  Method: avtMinMaxQuery::PostExecute
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
//    Kathleen Bonnell, Fri Dec 19 13:48:53 PST 2003
//    Change the order of the hasMin/hasMax tests so that it works correctly in
//    parallel when one of the processors hsas no data.
//
// ****************************************************************************

void
avtMinMaxQuery::PostExecute(void)
{
    int hasMin, hasMax; 

    int ts = queryAtts.GetTimeStep(); 
    string var = queryAtts.GetVariables()[0];

    hasMin = (ThisProcessorHasMinimumValue(minVal) && minVal != FLT_MAX);
    hasMax = (ThisProcessorHasMaximumValue(maxVal) && maxVal != -FLT_MAX);

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
        if (minElementNum == -1)
        {
            src->FindElementForPoint(var.c_str(), ts, minDomain, 
                     elementName.c_str(), minCoord, minElementNum);
        }
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
        if (maxElementNum == -1)
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
            size = minMsg.size()+1;
            char *buf = new char[size];
            SNPRINTF(buf, size, minMsg.c_str());
            MPI_Send(&size, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
            MPI_Send(buf, size, MPI_CHAR, 0, myRank, MPI_COMM_WORLD);
            delete [] buf;
        }
        MPI_Send(&hasMax, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
        if (hasMax)
        {
            size = maxMsg.size()+1;
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
//  Method: avtMinMaxQuery::PreExecute
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
avtMinMaxQuery::PreExecute()
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
    minCoord[0] = 0.; // x
    minCoord[1] = 0.; // y
    minCoord[2] = 0.; // z
    maxCoord[0] = 0.; // x
    maxCoord[1] = 0.; // y
    maxCoord[2] = 0.; // z
    minMsg = "No Information Found";
    maxMsg = "No Information Found";
    elementName = "";
}


// ****************************************************************************
//  Method: avtMinMaxQuery::Preparation
//
//  Purpose:
//    Preforms preparation tasks common to all derived types. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtMinMaxQuery::Preparation(avtDataObject_p inData)
{
    avtDataSpecification_p dspec = inData->GetTerminatingSource()
        ->GetGeneralPipelineSpecification()->GetDataSpecification();

    src = inData->GetQueryableSource();

    intVector dlist;
    dspec->GetSIL().GetDomainList(dlist);
    if (dlist.size() == 1 && dspec->UsesAllDomains())
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
}


// ****************************************************************************
//  Method: avtMinMaxQuery::GetNodeCoord
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
avtMinMaxQuery::GetNodeCoord(vtkDataSet *ds, const int id, float coord[3])
{
    float *fp = ds->GetPoint(id);
    coord[0] = fp[0];
    coord[1] = fp[1];
    coord[2] = fp[2];
}


// ****************************************************************************
//  Method: avtMinMaxQuery::GetCellCoord
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
avtMinMaxQuery::GetCellCoord(vtkDataSet *ds, const int id, float coord[3])
{
    vtkCell *cell = ds->GetCell(id);
    float parametricCenter[3];
    float *weights = new float[cell->GetNumberOfPoints()];
    int subId = cell->GetParametricCenter(parametricCenter);
    cell->EvaluateLocation(subId, parametricCenter, coord, weights);
    delete [] weights;
}


// ****************************************************************************
//  Method: avtMinMaxQuery::CreateMinMessage
//
//  Purpose:
//    Creates a string containing information about the minimum value. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec 22 14:48:57 PST 2003
//    Retrieve DomainName and use it in output, if available.
//
// ****************************************************************************

void
avtMinMaxQuery::CreateMinMessage()
{
    char buff[256]; 
    if (strcmp(elementName.c_str(), "zone") == 0)
        minElementNum += cellOrigin;

    string var = queryAtts.GetVariables()[0]; 
    minMsg = var + " -- Min = ";
    SNPRINTF(buff, 256, "%f (%s %d ", minVal, 
             elementName.c_str(), minElementNum);
    minMsg += buff; 
    if (!singleDomain)
    {
        string domainName;
        src->GetDomainName(var, queryAtts.GetTimeStep(), minDomain, domainName);
     
        if (domainName.size() > 0)
        { 
            minMsg += "in " + domainName + " " ;
        }
        else 
        { 
            SNPRINTF(buff, 256, "in domain %d ", minDomain+blockOrigin);
            minMsg += buff;
        }
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
//  Method: avtMinMaxQuery::CreateMaxMessage
//
//  Purpose:
//    Creates a string containing information about the maximum value. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec 22 14:48:57 PST 2003
//    Retrieve DomainName and use it in output, if available.
//
// ****************************************************************************

void
avtMinMaxQuery::CreateMaxMessage()
{
    char buff[256]; 
    if (strcmp(elementName.c_str(), "zone") == 0)
        maxElementNum += cellOrigin;
 
    string var = queryAtts.GetVariables()[0]; 
    maxMsg = var + " -- Max = ";
    SNPRINTF(buff, 256, "%f (%s %d ", maxVal, elementName.c_str(), 
             maxElementNum);
    maxMsg += buff; 

    if (!singleDomain)
    {
        string domainName;
        src->GetDomainName(var, queryAtts.GetTimeStep(), maxDomain, domainName);
     
        if (domainName.size() > 0)
        { 
            maxMsg += "in " + domainName + " ";
        }
        else 
        { 
            SNPRINTF(buff, 256, "in domain %d ", maxDomain+blockOrigin);
            maxMsg += buff;
        }
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
//  Method: avtMinMaxQuery::CreateMaxMessage
//
//  Purpose:
//    Concatenates the Min and Max messages.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 28, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 16:13:07 PST 2004
//    Modified so that either Min Or Max could be done separately.
//
// ****************************************************************************

void
avtMinMaxQuery::CreateResultMessage()
{
    string msg = "\n";
    if (doMin)
    {
        if (doMax)
        {
            doubleVector vals;
            msg += minMsg + "\n" + maxMsg + "\n\n";
            vals.push_back(minVal);
            vals.push_back(maxVal);
            SetResultValues(vals);
        }
        else 
        {
            msg += minMsg + "\n\n";
            SetResultValue(minVal);
        }
    }
    else 
    {
        msg += maxMsg + "\n\n";
        SetResultValue(maxVal);
    }
    SetResultMessage(msg);
}

