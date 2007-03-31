// ************************************************************************* //
//                             avtVariableQuery.C                           //
// ************************************************************************* //

#include <avtVariableQuery.h>

#include <snprintf.h>

#include <vector>
#include <float.h>
#include <vtkDataSet.h>

#include <avtTerminatingSource.h>
#include <PickVarInfo.h>

#include <DebugStream.h>

#ifdef PARALLEL
#include <mpi.h>
#include <BufferConnection.h>
#endif

using std::vector;
using std::string;



// ****************************************************************************
//  Method: avtVariableQuery::avtVariableQuery
//
//  Purpose:
//      Construct an avtVariableQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 23, 2003
//
//  Modifications:
//
// ****************************************************************************

avtVariableQuery::avtVariableQuery()
{
    searchDomain = 0;
    searchElement = 0;
}

// ****************************************************************************
//  Method: avtVariableQuery::~avtVariableQuery
//
//  Purpose:
//      Destruct an avtVariableQuery object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 23, 2003 
//
//  Modifications:
//
// ****************************************************************************

avtVariableQuery::~avtVariableQuery()
{
}



// ****************************************************************************
//  Method: avtVariableQuery::Execute
//
//  Purpose:
//    Retrieves var information from the dataset, based on a domain and zone.
//
//  Arguments:
//    ds          The input dataset.
//    dom         The domain number.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     July 23, 2003
//
//  Modifications:
//
// ****************************************************************************

void 
avtVariableQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (dom != searchDomain || pickAtts.GetFulfilled() || ds == NULL)
    {
        return;
    }

    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();

    int ts = dspec->GetTimestep();
    pickAtts.SetTimeStep(ts);
    pickAtts.SetActiveVariable(dspec->GetVariable());
    pickAtts.SetDomain(searchDomain);
    pickAtts.SetElementNumber(searchElement);
    if (queryAtts.GetElementType() == QueryAttributes::Zone)
        pickAtts.SetPickType(PickAttributes::Zone);
    else 
        pickAtts.SetPickType(PickAttributes::Node);

    pickAtts.SetVariables(queryAtts.GetVariables());

    intVector dlist;
    dspec->GetSIL().GetDomainList(dlist);

    GetInput()->GetTerminatingSource()->Query(&pickAtts);

    if (dlist.size() == 1 && dspec->UsesAllDomains())
    {
        //
        // Indicate that there was only one domain.
        // We don't report the domain number for single-domain problems.
        //
        pickAtts.SetDomain(-1);
    }
    else
        pickAtts.SetDomain(queryAtts.GetDomain());

    pickAtts.SetElementNumber(queryAtts.GetElement());
}

// ****************************************************************************
//  Method: avtVariableQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//    If in parallel, collects the correct pickAtts from the processor that
//    gathered the info, to processor 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
avtVariableQuery::PostExecute(void)
{
#ifdef PARALLEL
    int myRank, numProcs;
    int hasFulfilledPick;
    int size, i;
    BufferConnection b;
    unsigned char *buf;
 
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    if (myRank == 0)
    {
        for (i = 1; i < numProcs; i++)
        {
            MPI_Status stat, stat2;
            MPI_Recv(&hasFulfilledPick, 1, MPI_INT, MPI_ANY_SOURCE,
                     MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if (hasFulfilledPick)
            {
                MPI_Recv(&size, 1, MPI_INT, stat.MPI_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &stat2);
                buf = new unsigned char[size];
                MPI_Recv(buf, size, MPI_UNSIGNED_CHAR, stat.MPI_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &stat2);
                b.Append(buf, size);
                pickAtts.Read(b);
                delete [] buf;
            }
        }
    }
    else
    {
        hasFulfilledPick = (int) pickAtts.GetFulfilled();
        MPI_Send(&hasFulfilledPick, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
        if (hasFulfilledPick)
        {
            pickAtts.SelectAll();
            pickAtts.Write(b);
            size = pickAtts.CalculateMessageSize(b);
            buf = new unsigned char[size];
            for (int i = 0; i < size; ++i)
                b.Read(buf+i);
 
            MPI_Send(&size, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
            MPI_Send(buf, size, MPI_UNSIGNED_CHAR, 0, myRank, MPI_COMM_WORLD);
            delete [] buf;
        }
    }

#endif

    if (pickAtts.GetFulfilled())
    {
        // Special indication that the pick point should not be displayed.
        float cp[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
        string msg;
        pickAtts.SetCellPoint(cp);
        pickAtts.CreateOutputString(msg);
        SetResultMessage(msg.c_str());
    }
    else
    {
        char msg[120]; 
        SNPRINTF(msg, 120, "Could not retrieve information from domain "
                 " %d element %d.", queryAtts.GetDomain(), queryAtts.GetElement());
        SetResultMessage(msg);
    }
}


// ****************************************************************************
//  Method: avtVariableQuery::PreExecute
//
//  Purpose:
//    This is called before any of the domains are executed.
//    Retrieves the correct spatial dimension and sets it in pickAtts. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003
//
// ****************************************************************************

void
avtVariableQuery::PreExecute()
{
    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();

    pickAtts.SetDimension(data.GetSpatialDimension());
    searchDomain = queryAtts.GetDomain() - data.GetBlockOrigin();
    searchElement = queryAtts.GetElement(); 
    if (queryAtts.GetElementType() == QueryAttributes::Zone)
        searchElement -=  data.GetCellOrigin();
}


// ****************************************************************************
//  Method: avtVariableQuery::ApplyFilters
//
//  Purpose:
//    Retrieves the termnating source to use as input. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003
//
// ****************************************************************************

avtDataObject_p
avtVariableQuery::ApplyFilters(avtDataObject_p inData)
{
    return inData->GetTerminatingSource()->GetOutput();
}


