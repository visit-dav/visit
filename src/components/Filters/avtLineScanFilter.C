/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                           avtLineScanFilter.C                             //
// ************************************************************************* //

#include <avtLineScanFilter.h>

#include <stdlib.h>

#ifdef PARALLEL
#include <mpi.h>
#endif 

#include <algorithm>

#include <vtkAppendPolyData.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkUnsignedCharArray.h>

#include <vtkVisItCellLocator.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtIntervalTree.h>
#include <avtParallel.h>

#include <Utility.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>


using     std::string;


static int IntersectLineWithRevolvedSegment(const double *line_pt,
                                            const double *, const double *, 
                                            const double *, double *);


// ****************************************************************************
//  Method: avtLineScanFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
// ****************************************************************************

avtLineScanFilter::avtLineScanFilter()
{
    nLines = 1000;
    lines  = NULL;
    seed   = 0;

    distribType = UNIFORM_RANDOM_DISTRIB;
    pos.x  = 0.0f;
    pos.y  = 0.0f;
    pos.z  = 0.0f;
    theta  = 0.0f;
    phi    = 0.0f;
    radius = 1.0f;
}


// ****************************************************************************
//  Method: avtLineScanFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
// ****************************************************************************

avtLineScanFilter::~avtLineScanFilter()
{
    if (lines != NULL)
        delete [] lines;
}


// ****************************************************************************
//  Method: avtLineScanFilter::SetNumberOfLines
//
//  Purpose:
//      Sets the number of lines to take when doing the line scan.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
// ****************************************************************************

void
avtLineScanFilter::SetNumberOfLines(int nl)
{
    nLines = nl;
}


// ****************************************************************************
//  Method: avtLineScanFilter::SetUniformRandomDistrib
//
//  Purpose:
//    Set the distribution of lines to be random and uniformly distributed in
//    both position and orientation, within a sphere enclosing the data bounds.
//
//  Programmer: David Bremer
//  Creation:   Dec 5, 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtLineScanFilter::SetUniformRandomDistrib()
{
    distribType = avtLineScanFilter::UNIFORM_RANDOM_DISTRIB;
}


// ****************************************************************************
//  Method: avtLineScanFilter::SetCylinderDistrib
//
//  Purpose:
//    Set the distribution of lines to be within a cylinder (3D) or rectangle 
//    (2D) given by the parameters.  Lines all have the same orientation, and 
//    are uniformly distributed in space.  This was done to support Hank Shay's
//    request for the Hohlraum Flux Query.
//
//  Programmer: David Bremer
//  Creation:   Dec 5, 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtLineScanFilter::SetCylinderDistrib(float *pos_, 
                                      float  theta_,
                                      float  phi_, 
                                      float  radius_)
{
    distribType = avtLineScanFilter::CYLINDER_DISTRIB;
    pos.x = pos_[0];
    pos.y = pos_[1];
    pos.z = pos_[2];
    theta = theta_;
    phi = phi_;
    radius = radius_;
}


// ****************************************************************************
//  Method: avtLineScanFilter::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.  This gives us
//      a chance to randomly pick the lines.
//
//  Notes:
//      The construction for the lines comes from Michael Cannon's and Tony
//      Warnock's Line Scan Transform paper.  That paper claims that this
//      construction gnerates lines that randomly and uniformly cover a
//      set of objects.  The uniformity is critical so that no portion of
//      object is over represented.
//
//      u1, u2: random numbers
//      2D: r=u1
//          theta=2pi*u2
//
//      Construction: go r units along angle theta from the origin, choose
//                    tangent line.
//
//      3D: z=1-2u1
//          x=cos(2piu2)*sqrt(1-z2)
//          y=sin(2piu2)*sqrt(1-z2)
//          (that's one endpoint, then do it again for the other)
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
//  Modifications:
//      Dave Bremer, Tue Dec  5 12:27:31 PST 2006
//      Added construction of lines within a cylinder or rectangle.
//
//      Dave Bremer, Wed Dec 20 16:22:06 PST 2006
//      Only use the cylindrical execute mode if we are in two dimensions.
//
// ****************************************************************************

