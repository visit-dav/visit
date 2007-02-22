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

// ************************************************************************* //
//                             IntervalTree.h                                //
// ************************************************************************* //

#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>


// ****************************************************************************
//  Class: IntervalTree
// 
//  Purpose:
//      Keeps an interval tree.
//
//  Data Members:
//      nNodes         The number of nodes in the tree.
//      nDomains       The number of domains in the problem.  This is also the
//                     number of leaf nodes in the tree.
//      nDims          The dimension of the values of the variable. 
//      nodeExtents    The minimum and maximum bounds for all node that 
//                     are covered by this node.  An array of size nNodes by
//                     nDims*2.  Note the nodeExtents is used to store the 
//                     domain values until the tree is constructed.  The
//                     array is kept as 1-dimensional to prevent excessive 
//                     'new's and to allow for MPI calls.
//      nodeIDs        A list that will convert a node number for the interval
//                     tree back to its original domain number.  An array of 
//                     size nNodes.
//      vectorSize     The size of the vector associated with one domain.  This
//                     should be 2*nDims.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
//  Modifications:
//
//    Hank Childs, Fri Aug 11 09:20:22 PDT 2000
//    Grant access to avtIntervalTree for its conversion constructor.
//
// ****************************************************************************

class SILOOBJ_API IntervalTree
{
    friend class   avtIntervalTree;

  public:
                   IntervalTree();
    virtual       ~IntervalTree();

    void           GetExtents(float *);
    char          *GetName() { return name; };
    void           PrintSelf(ostream &);

    void           Read(DBobject *, DBfile *);
    virtual void   Write(DBfile *);
      
  protected:
    int            nNodes;
    int            nDomains;
    int            nDims;
    float         *nodeExtents;
    int           *nodeIDs;

    char          *name;

  public:
    static char * const    NAME;
    static char * const    SILO_TYPE;
  protected:
    static int const       N_DIMS_LIMIT;
    static int const       SILO_NUM_COMPONENTS;
    static char * const    SILO_OBJ_NAME;

    static char * const    SILO_N_NODES_NAME;
    static char * const    SILO_NODE_EXTENTS_NAME;
    static char * const    SILO_NODE_IDS_NAME;
    static char * const    SILO_N_DIMS_NAME;
};


#endif


