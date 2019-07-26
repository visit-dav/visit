// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkConnectedTubeFilter_h
#define __vtkConnectedTubeFilter_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

class vtkCellArray;
class vtkPoints;

// ****************************************************************************
//  Class:  vtkConnectedTubeFilter
//
//  Purpose:
//    A more suitable implementation of the vtkTubeFilter.  It assumes the
//    lines are all connected (it will return an error otherwise).
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  1, 2002
//
//  Modifications:
//    Eric Brugger, Wed Jan  9 11:29:49 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************
class VISIT_VTK_API vtkConnectedTubeFilter : public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkConnectedTubeFilter,vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;
    bool BuildConnectivityArrays(vtkPolyData *);

    // Description:
    // Construct object with radius 0.5, radius variation turned off, the number 
    // of sides set to 3, and radius factor of 10.
    static vtkConnectedTubeFilter *New();

    // Description:
    // Set the minimum tube radius (minimum because the tube radius may vary).
    vtkSetClampMacro(Radius,float,0.0,VTK_FLOAT_MAX);
    vtkGetMacro(Radius,float);

    // Description:
    // Set the number of sides for the tube. At a minimum, number of sides is 3.
    vtkSetClampMacro(NumberOfSides,int,3,VTK_INT_MAX);
    vtkGetMacro(NumberOfSides,int);

    // Description:
    // Set a boolean to control whether to create normals.
    // DefaultNormalOn is set.
    vtkSetMacro(CreateNormals,bool);
    vtkGetMacro(CreateNormals,bool);
    vtkBooleanMacro(CreateNormals,bool);

    // Description:
    // Turn on/off whether to cap the ends with polygons.
    vtkSetMacro(Capping,bool);
    vtkGetMacro(Capping,bool);
    vtkBooleanMacro(Capping,bool);

  protected:
    // ************************************************************************
    //  Class:  PointSequence
    //
    //  Purpose:
    //    Encapsulates a single doubly connected point sequence.
    //
    //    Jean Favre, Tue May  7 16:38:37 CEST 2013
    //    Used vtkIdType where needed
    // ************************************************************************
    struct PointSequence
    {
        int length;
        vtkIdType *index;
        vtkIdType *cellindex;
      public:
        PointSequence();
        ~PointSequence();
        void Init(int maxlen);
        void Add(vtkIdType i, vtkIdType ci);
    };

    // ************************************************************************
    //  Class:  PointSequenceList
    //
    //  Purpose:
    //    Encapsulates a list of separate point sequences.
    //
    //  Modifications:
    //    Rich Cook and Hank Childs, Thu Oct  2 16:31:45 PDT 2008
    //    Added data member to support tubing over loops.
    //
    //    Jean Favre, Tue May  7 16:38:37 CEST 2013
    //    Used vtkIdType where needed
    // ************************************************************************
    class PointSequenceList
    {
      private:
        // connectivity data
        int          len;
        vtkIdType         *numneighbors;
        vtkIdType         *connectivity[2];
        vtkIdType         *cellindex;
        vtkPoints   *pts;

        // traversal variables
        bool  *visited;
        vtkIdType    index;
        bool   lookforloops;
      public:
        PointSequenceList();
        ~PointSequenceList();
        bool Build(vtkPoints *points, vtkCellArray *lines);
        void InitTraversal();
        bool GetNextSequence(PointSequence &seq);
    };

  protected:
    vtkConnectedTubeFilter();
    ~vtkConnectedTubeFilter();

    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *) override;

    float Radius;       // minimum radius of tube
    int NumberOfSides;  // number of sides to create tube
    bool CreateNormals; // true to create normals
    bool Capping;       // true to cap ends

    PointSequenceList *pseqlist;

  private:
    vtkConnectedTubeFilter(const vtkConnectedTubeFilter&);  // Not implemented.
    void operator=(const vtkConnectedTubeFilter&);  // Not implemented.
};

#endif
