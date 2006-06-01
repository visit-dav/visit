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

#include "ResampledMat.h"

#include <ImproperUseException.h>
#include <TimingsManager.h>

// ****************************************************************************
//  Constructor:  ResampledMat::ResampledMat
//
//  Arguments:
//    nc_        the number of cells
//    np_        the number of points
//    m_         the material
//    c_         the MIR connectivity object
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
ResampledMat::ResampledMat(int nc_, int np_,
                           avtMaterial *m_, MIRConnectivity *c_)
{
    nCells  = nc_;
    nPoints = np_;
    mat     = m_;
    conn    = c_;
    matlist = mat->GetMatlist();
    mixMat  = mat->GetMixMat();
    mixNext = mat->GetMixNext();
    mixVF   = mat->GetMixVF();

    nBPE = int((mat->GetNMaterials()+7)/8);
    if (nBPE > 32)
    {
        EXCEPTION1(ImproperUseException,
                   "More than 256 materials per domain is not supported");
    }

    matsAtNode            = NULL;
    matsAtCellOrig        = NULL;
    matsAtCellOneAway     = NULL;
    nCellsAdjacentToNode  = NULL;
    nodeIndexInMatArray   = NULL;

    matArrayLen           = 0;
    matArrayNodeVF        = NULL;
    matArrayMatNo         = NULL;
}

// ****************************************************************************
//  Destructor:  ResampledMat::~ResampledMat
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
ResampledMat::~ResampledMat()
{
    delete[] matsAtNode;
    delete[] nodeIndexInMatArray;
    delete[] matArrayNodeVF;
    delete[] matArrayMatNo;
    delete[] nCellsAdjacentToNode;
    delete[] matsAtCellOrig;
    delete[] matsAtCellOneAway;
}

// ****************************************************************************
//  Method:  ResampledMat::Resample
//
//  Purpose:
//    Perform the individual steps of the resampling process.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::Resample()
{
    int th1, th2;
    th1 = visitTimer->StartTimer();

    th2 = visitTimer->StartTimer();
    CountMatsAtAdjacentNodes();
    visitTimer->StopTimer(th2, "MIR: Subsample: OR to nodes");

    th2 = visitTimer->StartTimer();
    CountMatArrayLength();
    visitTimer->StopTimer(th2, "MIR: Subsample: Count array len");

    th2 = visitTimer->StartTimer();
    AccumulateVFsToMatArray();
    visitTimer->StopTimer(th2, "MIR: Subsample: Accumulate to arrays");

    th2 = visitTimer->StartTimer();
    RenormalizeMatArray();
    visitTimer->StopTimer(th2, "MIR: Subsample: Renormalize node VFs to [0,1]");

    th2 = visitTimer->StartTimer();
    CountMatsAtAdjacentCells();
    visitTimer->StopTimer(th2, "MIR: Subsample: OR to cells");

    visitTimer->StopTimer(th1, "MIR: Cumulative Subsample");
    visitTimer->DumpTimings();
}

// ****************************************************************************
//  Method:  ResampledMat::CountMatsAtAdjacentNodes
//
//  Purpose:
//    Traverse the matlist and perform an "OR" to the nodes so we find out
//    which materials a node is adjacent to (i.e. has a nonzero volume
//    fraction for after averaging cell values to the nodes).
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::CountMatsAtAdjacentNodes()
{
    matsAtCellOrig = new unsigned char[nBPE * nCells];
    memset(matsAtCellOrig, 0, nBPE*nCells);
    matsAtNode = new unsigned char[nBPE * nPoints];
    memset(matsAtNode, 0, nBPE*nPoints);

    //
    // Go through each cell, extract the materials from the avtMaterial,
    // and subsample the zone volume fractions to the nodes and the faces.
    //
    const int *c_ptr = conn->connectivity;
    for (int c=0; c<nCells; c++)
    {
        int        nPts = *c_ptr;
        const int *ids  = c_ptr+1;

        //
        // Create a list of materials and their corresponding volume
        // fractions for the cell for the materials actually used.
        //
        int matno = matlist[c];

        if (matno < 0)
        {
            int  mixIndex  = -matno - 1;

            while (mixIndex >= 0)
            {
                matno = mixMat[mixIndex];
                matsAtCellOrig[nBPE*c + byteForBit(matno)] |= bitForBit(matno);
                mixIndex = mixNext[mixIndex] - 1;
            }
            for (int n=0; n<nPts; n++)
            {
                for (int b=0; b<nBPE; b++)
                {
                    matsAtNode[nBPE*ids[n] + b] |= matsAtCellOrig[nBPE*c + b];
                }
            }
        }
        else
        {
            matsAtCellOrig[nBPE*c + byteForBit(matno)] |= bitForBit(matno);
            for (int n=0; n<nPts; n++)
            {
                matsAtNode[nBPE*ids[n] + byteForBit(matno)] |=
                                                             bitForBit(matno);
            }
        }

        c_ptr += *c_ptr+1;
    }
}

