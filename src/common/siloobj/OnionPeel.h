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
//                              OnionPeel.h                                  //
// ************************************************************************* //

#ifndef ONION_PEEL_H
#define ONION_PEEL_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>


// ****************************************************************************
//  Class: OnionPeel
//
//  Purpose:
//      Takes in an ucd mesh and calculates the onion peel information.
//
//  Data Members:
//      domain      -  The domain to which the connectivity information 
//                     corresponds.
//      meshName    -  The name of the mesh.
//      list        -  The list of which zones correspond to which nodes.
//                     This is an array of size [nnodes]x[zones for node i],
//                     but is implemented as a one-dimensional array.
//      offset      -  The offsets into list that allow the two-dimensional
//                     array (list) to be implemented as a one-dimensional 
//                     array.  This is an array of size nnodes.
//      nnodes      -  The number of nodes for this mesh in this domain.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2000
//
// ****************************************************************************

class SILOOBJ_API OnionPeel
{
  friend         class BoundaryList;

  public:
                 OnionPeel();
                ~OnionPeel();

    void         AddMesh(DBucdmesh *, int);
    void         Write(DBfile *);

  private:
    int          domain;
    int         *list;
    char        *meshName;
    int          nnodes;
    int         *offset;

    // Class-scoped constants
    static char * const    SILO_BLOCK_NAME;
    static char * const    SILO_IDENTIFIER;
    static char * const    SILO_LIST_NAME;
    static char * const    SILO_OFFSET_NAME;
};


#endif


