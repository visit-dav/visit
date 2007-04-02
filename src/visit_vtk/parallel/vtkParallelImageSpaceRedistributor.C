// ************************************************************************* //
//                    vtkParallelImageSpaceRedistributor.C                   //
// ************************************************************************* //

#include "vtkParallelImageSpaceRedistributor.h"

#include <stdlib.h>

#include <vtkAppendPolyData.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkCharArray.h>
#include <vtkDataSet.h>
#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkDataSetWriter.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataRelevantPointsFilter.h>

#include <ImproperUseException.h>
#include <NoInputException.h>
#include <TimingsManager.h>

#include <DebugStream.h>

#ifdef PARALLEL
#include <mpi.h>
#endif


using std::string;
using std::vector;

vtkStandardNewMacro(vtkParallelImageSpaceRedistributor);

// ****************************************************************************
//  Function:  AreaOwned
//
//  Purpose:
//    Returns the x,y lower (inclusive) limit and the x,y upper (exclusive)
//    limit on the region owned by any particular processor when doing
//    parallel scalable rendering with transparency and tiled image
//    compositing.
//
//  NOTE:  THIS CODE IS DUPLICATED IN avtTiledImageCompositor.C!!!
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  1, 2004
//
// ****************************************************************************
static void AreaOwned(int rank, int size, int w, int h,
                      int &x1,int &y1, int &x2,int &y2)
{
    x1 = 0;
    x2 = w-1;
    y1 = (h*rank)/size;
    y2 = ((h*(rank+1))/size);
}

// ****************************************************************************
//  Constructor: vtkParallelImageSpaceRedistributor()
//
//  Programmer: Chris Wojtan
//  Creation:   July 7, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 21 18:21:50 PDT 2004
//    Rewrote basic screen-division algorithm.
//
// ****************************************************************************
vtkParallelImageSpaceRedistributor::vtkParallelImageSpaceRedistributor()
{
    SetNumberOfOutputs(1);
    rank = 0;
    size = 1;
    x1 = x2 = y1 = y2 = 0;
}

// ****************************************************************************
//  Destructor: ~vtkParallelImageSpaceRedistributor()
//
//  Programmer: Jeremy Meredith
//  Creation:   October 21, 2004
//
// ****************************************************************************
vtkParallelImageSpaceRedistributor::~vtkParallelImageSpaceRedistributor()
{
    delete[] x1;
    delete[] x2;
    delete[] y1;
    delete[] y2;
}

// ****************************************************************************
// Method: vtkParallelImageSpaceRedistributor::SetRankAndSize
//
// Purpose: 
//   Sets the number of processors and the processor rank so that we don't
//   rely on external symbols for doing so.
//
// Arguments:
//   r : The processor rank.
//   s : The number of processors.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 1 15:12:33 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
vtkParallelImageSpaceRedistributor::SetRankAndSize(int r, int s)
{
    rank = r;
    size = s;
    x1 = new int[size];
    x2 = new int[size];
    y1 = new int[size];
    y2 = new int[size];
}

// ****************************************************************************
//  Method: vtkParallelImageSpaceRedistributor::GetOutput()
//
//  Purpose:
//      retrieves output from this filter
//
//  Programmer: Chris Wojtan
//  Creation:   July 7, 2004
//
// ****************************************************************************
vtkPolyData *
vtkParallelImageSpaceRedistributor::GetOutput()
{
    return (vtkPolyData *) vtkSource::GetOutput(0);
}