// ****************************************************************************
//  Method:  ResampledMat::CountMatArrayLength
//
//  Purpose:
//    Find out how many materials are actually present at each node
//    and sum these numbers all up.  Set the starting index for each
//    node while we're traversing this anyway.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::CountMatArrayLength()
{
    //
    // Count the array length
    //
    nodeIndexInMatArray = new int[nPoints+1];
    int    curVFIndex = 0;
    for (int n=0; n<nPoints; n++)
    {
        nodeIndexInMatArray[n] = curVFIndex;
        for (int b=0; b<nBPE; b++)
        {
            curVFIndex += nBitsSet[matsAtNode[nBPE*n + b]];
        }
    }
    matArrayLen = curVFIndex;
    nodeIndexInMatArray[nPoints] = curVFIndex;
}

// ****************************************************************************
//  Method:  ResampledMat::AccumulateVFsToMatArray
//
//  Purpose:
//    Add the VF's in the original cells up to the new nodal values.
//    We will divide by the cell count later so they get renormalized.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::AccumulateVFsToMatArray()
{
    //
    // Accumulate original cell vf's to nodes
    //
    matArrayNodeVF = new float[matArrayLen];
    matArrayMatNo = new unsigned char[matArrayLen];
    nCellsAdjacentToNode = new unsigned char[nPoints];
    memset(matArrayNodeVF, 0, matArrayLen * sizeof(float));
    memset(nCellsAdjacentToNode, 0, nPoints);
    const int *c_ptr = conn->connectivity;
    for (int c=0; c<nCells; c++)
    {
        int        nPts = *c_ptr;
        const int *ids  = c_ptr+1;

        for (int n=0; n<nPts; n++)
        {
            nCellsAdjacentToNode[ids[n]]++;
        }

        //
        // Create a list of materials and their corresponding volume
        // fractions for the cell for the materials actually used.
        //
        int matno = matlist[c];

        if (matno < 0)
        {
            int  mixIndex  = -matno - 1;

            while (mixIndex >= 0)
            {
                matno = mixMat[mixIndex];
                float vf = mixVF[mixIndex];
                for (int n=0; n<nPts; n++)
                {
                    // for each point in the cell, add this vf (1.0) to its
                    // matArrayNodeVF and increment nCellsAdjacentToNode for it
                    int index = GetNodeMatArrayIndex(ids[n], matno);
                    matArrayNodeVF[index] += vf;
                    matArrayMatNo[index] = matno;
                }
                mixIndex = mixNext[mixIndex] - 1;
            }
        }
        else
        {
            for (int n=0; n<nPts; n++)
            {
                // for each point in the cell, add this vf (1.0) to its
                // matArrayNodeVF and increment nCellsAdjacentToNode for it
                int index = GetNodeMatArrayIndex(ids[n], matno);
                matArrayNodeVF[index] += 1.0;
                matArrayMatNo[index] = matno;
            }
        }

        c_ptr += *c_ptr+1;
    }
}

// ****************************************************************************
//  Method:  ResampledMat::RenormalizeMatArray
//
//  Purpose:
//    Divide the accumulated VF's so that they are back in the range of 0 - 1.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::RenormalizeMatArray()
{
    for (int n = 0 ; n < nPoints ; n++)
    {
        int startIndex = nodeIndexInMatArray[n];
        int len = (nodeIndexInMatArray[n+1]-startIndex);
        for (int i=0; i<len; i++)
        {
            int index = startIndex+i;

            matArrayNodeVF[index] /= float(nCellsAdjacentToNode[n]);
        }
    }
}

