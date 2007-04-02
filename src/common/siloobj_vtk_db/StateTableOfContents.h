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
//                          StateTableOfContents.h                           //
// ************************************************************************* //

#ifndef STATE_TABLE_OF_CONTENTS_H
#define STATE_TABLE_OF_CONTENTS_H
#include <siloobj_vtk_exports.h>

#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkScalars.h>
#include <vtkStructuredGrid.h>

#include <Material.h>
#include <Mesh_VTK.h>
#include <TableOfContents.h>


// ****************************************************************************
//  Class: StateTableOfContents
//
//  Purpose:
//      Reads in the meshtvprep object based SILO format for a state.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
//  Modifications:
//  
//    Hank Childs, Sat Mar  4 11:00:47 PST 2000
//    Added a GetCoords routine for rectilinear meshes.
//
//    Hank Childs, Mon Apr  3 15:21:51 PDT 2000
//    Added a GetCoords routine for curvilinear meshes.
//
//    Hank Childs, Wed Apr 12 21:30:02 PDT 2000
//    Added a GetMetaData method.  Removed methods GetVarRange,
//    GetMeshDomainList, and GetVarDomainList.
//
//    Hank Childs, Thu Aug 10 11:12:08 PDT 2000
//    Added GetNDomains for meshes.
//
//    Hank Childs, Fri Sep 15 18:10:47 PDT 2000
//    Get centering type of a variable.
//
// ****************************************************************************

class SILOOBJ_VTK_API StateTableOfContents : public TableOfContents
{
    friend class                avtSiloObjDatabase;

  public:
                                StateTableOfContents();
    virtual                    ~StateTableOfContents();

    const char                 *GetMeshName(const char *);
    MESH_TYPE_e                 GetMeshType(const char *);
    void                        GetCoords(const char *, const int *, int, 
                                          vtkUnstructuredGrid **);
    void                        GetCoords(const char *, const int *, int, 
                                          vtkRectilinearGrid **);
    void                        GetCoords(const char *, const int *, int, 
                                          vtkStructuredGrid **);
    void                        GetVar(const char *, const int *, int, 
                                       vtkScalars **);
    const IntervalTree_VTK     *GetMetaData(const char *);

    int                         GetNDomains(const char *);
    int                         GetCentering(const char *);

    virtual Field_VTK          *GetField(const char *);
    virtual IntervalTree_VTK   *GetIntervalTree(const char *);
    virtual Value_VTK          *GetValue(const char *);

    virtual void                UpdateReferences(void);

  protected:
    Field_VTK                 **fields;
    int                         fieldsN;
    IntervalTree_VTK          **trees;
    int                         treesN;
    Material                   *mats;
    int                         matsN;
    Mesh_VTK                   *meshes;
    int                         meshesN;
    Value_VTK                 **values;
    int                         valuesN;

    // Protected Methods
    virtual void                ReadFile(const char *);
    Field_VTK                  *ResolveField(const char *);
    Mesh_VTK                   *ResolveMesh(const char *);
};


#endif