void
avtLineScanFilter::PreExecute(void)
{
    avtStreamer::PreExecute();

    if (lines != NULL)
        delete [] lines;
    lines = new double[6*nLines];

    int spatDim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

    double extents[6];
    avtDataset_p input = GetTypedInput();
    avtDatasetExaminer::GetSpatialExtents(input, extents);
    UnifyMinMax(extents, 6);

    if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType() == AVT_RZ &&
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        spatDim = 3;
        extents[4] = extents[0];
        extents[5] = extents[1];
        double max1 = fabs(extents[2]);
        double max2 = fabs(extents[3]);
        double max = (max1 > max2 ? max1 : max2);
        extents[0] = -max;
        extents[1] = +max;
        extents[2] = -max;
        extents[3] = +max;
    }
    double length = sqrt((extents[1]-extents[0])*(extents[1]-extents[0])+
                         (extents[3]-extents[2])*(extents[3]-extents[2])+
                         (extents[5]-extents[4])*(extents[5]-extents[4]));
    length = (length / 2.) * 1.01;
    double origin[3];
    origin[0] = (extents[0]+extents[1])/2.;
    origin[1] = (extents[2]+extents[3])/2.;
    origin[2] = (extents[4]+extents[5])/2.;

    srand(nLines+seed);
    if (distribType == UNIFORM_RANDOM_DISTRIB)
    {
        for (int i = 0 ; i < nLines ; i++)
        {
            if (spatDim == 3)
            {
                double u1 = (double) rand() / (double) RAND_MAX;
                double u2 = (double) rand() / (double) RAND_MAX;
                double z = (1-2*u1);
                double rt_z = sqrt(1-z*z);
                double x = cos(2*M_PI*u2)*rt_z;
                double y = sin(2*M_PI*u2)*rt_z;
                lines[6*i+0] = x*length - origin[0];
                lines[6*i+2] = y*length - origin[1];
                lines[6*i+4] = z*length - origin[2];
        
                u1 = (double) rand() / (double) RAND_MAX;
                u2 = (double) rand() / (double) RAND_MAX;
                z = (1-2*u1);
                rt_z = sqrt(1-z*z);
                x = cos(2*M_PI*u2)*rt_z;
                y = sin(2*M_PI*u2)*rt_z;
                lines[6*i+1] = x*length + origin[0];
                lines[6*i+3] = y*length + origin[1];
                lines[6*i+5] = z*length + origin[2];
            }
            else
            {
                //
                // Tangent line has eqn: xcos + ysin = R
                // So when is this equal to unit circle:
                // x^2 + y^2 = 1
                // You can do the following derivation:
                // y = (-xcos/sin + R/sin)
                // and then substitute that into the unit circle eqn and ultimately
                // get:
                // x^2 - 2R*x*cos + (R^2-sin^2) = 0
                // which has roots
                // X=-B +/- sqrt(B^2 - 4AC)
                //       / 2A
                double u1 = (double) rand() / (double) RAND_MAX;
                double u2 = (double) rand() / (double) RAND_MAX;
                double r = u1;
                double t = 2*M_PI*u2;
                double cosT = cos(t);
                double sinT = sin(t);
                double x1, x2, y1, y2;
                if (fabs(sinT) > 1e-8)
                {
                    double B = -2*r*cosT;
                    double C = r*r-sinT*sinT;
                    double determinant = sqrt(B*B - 4*C);
                    x1 = (-B + determinant) * 0.5;
                    x2 = (-B - determinant) * 0.5;
                    y1 = -x1*cosT / sinT + r/sinT;
                    y2 = -x2*cosT / sinT + r/sinT;
                }
                else
                {
                    // Easy derivation here:
                    // sinT = 0, so tangent line is vertical.  We know X is
                    // at either +r or -r (depending on theta), which can
                    // be written as cosT*r.  Then we can directly calculate Y
                    // on the sphere since we know X.
                    x1 = cosT*r;
                    x2 = cosT*r;
                    y1 = sqrt(1-x1*x1);
                    y2 = -sqrt(1-x2*x2);
                }
                lines[6*i+0] = x1*length + origin[0];
                lines[6*i+2] = y1*length + origin[1];
                lines[6*i+4] = 0.;
                lines[6*i+1] = x2*length + origin[0];
                lines[6*i+3] = y2*length + origin[1];
                lines[6*i+5] = 0.;
            }
        }
    }
    else if (distribType == CYLINDER_DISTRIB)
    {
        if (spatDim == 3)
        {
            double cosT = cos(theta);
            double sinT = sin(theta);
            double cosP = cos(phi);
            double sinP = sin(phi);
            avtVector dir( sinT*sinP, sinT*cosP, cosT );
            avtVector perp0(0,1,0), perp1(1,0,0);
            if (theta != 0.0f)
            {
                perp0 = avtVector(0,0,1) % dir;
                perp1 = perp0 % dir;

                //I don't think normalization is necessary here, but not 100% sure.
                perp0.normalize();
                perp1.normalize();
            }

            for (int i = 0 ; i < nLines ; i++)
            {
                double u1 = (double) rand() / (double) RAND_MAX;
                double u2 = (double) rand() / (double) RAND_MAX;

                // u1 is uniformly in [0..2pi]
                // u2 is in [0..radius], and sqrt is used to push points toward the outside,
                //   creating a uniform spatial distribution
                u1 *= 2.0 * M_PI;
                u2 = sqrt(u2) * radius;

                avtVector  randPt = pos + perp0*cos(u1)*u2 + perp1*sin(u1)*u2;

                lines[6*i+0] = randPt.x - length*dir.x;
                lines[6*i+1] = randPt.x + length*dir.x;
                lines[6*i+2] = randPt.y - length*dir.y;
                lines[6*i+3] = randPt.y + length*dir.y;
                lines[6*i+4] = randPt.z - length*dir.z;
                lines[6*i+5] = randPt.z + length*dir.z;
            }
        }
        else
        {
            avtVector dir( cos(theta), sin(theta), 0 );
            avtVector perp( -dir.y, dir.x, 0 );

            for (int i = 0 ; i < nLines ; i++)
            {
                double u1 = radius * (2.0 * (double) rand() / (double) RAND_MAX - 1.0);
                avtVector  randPt = pos + perp*u1;

                lines[6*i+0] = randPt.x - length*dir.x;
                lines[6*i+1] = randPt.x + length*dir.x;
                lines[6*i+2] = randPt.y - length*dir.y;
                lines[6*i+3] = randPt.y + length*dir.y;
                lines[6*i+4] = 0;
                lines[6*i+5] = 0;
            }
        }
    }
}


