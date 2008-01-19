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

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

#include <InvalidVariableException.h>
#include <ImproperUseException.h>



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
//  Method:  avtNekDomainBoundaries::Face::Sort
//
//  Purpose:
//    Sort the points that make up a face
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtNekDomainBoundaries::Face::Sort()
{
    //Do a bubble sort on the 4 points
    int hh, ii, jj, kk;
    for (hh = 0; hh < 3; hh++)
    {    
        bool anySwaps = false;
        for (ii = 0; ii < 3; ii++)       //iterate over 3 pairs
        {
            for (jj = 0; jj < 3; jj++)   //compare the current pair of points
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

    for (int ii = 0; ii < 12; ii++)
    {
        if (face0->pts[ii] < face1->pts[ii])
            return -1;
        else if (face0->pts[ii] > face1->pts[ii])
            return 1;
    }
    return 0;
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
//    data, then all the lists of matches and unmatched faces are sent to
//    process 0, which matches the remaining faces, builds aNeighborList,
//    and broadcasts aNeighborList to the rest of the processes.
//
//  Programmer:  Dave Bremer
//  Creation:    Fri Jan 18 16:21:34 PST 2008
//
//  Modifications:
//
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
    
    aNeighborDomains = new int[nDomains*6 + 1];
    
    double corners[24];
    Face *faces      = (Face *)malloc(sizeof(Face)*nLocalDomains*6);
        
    for (ii = 0; ii < nLocalDomains; ii++)
    {
        for (cc = 0; cc < 8; cc++)
            meshes[ii]->GetPoint( aCornerOffsets[cc], corners+3*cc );
        
        for (ff = 0; ff < 6; ff++)
        {
            for (pp = 0; pp < 4; pp++)
            {
                faces[ii*6 + ff].pts[pp*3  ] = (float)corners[f[ff][pp]*3  ];
                faces[ii*6 + ff].pts[pp*3+1] = (float)corners[f[ff][pp]*3+1];
                faces[ii*6 + ff].pts[pp*3+2] = (float)corners[f[ff][pp]*3+2];
            }
            faces[ii*6 + ff].domain = domainNum[ii];
            faces[ii*6 + ff].side = ff;
            faces[ii*6 + ff].Sort();
        }
    }

    //Sort the face structs
    qsort(faces, nLocalDomains*6, sizeof(Face), avtNekDomainBoundaries::CompareFaces);

#ifndef PARALLEL
    //Scan the faces for matching pairs
    for (ii = 0; ii < nLocalDomains*6; ii++)
    {
        if ( ii == nLocalDomains*6-1 )
        {
            aNeighborDomains[faces[ii].domain*6   + faces[ii].side]   = -1;
        }
        else if ( CompareFaces(faces+ii, faces+ii+1) == 0 )
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
    int nProcs, iRank;
    MPI_Comm_rank(VISIT_MPI_COMM, &iRank);
    MPI_Comm_size(VISIT_MPI_COMM, &nProcs);


    //Scan the faces for matching pairs
    int iCurrUnmatchedFace = 0;
    vector<int> aMatchedFaces;
    aMatchedFaces.reserve(16384);
    
    for (ii = 0; ii < nLocalDomains*6; ii++)
    {
        if ( ii == nLocalDomains*6-1 )
        {
            if (ii != iCurrUnmatchedFace)
                memcpy(faces+iCurrUnmatchedFace, faces+ii, sizeof(Face));
            iCurrUnmatchedFace++;
        }
        else if ( CompareFaces(faces+ii, faces+ii+1) == 0 )
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
            if (ii != iCurrUnmatchedFace)
                memcpy(faces+iCurrUnmatchedFace, faces+ii, sizeof(Face));
            iCurrUnmatchedFace++;
        }
    }

    int sendBuf[2], err;
    sendBuf[0] = aMatchedFaces.size()/4;
    sendBuf[1] = iCurrUnmatchedFace;
    
    int *aBufSizes = NULL;
    if (iRank == 0)
        aBufSizes = new int[2*nProcs];
    
    err = MPI_Gather(sendBuf,   2, MPI_INT,
                     aBufSizes, 2, MPI_INT,
                     0, VISIT_MPI_COMM);
    if (err != MPI_SUCCESS)
        EXCEPTION1(ImproperUseException,
            "Error in MPI_Gather, in avtNekDomainBoundaries::CreateNeighborList");

    MPI_Status status;
    if (iRank == 0)
    {
        int nMatchedFaces = 0, nUnmatchedFaces = 0, iMaxMatchedFaces = 0;
        for (ii = 0; ii < nProcs; ii++)
        {
            if (iMaxMatchedFaces < aBufSizes[ii*2])
                iMaxMatchedFaces = aBufSizes[ii*2];

            nMatchedFaces   += aBufSizes[ii*2];
            nUnmatchedFaces += aBufSizes[ii*2+1];
        }
        aMatchedFaces.resize(iMaxMatchedFaces*4);
        faces = (Face *)realloc( faces, sizeof(Face) * nUnmatchedFaces );

        //printf("sizeof unmatched faces: %d.  sizeof aNeighborDomains: %d\n", 
        //    sizeof(Face) * nUnmatchedFaces, 
        //    sizeof(int) * (nDomains * 6 + 1));
        
        Face *currFace = faces;
        
        //Process the matches right away, and collect all the 
        //unmatched faces into the faces array
        for (ii = 0; ii < nProcs; ii++)
        {
            if (ii >= 1)
            {
                err = MPI_Recv( &(aMatchedFaces[0]), aBufSizes[ii*2]*4, 
                                MPI_INT,  ii, 888, VISIT_MPI_COMM, &status);
                if (err != MPI_SUCCESS)
                    EXCEPTION1(ImproperUseException,
                        "Error in MPI_Recv (1), in avtNekDomainBoundaries::CreateNeighborList");
        
                err = MPI_Recv( currFace, aBufSizes[ii*2+1] * sizeof(Face), 
                                MPI_BYTE, ii, 999, VISIT_MPI_COMM, &status);
                if (err != MPI_SUCCESS)
                    EXCEPTION1(ImproperUseException, 
                        "Error in MPI_Recv (2), in avtNekDomainBoundaries::CreateNeighborList");
            }
            //for each match...
            for (jj = 0; jj < aBufSizes[ii*2]; jj++)
            {
                int dom0  = aMatchedFaces[jj*4];
                int face0 = aMatchedFaces[jj*4+1];
                int dom1  = aMatchedFaces[jj*4+2];
                int face1 = aMatchedFaces[jj*4+3];
            
                aNeighborDomains[dom0*6+face0] = dom1;
                aNeighborDomains[dom1*6+face1] = dom0;
            }
            
            currFace += aBufSizes[ii*2+1];
        }
        delete[] aBufSizes;
        
        qsort(faces, nUnmatchedFaces, sizeof(Face), avtNekDomainBoundaries::CompareFaces);

        for (ii = 0; ii < nUnmatchedFaces; ii++)
        {
            if ( ii == nUnmatchedFaces-1 )
            {
                aNeighborDomains[faces[ii].domain*6   + faces[ii].side]   = -1;
            }
            else if ( CompareFaces(faces+ii, faces+ii+1) == 0 )
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
        //Write in a bool at the end of aNeighborDomains, telling 
        //whether aNeighborDomains accounts for all domains.
        aNeighborDomains[nDomains*6] = (nUnmatchedFaces+nMatchedFaces)/6 == nDomains;
    }
    else
    {
        err = MPI_Send( &(aMatchedFaces[0]), aMatchedFaces.size(), 
                        MPI_INT,  0, 888, VISIT_MPI_COMM);
        if (err != MPI_SUCCESS)
            EXCEPTION1(ImproperUseException, 
                "Error in MPI_Send (1), in avtNekDomainBoundaries::CreateNeighborList");

        err = MPI_Send( faces, iCurrUnmatchedFace * sizeof(Face), 
                        MPI_BYTE, 0, 999, VISIT_MPI_COMM);
        if (err != MPI_SUCCESS)
            EXCEPTION1(ImproperUseException, 
                "Error in MPI_Send (2), in avtNekDomainBoundaries::CreateNeighborList");
    
    }
    
#if 1
    
//int *orig = new int[nDomains*6+1];
//memcpy(orig, aNeighborDomains, 4*(nDomains*6+1));

//printf("rank %d:  num domains: %d \n", iRank, nDomains);
/*
    //Done making the structure.  Now send it to all procs.
    err = MPI_Scatter( aNeighborDomains, nDomains*6+1, MPI_INT,
                       aNeighborDomains, nDomains*6+1, MPI_INT,
                       0, VISIT_MPI_COMM );
    if (err != MPI_SUCCESS)
        EXCEPTION1(ImproperUseException, 
            "Error in MPI_Scatter, in avtNekDomainBoundaries::CreateNeighborList");
*/
    err = MPI_Bcast( aNeighborDomains, nDomains*6+1, MPI_INT,
                     0, VISIT_MPI_COMM );
    if (err != MPI_SUCCESS)
        EXCEPTION1(ImproperUseException, 
            "Error in MPI_Bcast, in avtNekDomainBoundaries::CreateNeighborList");
    

            
//int nChanged = 0;
//for (ii=0; ii<nDomains*6+1; ii++)
//{
//    printf("rank: %02d %03d   old: %d\tnew %d\n",  iRank, ii, orig[ii], aNeighborDomains[ii]);
//}
//delete[] orig;
#else
    //Homemade scatter
    if (iRank==0)
    {
        for (ii=1; ii<nProcs; ii++)
        {
            MPI_Send( aNeighborDomains, nDomains*6 + 1, 
                      MPI_INT, ii, 777, VISIT_MPI_COMM);
        }
    }
    else
    {
        MPI_Recv( aNeighborDomains, nDomains*6 + 1,
                  MPI_INT, 0, 777, VISIT_MPI_COMM, &status);
    }
#endif
    
    bFullDomainInfo = aNeighborDomains[nDomains*6];
#endif
    free(faces);
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
//
// ****************************************************************************

void                      
avtNekDomainBoundaries::CreateGhostNodes(vector<int>          domainNum,
                                         vector<vtkDataSet*>  meshes,
                                         vector<int>         &allDomains)
{
    if (!bFullDomainInfo)
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
    if (!bFullDomainInfo)
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

