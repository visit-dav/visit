// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_MULTI_SPLITTER_H
#define VTK_MULTI_SPLITTER_H
#include <visit_vtk_exports.h>

#include "vtkCSGFixedLengthBitField.h"
#include "vtkUnstructuredGridAlgorithm.h"

#include <vector>

class vtkImplicitFunction;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class:  vtkMultiSplitter
//
//  Purpose:
//    Splits a rectilinear dataset using multiple implicit functions,
//    tagging output cells as it splits, and later allowing extraction
//    of various region sets as whole data sets.
//
//  Note: Copied largely from vtkVisItSplitter
//
//  Programmer:  Eric Brugger
//  Creation:    July 23, 2012
//
//  Modifications:
//    Eric Brugger, Thu Apr  3 08:20:06 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

class VISIT_VTK_API vtkMultiSplitter :
  public vtkUnstructuredGridAlgorithm
{
  public:
    vtkTypeMacro(vtkMultiSplitter,vtkUnstructuredGridAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkMultiSplitter *New();

    virtual void SetTagBitField(std::vector<vtkCSGFixedLengthBitField> *);
    virtual void SetClipFunctions(double *, int);

  protected:
    vtkMultiSplitter();
    ~vtkMultiSplitter();

    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *) override;
    virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  private:
    double *bounds;
    int    nBounds;
    std::vector<vtkCSGFixedLengthBitField> *newTags;

    vtkMultiSplitter(const vtkMultiSplitter&);  // Not implemented.
    void operator=(const vtkMultiSplitter&);    // Not implemented.
};

#endif