static int
AssignToProc(int val, int nlines)
{
    static int nprocs = PAR_Size();

    int linesPerProc = nlines/nprocs + 1;
    int proc = val / linesPerProc;
    return proc;
}


// ****************************************************************************
//  Method: avtLineScanFilter::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
// ****************************************************************************

void
avtLineScanFilter::PostExecute(void)
{
    avtStreamer::PostExecute();

    avtDataTree_p output_tree = GetDataTree();
    int nLeaves = 0;
    vtkDataSet **leaves = output_tree->GetAllLeaves(nLeaves);

#ifdef PARALLEL
    int  i, j;
    int nprocs = PAR_Size();
    std::vector<int> ncells(nprocs, 0);
    for (i = 0 ; i < nLeaves ; i++)
    {
        vtkIntArray *lineId = (vtkIntArray *) 
                               leaves[i]->GetCellData()->GetArray("avtLineID");
        int numIds = lineId->GetNumberOfTuples();
        for (j = 0 ; j < numIds ; j++)
            ncells[AssignToProc(lineId->GetValue(j), nLines)]++;
    }
    vtkPolyData  **pd_msg = new vtkPolyData*[nprocs];
    vtkPoints    **pts    = new vtkPoints*[nprocs];
    vtkCellArray **lines  = new vtkCellArray*[nprocs];
    for (i = 0 ; i < nprocs ; i++)
    {
        if (ncells[i] > 0)
        {
            pd_msg[i] = vtkPolyData::New();

            pts[i] = vtkPoints::New();
            pts[i]->SetNumberOfPoints(ncells[i]*2);

            pd_msg[i]->GetPointData()->CopyAllocate(leaves[0]->GetPointData(), 
                                                    ncells[i]*2);
            pd_msg[i]->GetCellData()->CopyAllocate(leaves[0]->GetCellData(), 
                                                    ncells[i]);
            lines[i] = vtkCellArray::New();
            lines[i]->Allocate(3*ncells[i]);

            pd_msg[i]->SetLines(lines[i]);
            lines[i]->Delete();
            pd_msg[i]->SetPoints(pts[i]);
            pts[i]->Delete();
        }
        else
            pd_msg[i] = NULL;
    }

    vector<int> curCell(nprocs, 0);
    for (i = 0 ; i < nLeaves ; i++)
    {
        vtkIntArray *lineId = (vtkIntArray *) 
                               leaves[i]->GetCellData()->GetArray("avtLineID");
        int numIds = lineId->GetNumberOfTuples();
        for (j = 0 ; j < numIds ; j++)
        {
            int proc = AssignToProc(lineId->GetValue(j), nLines);
            int cc   = curCell[proc];
            int cp   = 2*curCell[proc];
            double pt[3];
            leaves[i]->GetPoint(2*j, pt);
            pts[proc]->SetPoint(cp, pt);
            leaves[i]->GetPoint(2*j+1, pt);
            pts[proc]->SetPoint(cp+1, pt);
            pd_msg[proc]->GetPointData()->CopyData(leaves[i]->GetPointData(), 2*j, cp);
            pd_msg[proc]->GetPointData()->CopyData(leaves[i]->GetPointData(), 2*j+1, cp+1);
            pd_msg[proc]->GetCellData()->CopyData(leaves[i]->GetCellData(), j, cc);
            vtkIdType ids[2];
            ids[0] = cp;
            ids[1] = cp+1;
            lines[proc]->InsertNextCell(2, ids);

            curCell[proc]++;
        }
    }
    
    char **sendmessages = new char*[nprocs];
    int *sendcount = new int[nprocs];
    vector<int> msg_length(nprocs, 0);
    for (i = 0 ; i < nprocs ; i++)
    {
        if (pd_msg[i] != NULL)
        {
            vtkDataSetWriter *writer = vtkDataSetWriter::New();
            writer->SetInput(pd_msg[i]);
            writer->SetWriteToOutputString(1);
            writer->SetFileTypeToBinary();
            writer->Write();
            sendcount[i] = writer->GetOutputStringLength();
            sendmessages[i] = writer->RegisterAndGetOutputString();
            writer->Delete();
        }
        else
        {
            sendmessages[i] = NULL;
            sendcount[i] = 0;
        }
    }

    int total_send = 0;
    for (i = 0 ; i < nprocs ; i++)
        total_send += sendcount[i];
    char *big_send_buff = new char[total_send];
    char *tmp = big_send_buff;
    for (i = 0 ; i < nprocs ; i++)
    {
        memcpy(tmp, sendmessages[i], sendcount[i]*sizeof(char));
        delete [] sendmessages[i];
        sendmessages[i] = tmp;
        tmp += sendcount[i];
    }

    int *recvcount = new int[nprocs];
    MPI_Alltoall(sendcount, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);
    char **recvmessages = new char*[nprocs];
    char *big_recv_buff = CreateMessageStrings(recvmessages,recvcount, nprocs);

    //
    // Calculate the displacement lists.
    //
    int *senddisp = new int[nprocs];
    int *recvdisp = new int[nprocs];
    senddisp[0] = 0;
    recvdisp[0] = 0;
    for (int i = 1 ; i < nprocs ; i++)
    {
        senddisp[i] = senddisp[i-1] + sendcount[i-1];
        recvdisp[i] = recvdisp[i-1] + recvcount[i-1];
    }

    //
    // Do the actual transfer of the lines.   The messages arrays are
    // actually indexes into one big array.  Since MPI expects that big
    // array, give that (which is at location 0).
    //
    MPI_Alltoallv(sendmessages[0], sendcount, senddisp, MPI_CHAR,
                  recvmessages[0], recvcount, recvdisp, MPI_CHAR,
                  MPI_COMM_WORLD);

    delete [] senddisp;
    delete [] recvdisp;

    vtkAppendPolyData *appender = vtkAppendPolyData::New();
    for (i = 0 ; i < nprocs ; i++)
    {
        if (recvcount[i] > 0)
        {
            int iOwnIt = 1;  // 1 means we own it -- you don't delete it.
            vtkPolyDataReader *reader = vtkPolyDataReader::New();
            vtkCharArray *charArray = vtkCharArray::New();
            charArray->SetArray((char *) recvmessages[i],recvcount[i], iOwnIt);
            reader->SetReadFromInputString(1);
            reader->SetInputArray(charArray);
            appender->AddInput(reader->GetOutput());
            reader->Delete();
            charArray->Delete();
        }
    }
    if (appender->GetTotalNumberOfInputConnections() >= 1)
    {
        vtkPolyData *output = appender->GetOutput();
        output->Update();
        avtDataTree_p newtree = new avtDataTree(output, -1);
        SetOutputDataTree(newtree);
    }
    else
    {
        avtDataTree_p newtree = new avtDataTree();
        SetOutputDataTree(newtree);
    }
    appender->Delete();

    for (i = 0 ; i < nprocs ; i++)
    {
        if (pd_msg[i] != NULL)
            pd_msg[i]->Delete();
    }
    delete [] pd_msg;
    delete [] lines;
    delete [] pts;

    delete [] big_send_buff;
    delete [] sendcount;
    delete [] sendmessages; // The content of recvmessages are pointer into
                            // big_recv_buff.
    delete [] big_recv_buff;
    delete [] recvcount;
    delete [] recvmessages; // The content of recvmessages are pointer into
                            // big_recv_buff.
#else
    vtkAppendPolyData *appender = vtkAppendPolyData::New();
    for (int i = 0 ; i < nLeaves ; i++)
        appender->AddInput((vtkPolyData *) leaves[i]);
    vtkPolyData *output = appender->GetOutput();
    output->Update();
    avtDataTree_p newtree = new avtDataTree(output, -1);
    SetOutputDataTree(newtree);
    appender->Delete();
#endif

    delete [] leaves;
}


