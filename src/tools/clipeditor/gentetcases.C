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

#include <visitstream.h>

// ****************************************************************************
//  File:  gentetcases.cpp
//
//  Purpose:
//    Generates the 48 transforms of a hex.
//    Points are 0-8, edges are A-L.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2003
//
// ****************************************************************************


static int tetVerticesFromEdges[6][2] =
{
    { 0, 1 },   /* Edge 0 */
    { 1, 2 },   /* Edge 1 */
    { 2, 0 },   /* Edge 2 */
    { 0, 3 },   /* Edge 3 */
    { 1, 3 },   /* Edge 4 */
    { 2, 3 }    /* Edge 5 */
};

void
GenCase(int n1, int n2, int n3, int n4, bool flip)
{
    cerr << "{{"
         << n1 <<","
         << n2 <<","
         << n3 <<","
         << n4 << "},{";

    int allnodes[4] = {n1, n2, n3, n4};
    for (int i=0; i<6; i++)
    {
        int e0 = tetVerticesFromEdges[i][0];
        int e1 = tetVerticesFromEdges[i][1];

        int n0 = allnodes[e0];
        int n1 = allnodes[e1];

        for (int j=0; j<6; j++)
        {
            int j0 = tetVerticesFromEdges[j][0];
            int j1 = tetVerticesFromEdges[j][1];

            if (n0==j0 && n1==j1)
            {
                cerr << "'" << char(int('a')+j) << "'";
            }
            else if (n0==j1 && n1==j0)
            {
                cerr << "'" << char(int('a')+j) << "'";
            }

        }
        if (i<5) cerr << ",";
    }
    cerr << "},";

    cerr << (flip ? "true " : "false");
    cerr << " }," << endl;
}

int main()
{
    int counter = 0;
    for (int n1=0; n1<4; n1++)
    {
        for (int n2=0; n2<4; n2++)
        {
            if (n2==n1)
                continue;
            for (int n3=0; n3<4; n3++)
            {
                if (n3==n1 || n3==n2)
                    continue;
                for (int n4=0; n4<4; n4++)
                {
                    if (n4==n1 || n4==n2 || n4==n3)
                        continue;

                    bool flip = (counter%4)==1 || (counter%4)==2;

                    GenCase(n1,n2,n3,n4,flip);
                    counter++;
                }
            }
        }
    }
}