// ****************************************************************************
//  Method: vtkParallelImageSpaceRedistributor::GetOutput()
//
//  Purpose:
//      executes the actual filtering of the polydata
//
//  Programmer: Chris Wojtan
//  Creation:   July 7, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 21 18:21:50 PDT 2004
//    Rewrote basic screen-division algorithm.
//    Made some heavy optimizations.  Freed some memory.
//
//    Jeremy Meredith, Tue Oct 26 22:19:35 PDT 2004
//    Another major rewrite.  It was being naive about outgoing allocation
//    and couldn't survive very large datasets.  I refactored a bunch,
//    rewrote the parallel communication so it could use MPI_Alltoallv which
//    is much faster than point-to-point, made a two-pass algorithm to
//    count output cells for outgoing data before doing the allocations,
//    tried to keep as much in-place transformation as possible, and added
//    vtkPolyDataRelevantPointsFilter to prevent communicating needless
//    point data.
//
// ****************************************************************************
void
vtkParallelImageSpaceRedistributor::Execute(void)
{
#ifdef PARALLEL
    //
    // Do some basic setup -- collect some important data
    //
    if (ren == NULL)
    {
        EXCEPTION0(ImproperUseException)
    }

    int TH_total = visitTimer->StartTimer(); 

    width  = ren->GetSize()[0];
    height = ren->GetSize()[1];

    for (int i=0; i<size; i++)
    {
        AreaOwned(i, size, width, height, x1[i],y1[i],x2[i],y2[i]);
    }

    int   i, j;
    MPI_Status stat;

    vtkPolyData *input = GetInput();
    int ncells = input->GetNumberOfCells();
    vtkPointData *inPD       = input->GetPointData();
    vtkCellData  *inCD       = input->GetCellData();
    vtkIdType     npts       = 0;
    vtkIdType    *cellPts    = 0;

    input->BuildCells();


    //
    // Initialize outgoing datasets
    //
    vector<vtkPolyData *> outgoingPolyData;
    vector<int>           outgoingCellCount;
    outgoingPolyData.resize(size, NULL);
    outgoingCellCount.resize(size, 0);

    //
    // Transform the points
    //
    int TH_transform = visitTimer->StartTimer(); 
    vtkMatrix4x4 *worldToView = CreateWorldToDisplayMatrix();
    float *xformedpts = new float[3 * input->GetNumberOfPoints()];
    double pt[3], p2[4];
    for (j=0; j < input->GetNumberOfPoints(); j++)
    {
        input->GetPoint(j, pt);

        double p1[4] = {0,0,0,1}; // set homogenous to 1.0
        input->GetPoint(j, p1);

        worldToView->MultiplyPoint(p1, p2);
        if (p2[3] != 0)
        {
            xformedpts[j*3+0]=p2[0]/p2[3];
            xformedpts[j*3+1]=p2[1]/p2[3];
            xformedpts[j*3+2]=p2[2]/p2[3];
        }
    }
    worldToView->Delete();
    visitTimer->StopTimer(TH_transform, "transform");
    visitTimer->DumpTimings();


    //
    // Figure out how many polgons I'm going to send out
    //
    int TH_countdestinations = visitTimer->StartTimer(); 
    for (i=0; i<ncells; i++)
    {
        input->GetCellPoints(i, npts, cellPts);
        IncrementOutgoingCellCounts(xformedpts, npts, cellPts,
                                    outgoingCellCount);
    }
    visitTimer->StopTimer(TH_countdestinations, "countdestinations");
    visitTimer->DumpTimings();


    //
    // Allocate the space for any outgoing polydata
    //
    int TH_allocate = visitTimer->StartTimer(); 
    for (i=0; i<size; i++)
    {
        if (outgoingCellCount[i] > 0)
        {
            outgoingPolyData[i] = vtkPolyData::New();
            outgoingPolyData[i]->SetPoints(input->GetPoints());
            outgoingPolyData[i]->GetPointData()->ShallowCopy(inPD);
            outgoingPolyData[i]->GetCellData()->CopyAllocate(inCD,outgoingCellCount[i]);
            outgoingPolyData[i]->Allocate(outgoingCellCount[i]*5);
        }
    }
    visitTimer->StopTimer(TH_allocate, "allocate");
    visitTimer->DumpTimings();

    //
    // Do the actual stuffing into the output data sets
    //
    int TH_finddestinations = visitTimer->StartTimer(); 
    vector<int> dests;
    for (i=0; i<ncells; i++)
    {
        input->GetCellPoints(i, npts, cellPts);

        int dest = WhichProcessorsForCell(xformedpts, npts, cellPts, dests);

        // code: dest > 0 means single destination
        //       dest==-1 means multiple destinations
        //       dest==-2 means no destinations
        if (dest >= 0)
        {
            int cnt = outgoingPolyData[dest]->InsertNextCell(
                                                   input->GetCellType(i),
                                                   npts, cellPts);
            outgoingPolyData[dest]->GetCellData()->CopyData(inCD, i, cnt);
        }
        else if (dest == -1)
        {
            for (j=0; j<dests.size(); j++)
            {
                int cnt;
                cnt = outgoingPolyData[dests[j]]->InsertNextCell(
                                                       input->GetCellType(i),
                                                       npts, cellPts);
                outgoingPolyData[dests[j]]->GetCellData()->CopyData(inCD, i, cnt);
            }
            dests.clear();
        }
        // else dest==-2 and thus no one owned it.
    }
    visitTimer->StopTimer(TH_finddestinations, "finddestinations");
    visitTimer->DumpTimings();

    //
    // Done with our xformed points, so delete that memory
    //
    delete[] xformedpts;

    //
    // Convert data to strings so we can send them to other processors
    //
    int TH_stringize = visitTimer->StartTimer(); 
    int totalSend = 0;
    vector<int> sendCount;
    vector<unsigned char*> sendString;
    sendString.resize(size, NULL);
    sendCount.resize(size, 0);
    for (i=0; i<size; i++)
    {
        // Note that we don't want to bother stringizing and all-to-all'ing
        // the polygons that we know are going to stay on this processor,
        // so we skip when i==rank.  Also, an outgoing polydata will still have
        // a minimal size with no cells, so explicitly skip them so we can
        // skip that bit of communication entirely.
        if (rank != i && outgoingCellCount[i] > 0)
        {
            sendString[i] = GetDataString(sendCount[i], outgoingPolyData[i]);
            outgoingPolyData[i]->Delete();
            totalSend += sendCount[i];
        }
    }
    visitTimer->StopTimer(TH_stringize, "stringize");
    visitTimer->DumpTimings();


    //
    // Determine how much data everyone will be sending to everyone else
    //
    vector<int> recvCount(size);
    MPI_Alltoall(&sendCount[0], 1, MPI_INT,
                 &recvCount[0], 1, MPI_INT, MPI_COMM_WORLD);

    int totalRecv = 0;
    for (i = 0 ; i < size ; i++)
    {
        totalRecv += recvCount[i];
    }
    
    //
    // Build the displacements for Alltoallv from the known sizes
    //
    vector<int> sendDisp(size);
    vector<int> recvDisp(size);
    sendDisp[0] = 0;
    recvDisp[0] = 0;
    for (i = 1 ; i < size ; i++)
    {
        sendDisp[i] = sendDisp[i-1] + sendCount[i-1];
        recvDisp[i] = recvDisp[i-1] + recvCount[i-1];
    }

    //
    // Copy data to a single big buffer for the Alltoallv
    //
    unsigned char *big_send_buffer = new unsigned char[totalSend];
    for (i = 0 ; i < size ; i++)
    {
        memcpy(&big_send_buffer[sendDisp[i]], sendString[i], sendCount[i]);
    }

    //
    // Transfer the actual data
    //
    int TH_communicate = visitTimer->StartTimer(); 
    unsigned char *big_recv_buffer = new unsigned char[totalRecv];
    MPI_Alltoallv(big_send_buffer, &sendCount[0], &sendDisp[0], MPI_CHAR,
                  big_recv_buffer, &recvCount[0], &recvDisp[0], MPI_CHAR,
                  MPI_COMM_WORLD);
    visitTimer->StopTimer(TH_communicate, "communicate");
    visitTimer->DumpTimings();


    //
    // Now all the data we need is in big_recv_buffer or
    // outgoingPolyData[rank], so free other memory
    //
    delete[] big_send_buffer;

    for (i=0; i<sendString.size(); i++)
    {
        delete[] sendString[i];
    }
    
    //
    // Now convert the received data to polydata
    //
    int TH_appending = visitTimer->StartTimer(); 
    vtkAppendPolyData *appender = vtkAppendPolyData::New();

    // remember we explicitly didn't convert our own data to a string, so
    // we have to add it to the appender explicitly
    if (outgoingPolyData[rank])
        appender->AddInput(outgoingPolyData[rank]);

    for (i=0; i<size; i++)
    {
        if (recvCount[i] > 0)
        {
            vtkPolyData *pd = GetDataVTK(&big_recv_buffer[recvDisp[i]],
                                         recvCount[i]);
            appender->AddInput(pd);
            pd->Delete();
        }
    }
    delete[] big_recv_buffer;
    
    appender->Update();
    GetOutput()->ShallowCopy(appender->GetOutput());
    visitTimer->StopTimer(TH_appending, "appending");
    visitTimer->DumpTimings();

    //
    // Free some memory!
    //
    appender->RemoveAllInputs();
    appender->Delete();
    if (outgoingPolyData[rank])
        outgoingPolyData[rank]->Delete();

    visitTimer->StopTimer(TH_total, "vtkParallelImageSpaceRedistributor");
    visitTimer->DumpTimings();
#endif
}


