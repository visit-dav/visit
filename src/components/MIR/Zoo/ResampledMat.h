/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#ifndef RESAMPLED_MAT_H
#define RESAMPLED_MAT_H

#include <avtMaterial.h>
#include <MIRConnectivity.h>

#include "BitUtils.h"

// ****************************************************************************
//  Class:  ResampledMat
//
//  Purpose:
//    This class encapsulates the resampling of material volume fractions
//    from their native zonal values to the nodes.
//
//    The materials are stored natively, sparsely at the cells, with volume
//    fractions of 0.0 where materials do not exist.  We want them averaged
//    to the nodes and still stored as sparsely as possible.  This is an
//    attempt to do just that.  There are probably some assumptions that
//    more than 256 materials, so there is an explicit check for that.
//
//    The matsAtCellOrig array is a bit array containing a "1" for every
//    material which was present in the original Material.
//
//    The matsAtNode array is a bit array containing a "1" for every material
//    which is non-zero after averaging to the nodes.
//
//    The matsAtCellOneAway array is a bit array containing a "1" for every
//    material which is adjacent to a node which has a non-zero volume
//    fraction for that material.
//
//    The matArray arrays contain the volume fraction and material number
//    for every material present at every node, where the materials and
//    volume fractions are stored in increasing material number order, packed
//    so that only non-zero volume fraction materials are stored.
//
//    The nodeIndexInMatArray array stores the first index into the matArray
//    arrays where a node's material VFs are stored.
//
//    The nCellsAdjacentToNode array simply counts the number of cells
//    adjacent to any given node so that it is straightforward to renormalize
//    the VFs that were accumulated to the nodes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 21, 2003
//
// ****************************************************************************
struct ResampledMat
{
  public:
    ResampledMat(int nCells, int nPoints, avtMaterial*, MIRConnectivity*);
    ~ResampledMat();

    void Resample();
    void PrintArrays();

    void PrintMatsAtCellOrig(int c);
    void PrintMatsAtCellOneAway(int c);
    void PrintMatsAtNode(int n);

    inline int GetNodeMatArrayIndex(int node, int matno)
    {
        int byteNum = int(matno / 8);
        int offset = bitPos[matsAtNode[nBPE*node+byteNum]][matno%8];
        if (offset < 0)
            return -1;

        int index = nodeIndexInMatArray[node];
        for (int b = 0; b < byteNum; b++)
            index += nBitsSet[matsAtNode[nBPE*node+b]];
        index += offset;
        return index;
    }

  public:
    // number of bytes per element  (i.e. nmats / 8)
    int               nBPE;

    // array of length nBPE * nCells
    unsigned char    *matsAtCellOrig;
    unsigned char    *matsAtCellOneAway;

    // array of length nBPE * nPoints
    unsigned char    *matsAtNode;

    // array of length nPoints (+1 for index array)
    unsigned char    *nCellsAdjacentToNode;
    int              *nodeIndexInMatArray;

    // arrays of length nPoints * avg(nmats_per_point)
    int               matArrayLen;
    float            *matArrayNodeVF;
    unsigned char    *matArrayMatNo;

  protected:
    void CountMatsAtAdjacentNodes();
    void CountMatArrayLength();
    void AccumulateVFsToMatArray();
    void RenormalizeMatArray();
    void CountMatsAtAdjacentCells();

  protected:
    int               nCells;
    int               nPoints;
    avtMaterial      *mat;
    MIRConnectivity  *conn;
    const int        *matlist;
    const int        *mixMat;
    const int        *mixNext;
    const float      *mixVF;
};

#endif
