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
#include <vtkPolyDataReader.h>

#include <ImproperUseException.h>
#include <NoInputException.h>
#include <TimingsManager.h>

#include <avtParallel.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

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


using std::string;
using std::vector;

vtkStandardNewMacro(vtkParallelImageSpaceRedistributor);


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
    rank = PAR_Rank();
    size = PAR_Size();
    x1 = new int[size];
    x2 = new int[size];
    y1 = new int[size];
    y2 = new int[size];
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
// ****************************************************************************
void
vtkParallelImageSpaceRedistributor::Execute(void)
{
#ifdef PARALLEL
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
    vector<vtkPolyData *>  givers;
    vector<unsigned char*> giverstrings;
    vector<unsigned char*> keeperstrings;
    vector<unsigned int>   giverlengths;
    vector<unsigned int>   keeperlengths;

    //
    // Create some automatic variables that we will use when generating the
    // outputs.
    //
    vtkPointData *inPD       = input->GetPointData();
    vtkCellData  *inCD       = input->GetCellData();
    vtkIdType     npts       = 0;
    vtkIdType    *cellPts    = 0;
    input->BuildCells();

    // initialize outgoing datasets
    giverstrings.resize(size);
    giverlengths.resize(size);
    givers.resize(size);
    keeperstrings.resize(size);
    keeperlengths.resize(size);
    for (i=0; i<size; i++)
    {
        givers[i] = vtkPolyData::New();
        givers[i]->SetPoints(input->GetPoints());
        givers[i]->GetPointData()->ShallowCopy(inPD);
        givers[i]->GetCellData()->CopyAllocate(inCD,input->GetNumberOfCells());
        givers[i]->Allocate(input->GetNumberOfCells()*5);
    }

    // divide up data
    float *xformedpts;
    float *pt;

    int TH_transform = visitTimer->StartTimer(); 
    xformedpts = new float[3 * input->GetNumberOfPoints()];

    // Get world->view matrix
    vtkMatrix4x4 *M1 = vtkMatrix4x4::New();
    M1->DeepCopy(ren->GetActiveCamera()->
                                GetCompositePerspectiveTransformMatrix(1,0,1));

    // Set up view->display matrix
    vtkMatrix4x4 *M2 = vtkMatrix4x4::New();
    {
        float *v = ren->GetViewport();
        float *a = ren->GetAspect();

        M2->Identity();
        M2->Element[0][0] = float(width )*(v[2]-v[0])/(2. * a[0]);
        M2->Element[1][1] = float(height)*(v[3]-v[1])/(2. * a[1]);
        M2->Element[0][3] = float(width )*(v[2]+v[0])/2.;
        M2->Element[1][3] = float(height)*(v[3]+v[1])/2.;
    }

    // Compose world->display matrix
    vtkMatrix4x4 *M3 = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(M2,M1,M3);

    for (j=0; j < input->GetNumberOfPoints(); j++)
    {
        pt = input->GetPoint(j);

        float p1[4] = {0,0,0,1}; // set homogenous to 1.0
        input->GetPoint(j, p1);
        float p2[4];

        M3->MultiplyPoint(p1, p2);
        if (p2[3] != 0)
        {
            xformedpts[j*3+0]=p2[0]/p2[3];
            xformedpts[j*3+1]=p2[1]/p2[3];
            xformedpts[j*3+2]=p2[2]/p2[3];
        }
    }
    visitTimer->StopTimer(TH_transform,
                          "vtkParallelImageSpaceRedistributor -- transform");

    int TH_finddestinations = visitTimer->StartTimer(); 
    vector<int> dests;
    for (i=0; i<ncells; i++)
    {
        input->GetCellPoints(i, npts, cellPts);

        int dest = WhichProcessors(xformedpts, npts, cellPts, dests);

        if (dest >= 0)
        {
            int cnt = givers[dest]->InsertNextCell(input->GetCellType(i),
                                               npts, cellPts);
            givers[dest]->GetCellData()->CopyData(inCD, i, cnt);
        }
        else if (dest == -1)
        {
            for (j=0; j<dests.size(); j++)
            {
                int cnt;
                cnt = givers[dests[j]]->InsertNextCell(input->GetCellType(i),
                                                       npts, cellPts);
                givers[dests[j]]->GetCellData()->CopyData(inCD, i, cnt);
            }
            dests.clear();
        }
        // else dest==-2 and thus no one owned it.
    }
    visitTimer->StopTimer(TH_finddestinations,
                     "vtkParallelImageSpaceRedistributor -- finddestinations");

    for (i=0; i<size; i++)
    {
        givers[i]->Squeeze();
    }
    delete [] xformedpts;

    // convert data to strings so we can send them to other processors
    int TH_stringize = visitTimer->StartTimer(); 
    for (i=0; i<size; i++)
    {
        if (rank != i)
            giverstrings[i] = GetDataString(giverlengths[i], givers[i]);
    }
    visitTimer->StopTimer(TH_stringize,
                          "vtkParallelImageSpaceRedistributor -- stringize");

    int TH_communicate = visitTimer->StartTimer(); 
    //
    // Send all data to the appropriate processors
    // and Receive data from other processors
    //
    for (i=0; i<size; i++)
    {
        for (j=0; j<i; j++)
        {
            if (i!=j)
            {
                if (rank == i)
                {
                    // send size to j, then send data to j
                    MPI_Send(&giverlengths[j], 1, MPI_INT, j, 0,
                             MPI_COMM_WORLD);
                    MPI_Send(giverstrings[j], giverlengths[j],
                             MPI_UNSIGNED_CHAR, j, 0, MPI_COMM_WORLD);
                    
                    // receive size from j, allocate new memory,
                    // and receive data from j
                    keeperlengths[j] = 0;
                    MPI_Recv(&keeperlengths[j], 1, MPI_INT, j, 0,
                             MPI_COMM_WORLD, &stat);
                    keeperstrings[j] = new unsigned char[ keeperlengths[j] ];
                    MPI_Recv(keeperstrings[j], keeperlengths[j],
                             MPI_UNSIGNED_CHAR, j, 0, MPI_COMM_WORLD, &stat);
                }
                else if (rank == j)
                {
                    // receive size from i, allocate new memory,
                    // and receive data from i
                    keeperlengths[i] = 0;
                    MPI_Recv(&keeperlengths[i], 1, MPI_INT, i, 0,
                             MPI_COMM_WORLD, &stat);
                    keeperstrings[i] = new unsigned char[ keeperlengths[i] ];
                    MPI_Recv(keeperstrings[i], keeperlengths[i],
                             MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, &stat);

                    // send size to i, then send data to i
                    MPI_Send(&giverlengths[i], 1, MPI_INT, i, 0,
                             MPI_COMM_WORLD);
                    MPI_Send(giverstrings[i], giverlengths[i],
                             MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
                    
                }
            }
        }
    }
    visitTimer->StopTimer(TH_communicate,
                          "vtkParallelImageSpaceRedistributor -- communicate");

    // now all the data we need is in givers[rank] and keeperstrings.
    givers.clear();
    giverstrings.clear();
    giverlengths.clear();
    for (i=0; i<givers.size(); i++)
    {
        delete [] giverstrings[i];
        if (i != rank)
            givers[i]->Delete();
    }
    
                   
    //
    // Now create the output.
    //
    int TH_appending = visitTimer->StartTimer(); 

    vtkAppendPolyData *appender = vtkAppendPolyData::New();
    appender->AddInput(givers[rank]);
    for (i=0; i<size; i++)
    {
        if (i!=rank)
        {
            vtkPolyData *pd = GetDataVTK(keeperstrings[i], keeperlengths[i]);
            appender->AddInput(pd);
        }
    }
    
    appender->Update();
    GetOutput()->ShallowCopy(appender->GetOutput());
    visitTimer->StopTimer(TH_appending,
                          "vtkParallelImageSpaceRedistributor -- appending");

    //
    // Free some memory!
    //
    appender->RemoveAllInputs();
    appender->Delete();
    for (i=0; i<keeperstrings.size(); i++)
        delete [] keeperstrings[i];
    givers[rank]->Delete();

    visitTimer->StopTimer(TH_total,
                          "vtkParallelImageSpaceRedistributor -- total");
    visitTimer->DumpTimings();
#endif
}


//***************************************************************************
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
//**************************************************************************

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

//***************************************************************************
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
//**************************************************************************

unsigned char *
vtkParallelImageSpaceRedistributor::GetDataString(unsigned int &length,
                                             vtkPolyData *asVTK)
{     
    unsigned char *asChar = NULL;
    
    if (asVTK == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    vtkDataSetWriter *writer = vtkDataSetWriter::New();
    writer->SetInput(asVTK);
    writer->SetWriteToOutputString(1);
    writer->SetFileTypeToBinary();
    writer->Write();
    length = (unsigned int) writer->GetOutputStringLength();
    asChar = (unsigned char *) writer->RegisterAndGetOutputString();
    writer->Delete();

    return asChar;
}


//***************************************************************************
//  Method: vtkParallelImageSpaceRedistributor::WhichProcessors()
//
//  Purpose:
//      Given a piece of vtkPolyData, the processor that we should use
//      to draw it will depend on where the data lies in image-space.
//      This function analyzes the data and returns a vector of
//      integers.  Each returned integer will represent a processor
//      that should draw this data.
//
// Note:  Currently assumes a horizontal banding!
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
//**************************************************************************

int
vtkParallelImageSpaceRedistributor::WhichProcessors(float *pts,
                                               vtkIdType npts,
                                               vtkIdType *cellPts,
                                               vector<int> &procs)
{
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