// ***************************************************************************
//  Method: vtkParallelImageSpaceRedistributor::GetDataVTK()
//
//  Purpose:
//      Takes a string as input, and converts it to vtkPolyData so we
//      can retreive data from other processors and convert it into a
//      usable form.  This was copied from avtDataRepresentation.C.
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 12 11:24:32 PDT 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 21 18:21:50 PDT 2004
//    Cleaned up a little.
//
// **************************************************************************

vtkPolyData *
vtkParallelImageSpaceRedistributor::GetDataVTK(unsigned char *asChar,
                                          unsigned int asCharLength)
{
    vtkPolyDataReader *reader = vtkPolyDataReader::New();

    vtkCharArray *charArray = vtkCharArray::New();
    int iOwnIt = 1;  // 1 means we own it -- you don't delete it.
    charArray->SetArray((char *) asChar, (int) asCharLength, iOwnIt);
    reader->SetReadFromInputString(1);
    reader->SetInputArray(charArray);
    
    vtkPolyData *asVTK = reader->GetOutput();
    asVTK->Update();
    
    asVTK->Register(NULL);
    asVTK->SetSource(NULL);
    reader->Delete();
    charArray->Delete();

    return asVTK;
}

// ***************************************************************************
//  Method: vtkParallelImageSpaceRedistributor::GetDataString()
//
//  Purpose:
//      Takes vtkPolyData as input, and converts it to a string so we
//      can send it to other processors.  This was copied from
//      avtDataRepresentation.C
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 12 11:23:23 PDT 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 21 18:21:50 PDT 2004
//    Cleaned up a little.
//
//    Jeremy Meredith, Tue Oct 26 22:31:02 PDT 2004
//    Added a relevant points filter -- we may have a *lot* more point data
//    in the outgoing datasets, because we just did a shallow copy on the
//    points originally.
//
// **************************************************************************