// ****************************************************************************
//  Method: avtLineScanFilter::ExecuteData
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
//  Modifications:
//
//    Dave Bremer, Wed Dec 20 16:22:06 PST 2006
//    Only use the cylindrical execute mode if we are in two dimensions.
//
// ****************************************************************************

vtkDataSet *
avtLineScanFilter::ExecuteData(vtkDataSet *ds, int dom, std::string)
{
    if (GetInput()->GetInfo().GetAttributes().GetMeshCoordType() == AVT_RZ &&
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        return CylindricalExecute(ds);
    }
    else
    {
        return CartesianExecute(ds);
    }
}


// ****************************************************************************
//  Method: avtLineScanFilter::CartesianExecute
//
//  Purpose:
//      Finds line intersections in cartesian space.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2006
//
// ****************************************************************************

vtkDataSet *
avtLineScanFilter::CartesianExecute(vtkDataSet *ds)
{
    int  i, j;

    vtkVisItCellLocator *locator = vtkVisItCellLocator::New();
    locator->SetDataSet(ds);
    locator->SetIgnoreGhosts(true);
    locator->BuildLocator();

    vtkPoints *ipts = vtkPoints::New();
    vtkPoints *cpts = vtkPoints::New();
    vtkIdList *cells = vtkIdList::New();

    vector<int>    cells_matched;
    vector<double> dist;
    vector<int>    line_id;

    int extraMsg = 100;
    int amtPerMsg = nLines / extraMsg + 1;
    int totalProg = totalNodes * extraMsg;
    UpdateProgress(extraMsg*currentNode, totalProg);
    int lastMilestone = 0;

    for (i = 0 ; i < nLines ; i++)
    {
        double pt1[3];
        pt1[0] = lines[6*i];
        pt1[1] = lines[6*i+2];
        pt1[2] = lines[6*i+4];
        double pt2[3];
        pt2[0] = lines[6*i+1];
        pt2[1] = lines[6*i+3];
        pt2[2] = lines[6*i+5];
        int success = locator->IntersectWithLine(pt1, pt2, ipts, cpts, cells);
        if (success == 0)
            continue;  // No intersection
        int nCells = cells->GetNumberOfIds();
        for (j = 0 ; j < nCells ; j++)
        {
            vtkIdType id = cells->GetId(j);
            vtkCell *cell = ds->GetCell(id);
            int nInter = 0;
            double inter[100];
            if (cell->GetCellDimension() == 3)
            {
                int nFaces = cell->GetNumberOfFaces();
                for (int k = 0 ; k < nFaces ; k++)
                {
                    vtkCell *face = cell->GetFace(k);
                    double x[3];
                    double pcoords[3];
                    double t;
                    int subId;
                    if (face->IntersectWithLine(pt1, pt2, 1e-10, t, x, pcoords, 
                                                subId))
                        inter[nInter++] = t;
                }
            }
            else if (cell->GetCellDimension() == 2)
            {
                int nEdges = cell->GetNumberOfEdges();
                for (int k = 0 ; k < nEdges ; k++)
                {
                    vtkCell *edge = cell->GetEdge(k);
                    double x[3];
                    double pcoords[3];
                    double t;
                    int subId;
                    if (edge->IntersectWithLine(pt1, pt2, 1e-10, t, x, pcoords, 
                                                subId))
                        inter[nInter++] = t;
                }
            }
            // See if the line segment originates or terminates in the cell.
            if (nInter == 1 || nInter == 0)
            {
                if (vtkVisItUtility::CellContainsPoint(cell, pt1))
                    inter[nInter++] = 0.;
                else if (vtkVisItUtility::CellContainsPoint(cell, pt2))
                    inter[nInter++] = 1.;
            }
            if (nInter == 0 || nInter == 1)
                continue;
            // See if we have any near duplicates.
            if (nInter > 2)
            {
                for (int ii = 0 ; ii < nInter-1 ; ii++)
                {
                    for (int jj = ii+1 ; jj < nInter ; jj++)
                    {
                        if (fabs(inter[ii]-inter[jj]) < 1e-10)
                        {
                            inter[ii] = inter[nInter];
                            nInter--;
                        }
                    }
                }
            }
            if (nInter == 2)
            {
                cells_matched.push_back(id);
                dist.push_back(inter[0]);
                dist.push_back(inter[1]);
                line_id.push_back(i);
            }
            else
            {
                // So this is technically an error state.  We have
                // intersected the shape an odd number of times, which
                // should mean that we are inside the shape.  We constructed
                // our lines so that is not possible.  In reality, this occurs
                // because of floating point precision issues.  In addition,
                // every time it occurs, it is because we have a *very*
                // small cell.  The queries that use this filter need to
                // call "CleanPolyData" on it anyway, so cells this small
                // will be "cleaned out".  So, rather than throwing an 
                // exception, we can just continue.
                continue;
            }

            int currentMilestone = (int)(((float) i) / amtPerMsg);
            if (currentMilestone > lastMilestone)
            {
                UpdateProgress(extraMsg*currentNode+currentMilestone, 
                               extraMsg*totalNodes);
                lastMilestone = currentMilestone;
            }
        }
  
        cells->Initialize();
        ipts->Initialize();
        cpts->Initialize();
    }

    cpts->Delete();
    ipts->Delete();
    cells->Delete();
    locator->Delete();

    //
    // Now construct a vtkPolyData object that has the segments.
    //
    vtkPolyData *output = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(dist.size());
    for (i = 0 ; i < dist.size() ; i++)
    {
        float pt[3];
        int line = line_id[i/2];
        pt[0] = (1-dist[i])*lines[6*line] + dist[i]*lines[6*line+1];
        pt[1] = (1-dist[i])*lines[6*line+2] + dist[i]*lines[6*line+3];
        pt[2] = (1-dist[i])*lines[6*line+4] + dist[i]*lines[6*line+5];
        pts->SetPoint(i, pt);
    }
    output->SetPoints(pts);
    pts->Delete();

    vtkPointData *outPD = output->GetPointData();
    vtkPointData *inPD  = ds->GetPointData();
    if (inPD->GetNumberOfArrays() > 0)
    {
        output->GetPointData()->CopyAllocate(ds->GetPointData(), dist.size());
        vtkIdList *ids = vtkIdList::New();
        double weights[100];
        double closestPoint[3];
        int    subId;
        double pcoords[3];
        double dist2;
        for (int i = 0 ; i < dist.size() ; i++)
        {
            int cellId = cells_matched[i/2];
            ds->GetCellPoints(cellId, ids);
            vtkCell *cell = ds->GetCell(cellId);
            double pt[3];
            pts->GetPoint(i, pt);
            cell->EvaluatePosition(pt, closestPoint, subId, pcoords, dist2,
                                   weights);
            outPD->InterpolatePoint(inPD, i, ids, weights);
        }
        ids->Delete();
    }

    vtkCellData *outCD = output->GetCellData();
    vtkCellData *inCD  = ds->GetCellData();
    outCD->CopyAllocate(inCD, cells_matched.size());
    vtkCellArray *line_cells = vtkCellArray::New();
    line_cells->Allocate(3*cells_matched.size());
    vtkIntArray *vtk_line_id = vtkIntArray::New();
    vtk_line_id->SetNumberOfTuples(cells_matched.size());
    vtk_line_id->SetName("avtLineID");
    for (int i = 0 ; i < cells_matched.size() ; i++)
    {
        vtkIdType ids[2];
        ids[0] = 2*i;
        ids[1] = 2*i+1;
        line_cells->InsertNextCell(2, ids);
        vtk_line_id->SetValue(i, line_id[i]);
        outCD->CopyData(inCD, cells_matched[i], i);
    }
    outCD->AddArray(vtk_line_id);
    vtk_line_id->Delete();
    output->SetLines(line_cells);
    line_cells->Delete();

    ManageMemory(output);
    output->Delete();

    UpdateProgress(extraMsg*(currentNode+1), extraMsg*totalNodes);
    return output;
}


