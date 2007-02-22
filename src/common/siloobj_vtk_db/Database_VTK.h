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
//                              Database_VTK.h                               //
// ************************************************************************* //

#ifndef DATABASE_VTK_H
#define DATABASE_VTK_H
#include <siloobj_vtk_exports.h>

#include <silo.h>

#include <vtkDataSet.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>

#include <MeshTableOfContents.h>
#include <StateTableOfContents.h>


// ****************************************************************************
//  Class: Database_VTK
//
//  Purpose:
//      Holds multiple table of contents objects that store the object based
//      SILO format.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Mar  4 10:12:08 PST 2000
//    Added routine CreateUnstructuredMesh, CreateCurvilinearMesh, and
//    CreateRectilinearMesh.
//
//    Hank Childs, Wed Apr 12 16:48:44 PDT 2000
//    Pushed operations (cutplane/contour) down to the Manager.
//
//    Hank Childs, Thu Aug 10 09:40:08 PDT 2000
//    Add GetNDomains method.
//     
//    Hank Childs, Wed Sep 13 15:17:27 PDT 2000
//    Add SetState routine.
//
// ****************************************************************************

class SILOOBJ_VTK_API Database_VTK
{
    friend class              avtSiloObjDatabase;

  public:
                              Database_VTK();
    virtual                  ~Database_VTK();

    vtkDataSet               *GetDataSet(int *, int, const char *);
    int                       GetNDomains(const char *);
    const IntervalTree_VTK   *GetSpacialMetaData(const char *);
    const IntervalTree_VTK   *GetVariableMetaData(const char *);

    void                      Read(int, const char * const *);
    void                      Read(const char *);
    void                      PrintSelf(void);

    void                      SetState(int);
    int                       GetNStates(void)  { return nStates; };

  protected:
    char                     *stem;
    int                       stemLength;

    int                       nVisitFiles;
    char                    **visitFiles;
    int                       nMeshFiles;
    char                    **meshFiles;
    int                       nStates;
    char                    **states;
    int                      *nStateFiles;
    char                   ***stateFiles;

    int                       currentState;
    StateTableOfContents     *stateTOC;
    MeshTableOfContents       meshTOC;

    // Protected Methods
    void                      ClassifyFiles(int, const char * const *);
    void                      CreateCurvilinearMesh(const char *, const char *,
                                                    const int *, int,
                                                    vtkDataSet **);
    void                      CreateRectilinearMesh(const char *, const char *,
                                                    const int *, int,
                                                    vtkDataSet **);
    void                      CreateUnstructuredMesh(const char *, 
                                                     const char *, const int *,
                                                     int, vtkDataSet **);
    void                      DetermineStem(const char * const *, int);
    void                      OrganizeStateFiles(int, const char * const *);
    void                      ReadVisit(const char *);
};


#endif