unsigned char *
vtkParallelImageSpaceRedistributor::GetDataString(int &length,
                                                  vtkPolyData *asVTK)
{     
    unsigned char *asChar = NULL;
    
    if (asVTK == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    vtkPolyDataRelevantPointsFilter *relpts;
    relpts = vtkPolyDataRelevantPointsFilter::New();
    relpts->SetInput(asVTK);
    relpts->Update();

    vtkDataSetWriter *writer = vtkDataSetWriter::New();
    writer->SetInput(relpts->GetOutput());
    writer->SetWriteToOutputString(1);
    writer->SetFileTypeToBinary();
    writer->Write();
    length = writer->GetOutputStringLength();
    asChar = (unsigned char *) writer->RegisterAndGetOutputString();
    writer->Delete();

    relpts->Delete();

    return asChar;
}


// ****************************************************************************
//  Method: vtkParallelImageSpaceRedistributor::WhichProcessorsForCell()
//
//  Purpose:
//      Given a piece of vtkPolyData, the processor that we should use
//      to draw it will depend on where the data lies in image-space.
//      This function analyzes the data and returns a vector of
//      integers.  Each returned integer will represent a processor
//      that should draw this data.
//
// Note:  Currently assumes a horizontal banding!
//        See below for comments on the return values.
//
// Arguments:
//    pts: an array of floats of points in imagespace
//    npts: number of points total
//    cellPts: pointer to cell point ids
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 12 15:15:48 PDT 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 21 12:00:22 PDT 2004
//    Rewrote for optimization.  It will avoid using the vector if it can.
//    It also uses the horizontal strips assumption for speed.
//
//    Jeremy Meredith, Tue Oct 26 22:36:29 PDT 2004
//    Renamed and added some more comments.
//
// ****************************************************************************

int
vtkParallelImageSpaceRedistributor::WhichProcessorsForCell(float *pts,
                                               vtkIdType npts,
                                               vtkIdType *cellPts,
                                               vector<int> &procs)
{
    // dest has some special values: If it is -2, then no processor
    // contained this cell.  If it is -1, then the client should walk
    // the procs vector to find destinations.  If it is >=0, then
    // there was only one destination processor, and this is it.
    // This is a big optimization because the common case is that
    // a polygon only belongs to one processor, and using the vector
    // for the common case can be costly.
    int dest = -2;

    //
    // current implementation divides the screen into vertical bands
    //

    // create 2d bounding box
    float minx, maxx, miny, maxy, tempx, tempy;
    minx = maxx = pts[3*cellPts[0]];
    miny = maxy = pts[3*cellPts[0]+1];

    int i;
    for (i=1; i < npts; i++)
    {
        tempx = pts[3 * cellPts[i] ];
        tempy = pts[3 * cellPts[i] + 1];
        if (tempx < minx)
            minx = tempx;
        if (tempx > maxx)
            maxx = tempx;
        if (tempy < miny)
            miny = tempy;
        if (tempy > maxy)
            maxy = tempy;
    }

    for (i=0; i < size; i++)
    {
        if (miny <= y2[i] && maxy >= y1[i])
        {
            if (dest == -2)
                dest = i;
            else if (dest == -1)
                procs.push_back(i);
            else
            {
                procs.push_back(dest);
                procs.push_back(i);
                dest = -1;
            }
        }
    }

    return dest;
}

// ****************************************************************************
//  Method: vtkParallelImageSpaceRedistributor::IncrementOutgoingCellCounts()
//
//  Purpose:
//      Almost identical to WhichProcessorsForCell, except that instead of
//      returning a list of outgoing processors, it
//
// Note:  Currently assumes a horizontal banding!
//
// Arguments:
//    pts: an array of floats of points in imagespace
//    npts: number of points total
//    cellPts: pointer to cell point ids
//    outgoingCellCount: array of counts of outgoing cells
//
//  Programmer: Jeremy Meredith
//  Creation:   October 26, 2004
//
//  Modifications:
//
// ****************************************************************************

void
vtkParallelImageSpaceRedistributor::IncrementOutgoingCellCounts(float *pts,
                                               vtkIdType npts,
                                               vtkIdType *cellPts,
                                               vector<int> &outgoingCellCount)
{
    //
    // See WhichProcessorsForCell for more notes
    //

    // create 2d bounding box
    float minx, maxx, miny, maxy, tempx, tempy;
    minx = maxx = pts[3*cellPts[0]];
    miny = maxy = pts[3*cellPts[0]+1];

    int i;
    for (i=1; i < npts; i++)
    {
        tempx = pts[3 * cellPts[i] ];
        tempy = pts[3 * cellPts[i] + 1];
        if (tempx < minx)
            minx = tempx;
        if (tempx > maxx)
            maxx = tempx;
        if (tempy < miny)
            miny = tempy;
        if (tempy > maxy)
            maxy = tempy;
    }

    for (i=0; i < size; i++)
    {
        if (miny <= y2[i] && maxy >= y1[i])
        {
            outgoingCellCount[i]++;
        }
    }
}

// ****************************************************************************
//  Method:  vtkParallelImageSpaceRedistributor::CreateWorldToDisplayMatrix()
//
//  Purpose:
//    Get and compose all matrices required to convert world coordinates
//    to display coordinates
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 26, 2004
//
// ****************************************************************************

vtkMatrix4x4 *
vtkParallelImageSpaceRedistributor::CreateWorldToDisplayMatrix()
{
    // Get world->view matrix
    vtkMatrix4x4 *M1 = vtkMatrix4x4::New();
    M1->DeepCopy(ren->GetActiveCamera()->
                                GetCompositePerspectiveTransformMatrix(1,0,1));

    // Set up view->display matrix
    vtkMatrix4x4 *M2 = vtkMatrix4x4::New();
    {
        double *v = ren->GetViewport();
        double *a = ren->GetAspect();

        M2->Identity();
        M2->Element[0][0] = float(width )*(v[2]-v[0])/(2. * a[0]);
        M2->Element[1][1] = float(height)*(v[3]-v[1])/(2. * a[1]);
        M2->Element[0][3] = float(width )*(v[2]+v[0])/2.;
        M2->Element[1][3] = float(height)*(v[3]+v[1])/2.;
    }

    // Compose world->display matrix
    vtkMatrix4x4 *M3 = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(M2,M1,M3);
    M2->Delete();
    M1->Delete();

    return M3;
}

