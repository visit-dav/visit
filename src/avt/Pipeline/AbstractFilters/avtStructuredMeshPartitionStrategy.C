/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                   avtStructuredMeshPartitionStrategy.C                    //
// ************************************************************************* //

#include <avtStructuredMeshPartitionStrategy.h>


// ****************************************************************************
//  Method: avtStructuredMeshPartitionStrategy constructor
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2004
//
// ****************************************************************************

avtStructuredMeshPartitionStrategy::avtStructuredMeshPartitionStrategy()
{
    minimumSize = 1024;
}


// ****************************************************************************
//  Method: avtStructuredMeshPartitionStrategy destructor
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2004
//
// ****************************************************************************

avtStructuredMeshPartitionStrategy::~avtStructuredMeshPartitionStrategy()
{
    ;
}


// ****************************************************************************
//  Method: avtStructuredMeshPartitionStrategy::SetMinimumSize
//
//  Purpose:
//      Specifies the minimum allowable size.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2004
//
// ****************************************************************************

void
avtStructuredMeshPartitionStrategy::SetMinimumSize(int ms)
{
    minimumSize = ms;
}


/* The code below is a unit test to test whether or not a particular
 * partition strategy is correctly covering all zones.

% cat Makefile
CC=g++
CFLAGS=-g

OBJ=avtSweepPlanePartitionStrategy.o avtStructuredMeshPartitionStrategy.o t.o

SRC=avtSweepPlanePartitionStrategy.C avtStructuredMeshPartitionStrategy.C t.C

t: $(OBJ)
<tab>   $(CC) $(CFLAGS) -o $@ $(OBJ)

.C.o: $<
<tab>   $(CC) -I. -c $(CFLAGS) $<

% cat t.C
#include <avtSweepPlanePartitionStrategy.h>
#include <avtStructuredMeshChunker.h>

int main()
{
    const int d0 = 30;
    const int d1 = 30;
    const int d2 = 40;
    const int dims[3] = { d0, d1, d2 };
    avtStructuredMeshChunker::ZoneDesignation *d_plus =
        new avtStructuredMeshChunker::ZoneDesignation[dims[0]*dims[1]*dims[2]];

    avtStructuredMeshChunker::ZoneDesignation r =
                                            avtStructuredMeshChunker::RETAIN;
    avtStructuredMeshChunker::ZoneDesignation d =
                                             avtStructuredMeshChunker::DISCARD;

    //
    // Set up the retained/discarded zones.  Two options here.  The first
    // option should have three boxes.  The second will have a ton.  The
    // trick for the second test is to just pass the print statements at the
    // bottom.
    //
    int num = dims[0]*dims[1]*dims[2];
    bool doTest1 = true;
    for (int n = 0 ; n < num ; n++)
    {
        if (doTest1)
        {
            int i = n % dims[0];
            int j = (n / dims[0]) % dims[1];
            int k = n / (dims[0]*dims[1]);
            if (i > d0/2 || j > d1/2 || k > d2/2)
                d_plus[n] = r;
            else
                d_plus[n] = d;
        }
        else
        {
            if ((rand()%2) == 0)
                d_plus[n] = r;
            else
                d_plus[n] = d;
        }
    }

    // 
    // Call the partition strategy.  This is still using sweep plane.
    // Note that the minimum size is set to 1.  This is because this is
    // needed to capture all the boxes from the "random" case.  Also, for
    // the "test 1" case, the boxes will be so big that any minimum is fine.
    //
    avtSweepPlanePartitionStrategy sp;
    sp.SetMinimumSize(1);
    std::vector<int> boxes;
    sp.ConstructPartition(dims, d_plus, boxes);

    //
    // Print out all the boxes that were created.
    //
    cerr << "Boxes = " << boxes.size() / 6 << endl;
    for (int i = 0 ; i < boxes.size() / 6 ; i++)
    {
        cerr << boxes[6*i] << "-" << boxes[6*i+1] << ", " << boxes[6*i+2]
             << "-" << boxes[6*i+3] << ", " << boxes[6*i+4] << "-"
             << boxes[6*i+5] << endl;
    }

    //
    // Now test for what can go wrong -- ids used multiple times or
    // retained zones discarded or discarded zones retained.
    //
    bool *turned_on = new bool[num];
    for (int i = 0 ; i < num ; i++)
        turned_on[i] = false;
    for (int b = 0 ; b < boxes.size()/6 ; b++)
    {
        for (int i = boxes[6*b] ; i <= boxes[6*b+1] ; i++)
            for (int j = boxes[6*b+2] ; j <= boxes[6*b+3] ; j++)
                for (int k = boxes[6*b+4] ; k <= boxes[6*b+5] ; k++)
                {
                    int idx = k*dims[0]*dims[1] + j*dims[0] + i;
                    if (turned_on[idx])
                        cerr<< "Boxes span multiple ids" << endl;
                    turned_on[idx] = true;
                }
    }
    for (int i = 0 ; i < num ; i++)
    {
        bool shouldPrint = false;
        if (turned_on[i] && d_plus[i] != r)
        {
            cerr << "Gave us zone  we shouldn't have" << endl;
            shouldPrint =true;
        }
        if (!turned_on[i] && d_plus[i] != d)
        {
            cerr << "Discarded zone we should have kept" << endl;
            shouldPrint =true;
        }
        if (shouldPrint)
        {
            int I = i % dims[0];
            int J = (i / dims[0]) % dims[1];
            int K = i / (dims[0]*dims[1]);
            cerr << "Zone[" << i << "] = " << I << ", " << J << ", " << K 
                 << endl;
        }
     }
    delete [] d_plus;
    delete [] turned_on;
    boxes.clear();
}
 */