// ****************************************************************************
//  Method:  ResampledMat::CountMatsAtAdjacentCells
//
//  Purpose:
//    Now that we've "OR"ed to the nodes, "OR" back to the cells
//    so we've reached to the cells that are adjacent to the nodes
//    (which are in turn adjacent to the original cells).
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::CountMatsAtAdjacentCells()
{
    //
    // Figure out which mats may exist in each cell
    //
    matsAtCellOneAway = new unsigned char[nBPE * nCells];
    memset(matsAtCellOneAway, 0, nBPE*nCells);
    const int *c_ptr = conn->connectivity;
    for (int c=0; c<nCells; c++)
    {
        int        nPts = *c_ptr;
        const int *ids  = c_ptr+1;

        unsigned char *cm = &matsAtCellOneAway[nBPE*c];
        for (int n=0; n<nPts; n++)
        {
            unsigned char *nm = &matsAtNode[nBPE*ids[n]];
            for (int b=0; b<nBPE; b++)
            {
                cm[b] |= nm[b];
            }
        }

        c_ptr += *c_ptr+1;
    }
}

// ****************************************************************************
//  Method:  ResampledMat::PrintMatsAtCellOrig
//
//  Purpose:
//    Print the existence of all materials for a single cell in the original
//    material.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::PrintMatsAtCellOrig(int c)
{
    cerr << "cell "<<c<<": ";
    for (int m=0; m<mat->GetNMaterials(); m++)
    {
        int b = byteForBit(m);
        unsigned char t = bitForBit(m);
        if (matsAtCellOrig[nBPE*c + b] & t)
            cerr << m << " ";
        else
            cerr << "  ";
    }
    cerr << endl;
}

// ****************************************************************************
//  Method:  ResampledMat::PrintMatsAtCellOneAway
//
//  Purpose:
//    Print the existence of all materials for a single cell when looking
//    at its adjacency to all neighboring nodes/cells.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::PrintMatsAtCellOneAway(int c)
{
    cerr << "cell "<<c<<": ";
    for (int m=0; m<mat->GetNMaterials(); m++)
    {
        int b = byteForBit(m);
        unsigned char t = bitForBit(m);
        if (matsAtCellOneAway[nBPE*c + b] & t)
            cerr << m << " ";
        else
            cerr << "  ";
    }
    cerr << endl;
}

// ****************************************************************************
//  Method:  ResampledMat::PrintMatsAtNode
//
//  Purpose:
//    Print the existence of all materials for a single node.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
void
ResampledMat::PrintMatsAtNode(int n)
{
    cerr << "pt "<<n<<": ";
    for (int m=0; m<mat->GetNMaterials(); m++)
    {
        int b = byteForBit(m);
        unsigned char t = bitForBit(m);
        if (matsAtNode[nBPE*n + b] & t)
            cerr << m << " ";
        else
            cerr << "  ";
    }
    cerr << endl;
}

// ****************************************************************************
//  Method:  ResampledMat::PrintArrays
//
//  Purpose:
//    Print all the arrays.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
//  Modifications:
//    Brad Whitlock, Tue Sep 23 09:46:55 PDT 2003
//    Changed so it builds on Windows.
//
// ****************************************************************************
void
ResampledMat::PrintArrays()
{
    //
    //  Print the new arrays
    //
    for (int n = 0 ; n < nPoints ; n++)
    {
        cerr << "Node "<<n<<":\n";
        int startIndex = nodeIndexInMatArray[n];
        int len = (nodeIndexInMatArray[n+1]-startIndex);
        cerr << "count="<<int(nCellsAdjacentToNode[n])<<"\n";
        for (int i=0; i<len; i++)
        {
            int index = startIndex+i;
            cerr << "  index="<<index<<"  ";
            cerr << "mat="<<int(matArrayMatNo[index])<<"  ";
            cerr << "vf="<<matArrayNodeVF[index]<<"\n";
        }
    }

    for (int nn=0; nn<nPoints; nn++)
    {
        PrintMatsAtNode(nn);
    }

    for (int c=0; c<nCells; c++)
    {
        PrintMatsAtCellOrig(c);
        PrintMatsAtCellOneAway(c);
    }
}

