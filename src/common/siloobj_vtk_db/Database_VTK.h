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