// ****************************************************************************
//  Method: avtLineScanFilter::CylindricalExecute
//
//  Purpose:
//      Finds line intersections in cylindrical space.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2006
//
// ****************************************************************************

vtkDataSet *
avtLineScanFilter::CylindricalExecute(vtkDataSet *ds)
{
    int  i, j;

    //
    // Set up an interval tree over the cells, which well help us locate
    // the cells when we cast the lines over the axially-symmetric mesh.
    //
    int nCells = ds->GetNumberOfCells();
    int dims   = 2;
    avtIntervalTree tree(nCells, dims);
    double bounds[6];
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    bool hasGhost = (ghosts != NULL);
    for (i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = ds->GetCell(i);
        cell->GetBounds(bounds);
        tree.AddElement(i, bounds);
    }
    tree.Calculate(true);

    vector<int>    cells_matched;
    vector<double> dist;
    vector<int>    line_id;

    int extraMsg = 100;
    int amtPerMsg = nLines / extraMsg + 1;
    int totalProg = totalNodes * extraMsg;
    UpdateProgress(extraMsg*currentNode, totalProg);
    int lastMilestone = 0;

    vector<int> list;
    for (i = 0 ; i < nLines ; i++)
    {
        double pt1[3];
        pt1[0] = lines[6*i];
        pt1[1] = lines[6*i+2];
        pt1[2] = lines[6*i+4];
        double pt2[3];
        pt2[0] = lines[6*i+1];
        pt2[1] = lines[6*i+3];
        pt2[2] = lines[6*i+5];
        double dir[3];
        dir[0] = pt2[0]-pt1[0];
        dir[1] = pt2[1]-pt1[1];
        dir[2] = pt2[2]-pt1[2];
        tree.GetElementsFromAxiallySymmetricLineIntersection(pt1, dir, list);
        int nCells = list.size();
        if (nCells == 0)
            continue;  // No intersection

        for (j = 0 ; j < nCells ; j++)
        {
            int id = list[j];
            if (hasGhost && ghosts->GetTuple1(id) != 0.)
                continue;
            vtkCell *cell = ds->GetCell(id);
            vector<double> inter;
            int nEdges = cell->GetNumberOfEdges();
            for (int k = 0 ; k < nEdges ; k++)
            {
                vtkCell *edge = cell->GetEdge(k);
                int id1 = edge->GetPointId(0);
                double ePt1[3];
                ds->GetPoint(id1, ePt1);
                int id2 = edge->GetPointId(1);
                double ePt2[3];
                ds->GetPoint(id2, ePt2);

                double curInter[100];  // shouldn't really be more than 4.
                int numInter =
                    IntersectLineWithRevolvedSegment(pt1, dir, ePt1, ePt2,
                                                     curInter);
                for (int l = 0 ; l < numInter ; l++)
                    inter.push_back(curInter[l]);
            }

            if (inter.size() % 2 == 0)
            {
                if (inter.size() > 0)
                {
                    std::sort(inter.begin(), inter.end());
                    for (int l = 0 ; l < inter.size() / 2 ; l++)
                    {
                        cells_matched.push_back(id);
                        dist.push_back(inter[2*l]);
                        dist.push_back(inter[2*l+1]);
                        line_id.push_back(i);
                    }
                }
            }
            else
            {
                // So this is technically an error state.  We have
                // intersected the shape an odd number of times, which
                // should mean that we are inside the shape.  We constructed
                // our lines so that is not possible.  In reality, this occurs
                // because of floating point precision issues.  In addition,
                // every time it occurs, it is because we have a *very*
                // small cell.  The queries that use this filter need to
                // call "CleanPolyData" on it anyway, so cells this small
                // will be "cleaned out".  So, rather than throwing an 
                // exception, we can just continue.
                continue;
            }

            int currentMilestone = (int)(((float) i) / amtPerMsg);
            if (currentMilestone > lastMilestone)
            {
                UpdateProgress(extraMsg*currentNode+currentMilestone, 
                               extraMsg*totalNodes);
                lastMilestone = currentMilestone;
            }
        }
    }

    //
    // Now construct a vtkPolyData object that has the segments.
    //
    vtkPolyData *output = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(dist.size());
    for (i = 0 ; i < dist.size() ; i++)
    {
        float pt[3];
        int line = line_id[i/2];
        pt[0] = (1-dist[i])*lines[6*line] + dist[i]*lines[6*line+1];
        pt[1] = (1-dist[i])*lines[6*line+2] + dist[i]*lines[6*line+3];
        pt[2] = (1-dist[i])*lines[6*line+4] + dist[i]*lines[6*line+5];
        pts->SetPoint(i, pt);
    }
    output->SetPoints(pts);
    pts->Delete();

    vtkPointData *outPD = output->GetPointData();
    vtkPointData *inPD  = ds->GetPointData();
    if (inPD->GetNumberOfArrays() > 0)
    {
        output->GetPointData()->CopyAllocate(ds->GetPointData(), dist.size());
        vtkIdList *ids = vtkIdList::New();
        double weights[100];
        double closestPoint[3];
        int    subId;
        double pcoords[3];
        double dist2;
        for (int i = 0 ; i < dist.size() ; i++)
        {
            int cellId = cells_matched[i/2];
            ds->GetCellPoints(cellId, ids);
            vtkCell *cell = ds->GetCell(cellId);
            double pt[3];
            pts->GetPoint(i, pt);
            // Before evaluating, project the point to cylindrical space.
            double cyl_pt[3];
            cyl_pt[0] = pt[2];
            cyl_pt[1] = sqrt(pt[0]*pt[0] + pt[1]*pt[1]);
            cyl_pt[2] = 0.;
            cell->EvaluatePosition(cyl_pt, closestPoint, subId, pcoords, dist2,
                                   weights);
            outPD->InterpolatePoint(inPD, i, ids, weights);
        }
        ids->Delete();
    }

    vtkCellData *outCD = output->GetCellData();
    vtkCellData *inCD  = ds->GetCellData();
    outCD->CopyAllocate(inCD, cells_matched.size());
    vtkCellArray *line_cells = vtkCellArray::New();
    line_cells->Allocate(3*cells_matched.size());
    vtkIntArray *vtk_line_id = vtkIntArray::New();
    vtk_line_id->SetNumberOfTuples(cells_matched.size());
    vtk_line_id->SetName("avtLineID");
    for (int i = 0 ; i < cells_matched.size() ; i++)
    {
        vtkIdType ids[2];
        ids[0] = 2*i;
        ids[1] = 2*i+1;
        line_cells->InsertNextCell(2, ids);
        vtk_line_id->SetValue(i, line_id[i]);
        outCD->CopyData(inCD, cells_matched[i], i);
    }
    outCD->AddArray(vtk_line_id);
    vtk_line_id->Delete();
    output->SetLines(line_cells);
    line_cells->Delete();

    ManageMemory(output);
    output->Delete();

    UpdateProgress(extraMsg*(currentNode+1), extraMsg*totalNodes);
    return output;
}


