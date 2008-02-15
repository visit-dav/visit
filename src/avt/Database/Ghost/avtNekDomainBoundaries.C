/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                       avtNekDomainBoundaries.C                            //
// ************************************************************************* //

#include <avtNekDomainBoundaries.h>
#include <ImproperUseException.h>
#include <vtkDataSet.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <float.h>
#include <string.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

#include <InvalidVariableException.h>
#include <ImproperUseException.h>

#include <TimingsManager.h>

// ****************************************************************************
//  Method:  avtNekDomainBoundaries::avtNekDomainBoundaries
//
//  Purpose:
//    Constructor
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
// ****************************************************************************

avtNekDomainBoundaries::avtNekDomainBoundaries()
{
    int ii;
    aNeighborDomains = NULL;
    bFullDomainInfo = false;
    nDomains = 0;
    for (ii = 0; ii < 3; ii++)
        iBlockSize[ii] = 0;
    for (ii = 0; ii < 8; ii++)
        aCornerOffsets[ii] = 0;
        
    bSaveDomainInfo = false;
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::~avtNekDomainBoundaries
//
//  Purpose:
//    Destructor
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
// ****************************************************************************

avtNekDomainBoundaries::~avtNekDomainBoundaries()
{
    if (aNeighborDomains)
    {
        delete[] aNeighborDomains;
        aNeighborDomains = NULL;
    }
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::SetDomainInfo
//
//  Purpose:
//    Constructor
//
//  Arguments:
//    num_domains:  total number of domains in the dataset
//    dims:         dimensions of each domain
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtNekDomainBoundaries::SetDomainInfo(int num_domains, 
                                      const int dims[3])
{
    iBlockSize[0] = dims[0];
    iBlockSize[1] = dims[1];
    iBlockSize[2] = dims[2];
    
    nDomains = num_domains;
    
    aCornerOffsets[0] = 0; 
    aCornerOffsets[1] = iBlockSize[0]-1;
    aCornerOffsets[2] = iBlockSize[0]*(iBlockSize[1]-1);
    aCornerOffsets[3] = iBlockSize[0]*(iBlockSize[1]-1) + iBlockSize[0]-1;
    aCornerOffsets[4] = iBlockSize[0]*iBlockSize[1]*(iBlockSize[2]-1);
    aCornerOffsets[5] = iBlockSize[0]*iBlockSize[1]*(iBlockSize[2]-1) + iBlockSize[0]-1;
    aCornerOffsets[6] = iBlockSize[0]*iBlockSize[1]*(iBlockSize[2]-1) + iBlockSize[0]*(iBlockSize[1]-1);
    aCornerOffsets[7] = iBlockSize[0]*iBlockSize[1]*(iBlockSize[2]-1) + iBlockSize[0]*(iBlockSize[1]-1) + iBlockSize[0]-1;
}




// ****************************************************************************
//  Method:  avtNekDomainBoundaries::Face::Set
//
//  Purpose:
//    Set the points that make up a face.  Four points are passed in but 
//    only three are stored, because if the first three match, so will the
//    fourth.  So, the max point is found and removed, and the other three
//    are stored in sorted order.
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtNekDomainBoundaries::Face::Set(const float *points)
{
    int iMaxPt = 0;
    int ii, jj;

    //Find the biggest point, which will be excluded from the Face struct    
    for (ii = 1; ii < 4; ii++)
    {
        for (jj = 0; jj < 3; jj++)
        {
            if (points[iMaxPt*3+jj] < points[ii*3+jj])
            {
                iMaxPt = ii;
                break;
            }
            else if (points[iMaxPt*3+jj] > points[ii*3+jj])
            {
                break;
            }
        }
    }
    const float *src = points;
    float *dst = pts;
    for (ii = 0; ii < 4; ii++, src += 3)
    {
        if (ii == iMaxPt)
            continue;

        dst[0] = src[0];    
        dst[1] = src[1];    
        dst[2] = src[2];
        dst += 3;
    }
    Sort();
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::Face::Sort
//
//  Purpose:
//    Sort the points that make up a face
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//    Dave Bremer, Thu Jan 24 14:53:27 PST 2008
//    Only sort 3 points now.  The fourth point was 
//    not necessary for finding a match.
// ****************************************************************************

void
avtNekDomainBoundaries::Face::Sort()
{
    //Do a bubble sort on the 3 points
    int hh, ii, jj, kk;
    for (hh = 0; hh < 2; hh++)           //make 2 passes
    {    
        bool anySwaps = false;
        for (ii = 0; ii < 2-hh; ii++)    //iterate over 2 pairs
        {
            for (jj = 0; jj < 3; jj++)   //compare 3 components of the current pair of points
            {
                if (pts[ii*3+jj] < pts[(ii+1)*3+jj])
                {
                    break;
                }
                else if (pts[ii*3+jj] > pts[(ii+1)*3+jj])
                {
                    float tmp;
                    for (kk = 0; kk < 3; kk++)   //swap points
                    {
                        tmp = pts[ii*3+kk];  
                        pts[ii*3+kk] = pts[(ii+1)*3+kk];  
                        pts[(ii+1)*3+kk] = tmp;
                    }
                    anySwaps = true;
                    break;
                }
            }
        }
        if (!anySwaps)
            break;
    }
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::CompareFaces
//
//  Purpose:
//    Used to order faces, determining if they are greater than, less than,
//    or equal to each other.
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
// ****************************************************************************

int
avtNekDomainBoundaries::CompareFaces(const void *f0, const void *f1)
{
    Face *face0 = (Face *)f0;
    Face *face1 = (Face *)f1;

    for (int ii = 0; ii < 9; ii++)
    {
        if (face0->pts[ii] < face1->pts[ii])
            return -1;
        else if (face0->pts[ii] > face1->pts[ii])
            return 1;
    }
    return 0;
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::CompareFaceProcs
//
//  Purpose:
//    Used to order faces by proc id
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
// ****************************************************************************

int
avtNekDomainBoundaries::CompareFaceProcs(const void *f0, const void *f1)
{
    Face *face0 = (Face *)f0;
    Face *face1 = (Face *)f1;

    return (face0->proc - face1->proc);
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::CreateNeighborList
//
//  Purpose:
//    Matches faces of some or all the domains in the problem to determine which 
//    faces have neighbors.  Fills in aNeighborList as a final result.
//    aNeighborList has 6 ints per domain, one per face, which indicates
//    which domain touches that face, or has -1 if it has no neighbor.
//    
//    If running in parallel, each process will get a different subset of
//    domains to work on.  Faces are extracted and matched on the local
//    data.  Unmatched faces are then distributed across all processes
//    using a spatial subdivision, and each process looks for matches again.
//    All matches found are sent to process 0, which consolidates them into
//    one table, and then broadcasts the table to all processes.
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//    Dave Bremer, Thu Jan 24 14:53:27 PST 2008
//    Rewrote the algorithm to divide up the faces spatially, rather than
//    sending them to proc 0 to perform much of the matching.  This allows
//    much less memory to be used by proc 0, and also made the code run in
//    about 0.4 instead of 1.4 seconds in one big test.
// ****************************************************************************

void
avtNekDomainBoundaries::CreateNeighborList(const vector<int>         &domainNum,
                                           const vector<vtkDataSet*> &meshes)
{
    int nLocalDomains = meshes.size();
    int ii, jj, ff, pp, cc;
    const int  f[6][4] = { {0, 2, 4, 6},
                           {1, 3, 5, 7},
                           {0, 1, 4, 5}, 
                           {2, 3, 6, 7},
                           {0, 1, 2, 3},
                           {4, 5, 6, 7} };
    
    aNeighborDomains = new int[nDomains*6];
    
    double corners[24];
    float  facePts[12];
    Face *faces = new Face[nLocalDomains*6];

    //range of all the first points for each face.
    float min[3] = { FLT_MAX, FLT_MAX, FLT_MAX};
    float max[3] = {-FLT_MAX,-FLT_MAX,-FLT_MAX};
    
    for (ii = 0; ii < nLocalDomains; ii++)
    {
        for (cc = 0; cc < 8; cc++)
            meshes[ii]->GetPoint( aCornerOffsets[cc], corners+3*cc );
        
        for (ff = 0; ff < 6; ff++)
        {
            for (pp = 0; pp < 4; pp++)
            {
                facePts[pp*3  ] = (float)corners[f[ff][pp]*3  ];
                facePts[pp*3+1] = (float)corners[f[ff][pp]*3+1];
                facePts[pp*3+2] = (float)corners[f[ff][pp]*3+2];
            }
            faces[ii*6 + ff].domain = domainNum[ii];
            faces[ii*6 + ff].side = ff;
            faces[ii*6 + ff].Set(facePts);
        
            for (pp = 0; pp < 3; pp++)
            {
                if (faces[ii*6 + ff].pts[pp] < min[pp])
                    min[pp] = faces[ii*6 + ff].pts[pp];
                
                if (faces[ii*6 + ff].pts[pp] > max[pp])
                    max[pp] = faces[ii*6 + ff].pts[pp];
            }
        }
    }

    //Sort the face structs
    qsort(faces, nLocalDomains*6, sizeof(Face), avtNekDomainBoundaries::CompareFaces);
    
#ifndef PARALLEL
    //Scan the faces for matching pairs
    for (ii = 0; ii < nLocalDomains*6; ii++)
    {
        if (ii != nLocalDomains*6-1 && CompareFaces(faces+ii, faces+ii+1) == 0)
        {
            aNeighborDomains[faces[ii].domain*6   + faces[ii].side]   = faces[ii+1].domain;
            aNeighborDomains[faces[ii+1].domain*6 + faces[ii+1].side] = faces[ii].domain;
            ii++;
        }
        else
        {
            aNeighborDomains[faces[ii].domain*6   + faces[ii].side]   = -1;
        }
    }
    bFullDomainInfo = (nDomains == meshes.size());

#else
    int nProcs, iRank, err;
    MPI_Comm_rank(VISIT_MPI_COMM, &iRank);
    MPI_Comm_size(VISIT_MPI_COMM, &nProcs);


    //Scan the faces for matching pairs
    vector<int> aMatchedFaces;
    aMatchedFaces.reserve(16384);
    int nUnmatchedFaces = ExtractMatchingFaces(faces, nLocalDomains*6, aMatchedFaces, true);
    
    float globalMin[3], globalMax[3];
    MPI_Allreduce(min, globalMin, 3, MPI_FLOAT, MPI_MIN, VISIT_MPI_COMM);
    MPI_Allreduce(max, globalMax, 3, MPI_FLOAT, MPI_MAX, VISIT_MPI_COMM);

    //Find some constants used for dividing up space.
    float blockSize[3] = {globalMax[0]-globalMin[0],
                          globalMax[1]-globalMin[1],
                          globalMax[1]-globalMin[2]};
    int nBlocks[3] = {1,1,1};
    int nProcsUsed = 1;
    while (nProcsUsed*2 <= nProcs)
    {
        if (blockSize[0] > blockSize[1] && blockSize[0] > blockSize[2])
        {
            nBlocks[0]*=2;
            blockSize[0] /= 2.0;
        }
        else if (blockSize[1] > blockSize[2] && blockSize[1] > blockSize[0])
        {
            nBlocks[1]*=2;
            blockSize[1] /= 2.0;
        }
        else
        {
            nBlocks[2]*=2;
            blockSize[2] /= 2.0;
        }
            
        nProcsUsed *= 2;
    }
    
    int *nFacesToSend = new int[nProcs];
    int *nFacesToRecv = new int[nProcs];
    
    int *aFaceSendOffsets = new int[nProcs];
    int *aFaceRecvOffsets = new int[nProcs];

    for (ii = 0; ii < nProcs; ii++)
        nFacesToSend[ii] = 0;
        
    //Assign each unmatched face to a proc.  This algorithm only sends data to
    //processes with a rank < (the largest power of 2 <= nProcs).
    for (ii = 0; ii < nUnmatchedFaces; ii++)
    {
        int tmp[3];
        for (jj = 0; jj < 3; jj++)
        {
            tmp[jj] = (int)floor((faces[ii].pts[jj] - globalMin[jj]) / blockSize[jj]);
            if (tmp[jj] < 0)
                tmp[jj] = 0;
            else if (tmp[jj] >= nBlocks[jj])
                tmp[jj] = nBlocks[jj]-1;
        }
        faces[ii].proc = tmp[2]*nBlocks[0]*nBlocks[1] + tmp[1]*nBlocks[0] + tmp[0];
        nFacesToSend[faces[ii].proc]++;
    }
    
    //Sort by proc
    qsort(faces, nUnmatchedFaces, sizeof(Face), avtNekDomainBoundaries::CompareFaceProcs);

    //All procs exchange info on the number of faces sent to and received from
    //all other procs.    
    err = MPI_Alltoall(nFacesToSend, 1, MPI_INT,
                       nFacesToRecv, 1, MPI_INT,
                       VISIT_MPI_COMM);
    if (err != MPI_SUCCESS)
        EXCEPTION1(ImproperUseException,
            "Error in MPI_Alltoall, in "
            "avtNekDomainBoundaries::CreateNeighborListScalably");

    int nFacesToMatch = 0;
    for (ii = 0; ii < nProcs; ii++)
    {
        nFacesToMatch += nFacesToRecv[ii];
        
        nFacesToSend[ii] *= sizeof(Face);
        nFacesToRecv[ii] *= sizeof(Face);
    }
    aFaceSendOffsets[0] = 0;
    aFaceRecvOffsets[0] = 0;
    for (ii = 1; ii < nProcs; ii++)
    {
        aFaceSendOffsets[ii] = aFaceSendOffsets[ii-1] + nFacesToSend[ii-1];
        aFaceRecvOffsets[ii] = aFaceRecvOffsets[ii-1] + nFacesToRecv[ii-1];
    }

    Face *moreFaces = new Face[nFacesToMatch];
    
    //All procs exchange faces, which are now divided up spatially
    err = MPI_Alltoallv(faces,     nFacesToSend, aFaceSendOffsets, MPI_BYTE,
                        moreFaces, nFacesToRecv, aFaceRecvOffsets, MPI_BYTE,
                        VISIT_MPI_COMM);
    if (err != MPI_SUCCESS)
        EXCEPTION1(ImproperUseException,
            "Error in MPI_Alltoallv, in "
            "avtNekDomainBoundaries::CreateNeighborListScalably");

    //Sort the new faces received and extract matches.
    qsort(moreFaces, nFacesToMatch, sizeof(Face), avtNekDomainBoundaries::CompareFaces);
    ExtractMatchingFaces(moreFaces, nFacesToMatch, aMatchedFaces, false);

    delete[] moreFaces;
    delete[] nFacesToSend;
    delete[] nFacesToRecv;
    delete[] aFaceSendOffsets;
    delete[] aFaceRecvOffsets;

    //Send all the matches to proc 0
    int *aNumMatches = NULL;
    int  nMatches = aMatchedFaces.size() / 4;
    if (iRank == 0)
        aNumMatches = new int[nProcs];
    
    err = MPI_Gather(&nMatches,   1, MPI_INT,
                     aNumMatches, 1, MPI_INT,
                     0, VISIT_MPI_COMM);
    if (err != MPI_SUCCESS)
        EXCEPTION1(ImproperUseException,
            "Error in MPI_Gather, in avtNekDomainBoundaries::CreateNeighborListScalably");

    MPI_Status status;
    if (iRank == 0)
    {
        for (ii = 0; ii < nDomains*6; ii++)
            aNeighborDomains[ii] = -1;
        
        //Process one set of matches at a time, to keep the max memory usage down.
        //Find the max number of matches, and resize aMatchedFaces to use as a 
        //destination buffer, once the matches for rank 0 have been pulled out.
        int iMaxMatchedFaces = 0;
        for (ii = 0; ii < nProcs; ii++)
        {
            if (iMaxMatchedFaces < aNumMatches[ii])
                iMaxMatchedFaces = aNumMatches[ii];
        }
        aMatchedFaces.resize(iMaxMatchedFaces*4);
        
        for (ii = 0; ii < nProcs; ii++)
        {
            if (ii >= 1)
            {
                err = MPI_Recv( &(aMatchedFaces[0]), aNumMatches[ii]*4, 
                                MPI_INT,  ii, 888, VISIT_MPI_COMM, &status);
                if (err != MPI_SUCCESS)
                    EXCEPTION1(ImproperUseException,
                        "Error in MPI_Recv, in avtNekDomainBoundaries::CreateNeighborListScalably");
            }
            //for each match...
            for (jj = 0; jj < aNumMatches[ii]; jj++)
            {
                int dom0  = aMatchedFaces[jj*4];
                int face0 = aMatchedFaces[jj*4+1];
                int dom1  = aMatchedFaces[jj*4+2];
                int face1 = aMatchedFaces[jj*4+3];
            
                aNeighborDomains[dom0*6+face0] = dom1;
                aNeighborDomains[dom1*6+face1] = dom0;
            }
        }

        delete[] aNumMatches;
    }
    else
    {
        err = MPI_Send( &(aMatchedFaces[0]), aMatchedFaces.size(), 
                        MPI_INT,  0, 888, VISIT_MPI_COMM);
        if (err != MPI_SUCCESS)
            EXCEPTION1(ImproperUseException, 
                "Error in MPI_Send, in avtNekDomainBoundaries::CreateNeighborListScalably");
    }

    //aNeighborDomains is now fully populated on rank 0, 
    //so broadcast to all procs
    err = MPI_Bcast( aNeighborDomains, nDomains*6, MPI_INT,
                     0, VISIT_MPI_COMM );
    if (err != MPI_SUCCESS)
        EXCEPTION1(ImproperUseException, 
            "Error in MPI_Bcast, in avtNekDomainBoundaries::CreateNeighborListScalably");

    //One last thing...determine if aNeighborDomains contains adjacency info
    //for all the domains in the mesh, or if some were excluded
    int iSumOfNumLocalDomains;
    MPI_Allreduce(&nLocalDomains, &iSumOfNumLocalDomains, 1, MPI_INT, MPI_SUM, VISIT_MPI_COMM);

    bFullDomainInfo = (iSumOfNumLocalDomains == nDomains);
#endif
    delete[] faces;
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::ExtractMatchingFaces
//
//  Purpose:
//    Iterate through a sorted list of faces, find matches, and add the matches
//    to an array.  Optionally compress the faces list to put all the 
//    unmatched faces adjacent to one another, and return the number 
//    of unmatched faces.
//
//  Programmer:  Dave Bremer
//  Creation:    Thu Jan 24 14:53:27 PST 2008
//
//  Modifications:
//
// ****************************************************************************

int
avtNekDomainBoundaries::ExtractMatchingFaces(Face *faces, int nFaces, 
                                             vector<int> &aMatchedFaces, 
                                             bool bCompressFaces)
{
    int iCurrUnmatchedFace = 0;
    int ii;
    for (ii = 0; ii < nFaces; ii++)
    {
        if (ii != (nFaces-1) && CompareFaces(faces+ii, faces+ii+1) == 0)
        {
            //Try to avoid lots of reallocs by reserving lots of space.
            if (aMatchedFaces.size() == aMatchedFaces.capacity())
                aMatchedFaces.reserve(aMatchedFaces.size()*2);
            
            aMatchedFaces.push_back(faces[ii].domain);
            aMatchedFaces.push_back(faces[ii].side);
            aMatchedFaces.push_back(faces[ii+1].domain);
            aMatchedFaces.push_back(faces[ii+1].side);
            ii++;
        }
        else
        {
            if (bCompressFaces && ii != iCurrUnmatchedFace)
                memcpy(faces+iCurrUnmatchedFace, faces+ii, sizeof(Face));
            iCurrUnmatchedFace++;
        }
    }
    return iCurrUnmatchedFace;
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::CreateGhostNodes
//
//  Purpose:
//    Marks the ghost nodes for all the domains listed in domainNum.
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//    Dave Bremer, Thu Jan 24 14:53:27 PST 2008
//    Only optionally cache aNeighborDomains now.
// ****************************************************************************

void                      
avtNekDomainBoundaries::CreateGhostNodes(vector<int>          domainNum,
                                         vector<vtkDataSet*>  meshes,
                                         vector<int>         &allDomains)
{
    if (!aNeighborDomains)
    {
        CreateNeighborList(domainNum, meshes);
    }

    int ii, jj, kk;
    bool  bAllDomainsSequential = true;
    bool  bAllDomainsSorted = true;

    //allDomains comes in corrupted if meshes.size() == 0, which is why this
    //loop is wrapped in an if test.
    if (meshes.size() > 0)
    {
        for (ii = 0; ii < allDomains.size()-1 && (bAllDomainsSequential || bAllDomainsSorted); ii++)
        {
            if (allDomains[ii] != allDomains[ii+1]-1)
                bAllDomainsSequential = false;
    
            if (allDomains[ii] > allDomains[ii+1])
                bAllDomainsSorted = false;
        }
    }
    
    for (ii = 0; ii < meshes.size(); ii++)
    {
        int dom = domainNum[ii];
    
        vtkDataSet *ds = meshes[ii];
        int nPts = ds->GetNumberOfPoints();

        vtkUnsignedCharArray *gn = vtkUnsignedCharArray::New();
        gn->SetNumberOfTuples(nPts);
        gn->SetName("avtGhostNodes");
        unsigned char *gnp = gn->GetPointer(0);
    
        for (jj = 0; jj < nPts; jj++)
            gnp[jj] = 0;

        for (jj = 0; jj < 6; jj++)
        {
            //Look in allDomains for the domain that neighbors the current face
            //If found, mark the points on this face.
            int iNeighborDomain = aNeighborDomains[dom*6 + jj];
            bool bMarkFace = false;
            if (bAllDomainsSequential)
            {
                if (allDomains[0] <= iNeighborDomain && iNeighborDomain < allDomains[0]+allDomains.size())
                    bMarkFace = true;
            }
            else if (bAllDomainsSorted)
            {
                int min = 0, max = allDomains.size()-1, mid;
                while (min <= max)
                {
                    mid = (max+min)/2;
                    if (allDomains[mid] == iNeighborDomain)
                    {
                        bMarkFace = true;
                        break;
                    }
                    else if (allDomains[mid] > iNeighborDomain)
                    {
                        max = mid-1;
                    }
                    else
                    {
                        min = mid+1;
                    }
                }
            }
            else
            {
                for (kk = 0; kk < allDomains.size(); kk++)
                {
                    if (allDomains[kk] == iNeighborDomain)
                    {
                        bMarkFace = true;
                        break;
                    }
                }
            }

            if (bMarkFace)
            {
                int mm, nn;
                if (jj == 0 || jj == 1)
                {
                    int iCurrIndex = jj*(iBlockSize[0]-1);

                    for (mm = 0; mm < iBlockSize[2]; mm++)
                        for (nn = 0; nn < iBlockSize[1]; nn++)
                        {
                            avtGhostData::AddGhostNodeType(gnp[iCurrIndex], DUPLICATED_NODE);
                            iCurrIndex += iBlockSize[0];
                        }
                }
                else if (jj == 2 || jj == 3)
                {
                    int iCurrIndex = (jj-2)*(iBlockSize[1]-1)*iBlockSize[0];

                    for (mm = 0; mm < iBlockSize[1]; mm++)
                    {
                        for (nn = 0; nn < iBlockSize[0]; nn++)
                        {
                            avtGhostData::AddGhostNodeType(gnp[iCurrIndex], DUPLICATED_NODE);
                            iCurrIndex++;
                        }
                        iCurrIndex += (iBlockSize[1]-1)*iBlockSize[0];
                    }
                }
                else if (jj == 4 || jj == 5)
                {
                    int iCurrIndex = (jj-4)*(iBlockSize[2]-1)*iBlockSize[1]*iBlockSize[0];

                    for (mm = 0; mm < iBlockSize[1]; mm++)
                        for (nn = 0; nn < iBlockSize[0]; nn++)
                        {
                            avtGhostData::AddGhostNodeType(gnp[iCurrIndex], DUPLICATED_NODE);
                            iCurrIndex++;
                        }
                }
            }
        }
        ds->GetPointData()->AddArray(gn);
        gn->Delete();
    }

    //Delete the table if it covers a subset of the data, otherwise 
    //save it for future calls.
    if (!bSaveDomainInfo || !bFullDomainInfo)
    {
        delete[] aNeighborDomains;
        aNeighborDomains = NULL;
    }
}


// ****************************************************************************
//  Method:  avtNekDomainBoundaries::Destruct
//
//  Purpose:
//    Call destructor on an object.
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtNekDomainBoundaries::Destruct(void *p)
{
    avtNekDomainBoundaries *db = (avtNekDomainBoundaries *)p;
    delete db;
}


bool                      
avtNekDomainBoundaries::RequiresCommunication(avtGhostDataType)
{
    //TODO  make sure it's this simple
#ifdef PARALLEL
    return true;
#else
    return false;
#endif
}


bool                      
avtNekDomainBoundaries::ConfirmMesh(vector<int>      domainNum,
                                    vector<vtkDataSet*> meshes)
{
    //TODO  make sure it's this simple
    return true;
}


vector<vtkDataSet*>       
avtNekDomainBoundaries::ExchangeMesh(vector<int>       domainNum,
                                     vector<vtkDataSet*>   meshes)
{
    EXCEPTION0(ImproperUseException);
}


vector<vtkDataArray*>     
avtNekDomainBoundaries::ExchangeScalar(vector<int>     domainNum,
                                       bool                  isPointData,
                                       vector<vtkDataArray*> scalars)
{
    EXCEPTION0(ImproperUseException);
}


vector<vtkDataArray*>     
avtNekDomainBoundaries::ExchangeFloatVector(vector<int> domainNum,
                                            bool                   isPointData,
                                            vector<vtkDataArray*>  vectors)
{
    EXCEPTION0(ImproperUseException);
}


vector<vtkDataArray*>     
avtNekDomainBoundaries::ExchangeIntVector(vector<int>  domainNum,
                                          bool                  isPointData,
                                          vector<vtkDataArray*> vectors)
{
    EXCEPTION0(ImproperUseException);
}


vector<avtMaterial*>      
avtNekDomainBoundaries::ExchangeMaterial(vector<int>   domainNum,
                                         vector<avtMaterial*>   mats)
{
    EXCEPTION0(ImproperUseException);
}


vector<avtMixedVariable*> 
avtNekDomainBoundaries::ExchangeMixVar(vector<int>     domainNum,
                                       const vector<avtMaterial*>   mats,
                                       vector<avtMixedVariable*>    mixvars)
{
    EXCEPTION0(ImproperUseException);
}

