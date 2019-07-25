// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_VISIT_SPLITTER_H
#define VTK_VISIT_SPLITTER_H
#include <visit_vtk_exports.h>

#include <vtkCSGFixedLengthBitField.h>
#include <vtkUnstructuredGridAlgorithm.h>

#include <vector>

class vtkImplicitFunction;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class:  vtkVisItSplitter
//
//  Purpose:
//    Splits a dataset using an implicit function or a scalars variable,
//    tagging output cells as it splits, and later allowing extraction
//    of various region sets as whole data sets.
//
//  Note: Copied largely from vtkVisItClipper
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 24, 2010
//
//  Modifications:
//    Eric Brugger, Wed Jul 25 10:09:42 PDT 2012
//    Increase the number of boundaries that can be handled by the mulit-pass
//    CSG discretization from 128 to 512.
//
//    Eric Brugger, Thu Apr  3 08:25:20 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItSplitter :
    public vtkUnstructuredGridAlgorithm
{
  public:
    vtkTypeMacro(vtkVisItSplitter,vtkUnstructuredGridAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkVisItSplitter *New();

    virtual void SetRemoveWholeCells(bool);
    virtual void SetClipFunction(vtkImplicitFunction*);
    virtual void SetClipScalars(vtkDataArray *, float);
    virtual void SetInsideOut(bool);
    virtual void SetUseZeroCrossings(bool);
    virtual void SetOldTagBitField(std::vector<vtkCSGFixedLengthBitField> *);
    virtual void SetNewTagBitField(std::vector<vtkCSGFixedLengthBitField> *);
    virtual void SetNewTagBit(int);

    void SetCellList(const vtkIdType *, vtkIdType);
    virtual void SetUpClipFunction(int) { ; };

    struct FilterState
    {
        FilterState();
       ~FilterState();
        void SetCellList(const vtkIdType *, vtkIdType);
        void SetClipFunction(vtkImplicitFunction *func);
        void SetClipScalars(vtkDataArray *, double);

        const vtkIdType     *CellList;
        vtkIdType            CellListSize;
  
        vtkImplicitFunction *clipFunction;
        vtkDataArray        *scalarArrayAsVTK;
        double               scalarCutoff;

        bool                 removeWholeCells;
        bool                 insideOut;
        bool                 useZeroCrossings;

        int                  newTagBit;
        std::vector<vtkCSGFixedLengthBitField> *newTags;
        std::vector<vtkCSGFixedLengthBitField> *oldTags;
    };

  protected:
    vtkVisItSplitter();
    ~vtkVisItSplitter();

    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *) override;
    virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  private:
     // Contains the state for the filter.
    FilterState state;

    vtkVisItSplitter(const vtkVisItSplitter&);  // Not implemented.
    void operator=(const vtkVisItSplitter&);  // Not implemented.
};

#endif