// ****************************************************************************
//  Method: avtIntersectionTests::IntersectLineWithRevolvedSegment
//
//  Purpose:
//      Takes a segment that is in cylindrical coordinates and revolves it
//      into three-dimensional Cartesian space and finds the intersections
//      with a line.  The number of intersections can be 1, 2, or 4.
//
//  Arguments:
//      line_pt    A point on the line (Cartesian)
//      line_dir   The direction of the line (Cartesian)
//      seg_1      One endpoint of the segment (Cylindrical)
//      seg_2      The other endpoint of the segment (Cylindrical)
//      inter      The intersections found.  Output value.  They are 
//                 represented distances along line_dir from line_pt.
//
//  Returns:       The number of intersections
//
//  Programmer:    Hank Childs
//  Creation:      July 28, 2006
//
// ****************************************************************************

int
IntersectLineWithRevolvedSegment(const double *line_pt,
                                 const double *line_dir, const double *seg_p1, 
                                 const double *seg_p2, double *inter)
{
    if (seg_p1[1] < 0. || seg_p2[1] < 0.)
    {
        // Doesn't make sense to cross the axis line.
        avtCallback::IssueWarning("VisIt is unable to execute this query, "
                     "because it has an encountered an RZ mesh with "
                     "negative R values.");

        EXCEPTION0(ImproperUseException);
    }

    //
    // Note that in the logic below, we are using the Z-component
    // of the line to compare with the X-component of the cell,
    // since the cell's X-component is actually 'Z' in RZ-space.
    //
    if (seg_p1[0] == seg_p2[0])
    {
        // Vertical line .. revolves to hollow disc.
        // Disc is at some constant Z (seg_p1[0]) and ranges between some
        // Rmin and Rmax.
        double Rmin = (seg_p1[1] < seg_p2[1] ? seg_p1[1] : seg_p2[1]);
        double Rmax = (seg_p1[1] > seg_p2[1] ? seg_p1[1] : seg_p2[1]);
        if (line_dir[2] == 0.)
        {
            if (seg_p1[0] != line_pt[2])
                return 0;
            
            // Solving for inequalities is tough.  In this case, we will
            // solve for equalities.  Solve for R = Rmax and R = Rmin.
            // At^2 + Bt + C = 0
            //  A = Dx^2 + Dy^2
            //  B = 2*Dx*Px + 2*Dy*Py
            //  C = Px^2 + Py^2 - R^2
            double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1];
            double B = 2*line_dir[0]*line_pt[0] + 2*line_dir[1]*line_pt[1];
            double C0 = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1];
            double C = C0 - Rmax*Rmax;
            double det = B*B - 4*A*C;
            int nInter = 0;
            if (det > 0)
            {
                double soln1 = (-B + sqrt(det)) / (2*A);
                double soln2 = (-B - sqrt(det)) / (2*A);
                inter[nInter++] = soln1;
                inter[nInter++] = soln2;
            }
            C = C0 - Rmin*Rmin;
            det = B*B - 4*A*C;
            if (det > 0)
            {
                double soln1 = (-B + sqrt(det)) / (2*A);
                double soln2 = (-B - sqrt(det)) / (2*A);
                inter[nInter++] = soln1;
                inter[nInter++] = soln2;
            }
        }
        else
        {
            // Our line will go through the constant Z-plane that the segment
            // gets revolved into just once.  So calculate that plane and
            // determine if the line intersects the disc.
            double t = (seg_p1[0] - line_pt[2]) / line_dir[2];
            double x = line_pt[0] + t*line_dir[0];
            double y = line_pt[1] + t*line_dir[1];
            double Rsq = x*x + y*y;
            if (Rmin*Rmin < Rsq && Rsq < Rmax*Rmax)
            {
                inter[0] = t;
                return 1;
            }
        }
    }
    else if (seg_p1[1] == seg_p2[1])
    {
        // Horizonal line ... revolves to cylindrical shell.
        // Shell has constant radius (seg_p1[1]) and ranges between Z=seg_p1[0]
        // and Z=seg_p2[0].
        // Solve for t that has intersection.
        // ((Px + t*Dx)^2 + (Py + t*Dy)^2)^0.5 = R
        // ((Px + t*Dx)^2 + (Py + t*Dy)^2) = R^2
        // At^2 + Bt + C = 0
        // A = (Dx^2 + Dy^2)
        // B = (2*Dx*Px + 2*Dy*Py)
        // C = (Px^2 + Py^2 - R^2)
        double R = seg_p1[1];
        double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1];
        double B = 2*line_pt[0]*line_dir[0] + 2*line_pt[1]*line_dir[1];
        double C = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1] - R*R;
        double det = B*B - 4*A*C;
        if (det < 0)
            return 0;
        double soln1 = (-B + sqrt(det)) / (2*A);
        double soln2 = (-B - sqrt(det)) / (2*A);
        double Z1 = line_pt[2] + soln1*line_dir[2];
        double Z2 = line_pt[2] + soln2*line_dir[2];

        int nInter = 0;
        double Zmin = (seg_p1[0] < seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        double Zmax = (seg_p1[0] > seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        if (Zmin <= Z1 && Z1 <= Zmax)
        {
            inter[nInter] = soln1;
            nInter++;
        }
        if (Zmin <= Z2 && Z2 <= Zmax)
        {
            inter[nInter] = soln2;
            nInter++;
        }
        return nInter;
    }
    else
    {
        // We have a segment that is neither horizontal or vertical.  So the
        // revolution of this segment will result in a conic shell, truncated
        // in Z.  The conic shell will be oriented around the Z-axis.
        // The segment is along a line of form r = mz+b.  If we can calculate
        // m and b, then we can ask when a point on the line will coincide
        // with our line.  If they coincided, they would have the same r and
        // z values.  Since the line is in 3D, having the same r values
        // means having the same sqrt(x^2+y^2) values.
        // 
        // Then, for the line:
        // x = Px + t*Dx
        // y = Py + t*Dy
        // z = Pz + t*Dz
        // For the conic shell, we know:
        // sqrt(x^2+y^2) = m*z+b
        // Substituting for Z gives:
        // sqrt(x^2+y^2) = m*(Pz+tDz)+b
        // sqrt(x^2+y^2) = (m*Pz+b) + t*m*Dz
        // Introducing K for m*Pz+b (to simplify algebra)
        // sqrt(x^2+y^2) = K + t*m*Dz
        // Substituting for x and y and squaring gives:
        // (Px^2 + Py^2) + (2*Px*Dx + 2*Py*Dy)t + (Dx^2 + Dy^2)t^2 
        //    = K^2 + (2*K*m*Dz)*t + m^2*Dz^2*t^2
        // Combining like terms gives:
        //  At^2 + Bt + C = 0
        //  A = Dx^2 + Dy^2 - m^2*Dz^2
        //  B = 2*Px*Dx + 2*Py*Dy - 2*K*m*Dz
        //  C = Px^2 + Py^2 - K^2
        // And then we can solve for t to find the intersections.
        // At the end, we will restrict the answer to be between the
        // valid range for the segment.
        double m = (seg_p1[1] - seg_p2[1]) / (seg_p1[0] - seg_p2[0]);
        double b = seg_p1[1] - m*seg_p1[0];
        double K = m*line_pt[2]+b;
        double A = line_dir[0]*line_dir[0] + line_dir[1]*line_dir[1]
                 - m*m*line_dir[2]*line_dir[2];
        double B = 2*line_pt[0]*line_dir[0] + 2*line_pt[1]*line_dir[1]
                 - 2*K*m*line_dir[2];
        double C = line_pt[0]*line_pt[0] + line_pt[1]*line_pt[1] - K*K;
        double det = B*B - 4*A*C;
        if (det < 0)
            return 0;
        double soln1 = (-B + sqrt(det)) / (2*A);
        double soln2 = (-B - sqrt(det)) / (2*A);
        double Z1 = line_pt[2] + soln1*line_dir[2];
        double Z2 = line_pt[2] + soln2*line_dir[2];
        int nInter = 0;
        double Zmin = (seg_p1[0] < seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        double Zmax = (seg_p1[0] > seg_p2[0] ? seg_p1[0] : seg_p2[0]);
        if (Zmin <= Z1 && Z1 <= Zmax)
        {
            inter[nInter] = soln1;
            nInter++;
        }
        if (Zmin <= Z2 && Z2 <= Zmax)
        {
            inter[nInter] = soln2;
            nInter++;
        }
        return nInter;
    }

    return 0;
}


// ****************************************************************************
//  Method:  avtLineScanFilter::RefashionDataObjectInfo
//
//  Purpose:
//    Set up the atttributes and validity for the output of the filter.
//
//  Programmer:  Hank Childs
//  Creation:    July 28, 2006
//
// ****************************************************************************
void
avtLineScanFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();

    if (inAtts.GetSpatialDimension() == 2 &&
        inAtts.GetMeshCoordType() == AVT_RZ)
        outAtts.SetSpatialDimension(3);

    outAtts.SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}


