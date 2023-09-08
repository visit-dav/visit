// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtExtrudeStackedFilter.h
// ************************************************************************* //

#ifndef AVT_EXTRUDE_STACKED_FILTER_H
#define AVT_EXTRUDE_STACKED_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <ExtrudeStackedAttributes.h>

class vtkDataArray;
class vtkDataSet;
class vtkPoints;
class vtkPointSet;
class vtkRectilinearGrid;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class: avtExtrudeStackedFilter
//
//  Purpose:
//      A plugin operator for ExtrudeStacked.
//
//  Note: Adapted from the original extrude operator.
//
//  Programmer: Allen Sanderson
//  Creation:   August 31, 2023

class avtExtrudeStackedFilter : public avtPluginDataTreeIterator
{
  public:
                         avtExtrudeStackedFilter();
    virtual             ~avtExtrudeStackedFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtExtrudeStackedFilter"; };
    virtual const char  *GetDescription(void)
                             { return "ExtrudeStacked"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    ExtrudeStackedAttributes    atts;

    std::string defaultVariable;

    virtual avtContract_p ModifyContract(avtContract_p in_contract);

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);

    virtual void         PostExecute(void);
    virtual void         UpdateDataObjectInfo(void);

    void                 CopyVariables(vtkDataSet *in_ds, vtkDataSet *out_ds,
                                       const int *cellReplication = NULL) const;
    vtkPoints           *CreateExtrudePoints(vtkPoints *inPoints, int nLevels);

    vtkDataSet          *ExtrudeToRectilinearGrid (vtkDataSet  *in_ds) const;
    vtkDataSet          *ExtrudeToStructuredGrid  (vtkDataSet  *in_ds);
    vtkDataSet          *ExtrudeToUnstructuredGrid(vtkPointSet *in_ds);

    vtkDataSet          *ExtrudeToUnstructuredGrid(vtkRectilinearGrid  *in_ds,
                                                   vtkUnstructuredGrid *out_ds);
    vtkDataSet          *ExtrudeToUnstructuredGrid(vtkPointSet         *in_ds,
                                                   vtkUnstructuredGrid *out_ds);

    void                 ExtrudeExtents(double *dbounds) const;

    // Flags to indicte if the varArray is node or cell based
    bool cellData {false};
    bool nodeData {false};
    vtkDataArray *varArray {nullptr};

    double scalarMin {0};
    double scalarMax {0};

    // Number of stacked extrusions only set if there are two or more
    // variables.
    size_t num_stacked_extrusions {0};
    size_t varStackedIndex {0};
    int    varNum {0};

    doubleVector variableMinimums;
    doubleVector variableMaximums;
    doubleVector variableScales;

    // Names for the variables created for stacked extrusions
    std::string stackedVarNames[3]
      {"NodeHeight", "CellHeight", "VarIndex"};
};

#endif
