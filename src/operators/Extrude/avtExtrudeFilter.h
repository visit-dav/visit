// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtExtrudeFilter.h
// ************************************************************************* //

#ifndef AVT_EXTRUDE_FILTER_H
#define AVT_EXTRUDE_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <ExtrudeAttributes.h>

class vtkDataArray;
class vtkDataSet;
class vtkPoints;
class vtkPointSet;
class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtExtrudeFilter
//
//  Purpose:
//      A plugin operator for Extrude.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jun 22 13:48:57 PST 2011
//
//  Modifications:
//    Eric Brugger, Thu Jul 24 13:31:45 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtExtrudeFilter : public avtPluginDataTreeIterator
{
  public:
                         avtExtrudeFilter();
    virtual             ~avtExtrudeFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtExtrudeFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Extrude"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    ExtrudeAttributes    atts;

    std::string mainVariable;  

    virtual avtContract_p ModifyContract(avtContract_p in_contract);

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);

    virtual void         UpdateDataObjectInfo(void);

    void                 CopyVariables(vtkDataSet *in_ds, vtkDataSet *out_ds, 
                                       int nLevels, const int *cellReplication = NULL) const;
    vtkPoints           *CreateExtrudedPoints(vtkPoints *inPoints, int nLevels);
    void                 ExtrudeExtents(double *dbounds) const;
    vtkDataSet          *ExtrudeToRectilinearGrid(vtkDataSet *in_ds) const;
    vtkDataSet          *ExtrudeToStructuredGrid(vtkDataSet *in_ds);
    vtkDataSet          *ExtrudeToUnstructuredGrid(vtkPointSet *in_ds);

    vtkDataSet  *ExtrudeCellVariableToUnstructuredGrid(vtkRectilinearGrid *in_ds);
    vtkDataSet  *ExtrudeCellVariableToUnstructuredGrid(vtkPointSet *in_ds);

    bool cellData{false};
    bool nodeData{false};
    vtkDataArray *varArray{nullptr};

    double minScalar{0};
    double maxScalar{0};
};

#endif
