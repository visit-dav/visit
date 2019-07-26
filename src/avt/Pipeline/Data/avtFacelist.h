// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtFacelist.h                               //
// ************************************************************************* //

#ifndef AVT_FACELIST_H
#define AVT_FACELIST_H
#include <pipeline_exports.h>


#include <vector>

#include <vtkSystemIncludes.h>  // for vtkIdType

class   vtkPoints;
class   vtkPolyData;
class   vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtFacelist
//
//  Purpose:
//      Contains a facelist for a domain.
//
//  Note:       This class was modelled after the struct DBfacelist from the
//              Silo library.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Aug  9 17:55:51 PDT 2001
//    Made nodelist be vtkIdType* instead of int* to match VTK 4.0 API
//    for insertion of cells.
//
//    Hank Childs, Mon Jul  1 20:13:33 PDT 2002
//    Added a bail-out option for when original zone numbers are not provided.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtFacelist
{
  public:
                           avtFacelist(int *, int, int, int *,int *,int *,int);
    virtual               ~avtFacelist();

    static void            Destruct(void *);

    void                   CalcFacelist(vtkUnstructuredGrid *, vtkPolyData *);
    void                   CalcFacelistFromPoints(vtkPoints *, vtkPolyData *);

    bool                   CanCalculateZonalVariables(void)
                               { return zones != NULL; };

  protected:
    vtkIdType             *nodelist;
    int                    lnodelist;
    int                    nshapes;
    int                    nfaces;
    int                   *shapecnt;
    int                   *shapesize;
    int                   *zones;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtFacelist(const avtFacelist &) {;};
    avtFacelist         &operator=(const avtFacelist &) { return *this; };
};


// ****************************************************************************
//  Class: avtMultiFacelist
//
//  Purpose:
//      Contains facelists for many domains.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
// ****************************************************************************

class PIPELINE_API avtMultiFacelist
{
  public:
                             avtMultiFacelist(int);
    virtual                 ~avtMultiFacelist();

    bool                     Valid();
    void                     SetDomain(avtFacelist *, int);

    void                     GetDomainsList(std::vector<int> &);

    void                     Calculate(void);
    void                     CalcFacelist(vtkUnstructuredGrid *, int,
                                          vtkPolyData *);

  protected:
    avtFacelist            **fl;
    bool                    *validFL;
    int                      numDomains;
};


#endif


