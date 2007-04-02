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
//                               MeshPrep.h                                  //
// ************************************************************************* //

#ifndef MESH_PREP_H
#define MESH_PREP_H

#include <Mesh.h>


// ****************************************************************************
//  Class: Mesh_Prep
//
//  Purpose:
//      A derived class of Mesh that is meant specifically for the meshtvprep
//      project.
//
//  Data Members:
//      read...  -  A boolean value which keeps track of whether that field
//                  has been read in.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modifications:
//     
//      Hank Childs, Sat Mar  4 10:12:08 PST 2000
//      Changed signature of ReadMesh for meshType.  Added data member
//      readMeshType.
//
// ****************************************************************************

class Mesh_Prep : public Mesh
{
  public:
                    Mesh_Prep();
    virtual        ~Mesh_Prep();

    void            ReadMesh(DBpointmesh *);
    void            ReadMesh(DBquadmesh *);
    void            ReadMesh(DBucdmesh *);

    void            SetName(char *);
    void            SetDomains(int);
    void            SetId(int);
  
    void            Consolidate(void);
    void            WrapUp(void);
    virtual void    Write(DBfile *);

  private:
    bool            readCoordinateDimension;
    bool            readDataType;
    bool            readLabels;
    bool            readMeshType;
    bool            readUnits;

    // Private Methods
    void         ReadMesh(int, char **, char **, int, MESH_TYPE_e);

    // Constants
    static char * const   SILO_LOCATION;
    static char * const   COORDS_NAME;
};


#endif


