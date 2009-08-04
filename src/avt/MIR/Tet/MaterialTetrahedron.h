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

#ifndef MATERIAL_TETRAHEDRON_H
#define MATERIAL_TETRAHEDRON_H

#include <MIROptions.h>
#include <vector>

// ****************************************************************************
//  Class:  MaterialTetrahedron
//
//  Purpose:
//    A triangle created from a cell, with VF's for every material at each node
//    Weights are percentages of each of the cell's original nodes.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 11:55:48 PDT 2001
//    Added an origindex field so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Fri Aug 30 17:04:55 PDT 2002
//    Added a reset() method.
//
// ****************************************************************************
struct MaterialTetrahedron
{
    struct Node
    {
        int                   origindex;
        float                 weight[MAX_NODES_PER_ZONE];
        std::vector<float>    matvf;
      public:
        Node()
        {
            reset();
        }
        void reset()
        {
            origindex=-1;
            for (int i=0; i<MAX_NODES_PER_ZONE; i++)
                weight[i]=0.0;
        }
    };

    int  nmat;
    Node node[4];

  public:
    MaterialTetrahedron();
    MaterialTetrahedron(int);
    MaterialTetrahedron(const MaterialTetrahedron&);
    void reset()
    {
        node[0].reset();
        node[1].reset();
        node[2].reset();
        node[3].reset();
    }
};

#endif
