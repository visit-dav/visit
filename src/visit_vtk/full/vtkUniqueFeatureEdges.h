// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkUniqueFeatureEdges - extract boundary, non-manifold, and/or 
// sharp edges from polygonal data
// .SECTION Description
// vtkUniqueFeatureEdges is a filter to extract special types of edges from
// input polygonal data. These edges are either 1) boundary (used by 
// one polygon) or a line cell; 2) non-manifold (used by three or more 
// polygons); 3) feature edges (edges used by two triangles and whose
// dihedral angle > FeatureAngle); or 4) manifold edges (edges used by
// exactly two polygons). 

// .SECTION Caveats
// This is almost exactly like vtkFeatureEdges with the exceptions:
//     (1)  there is no edge 'coloring'. 
//     (2)  vtkEdgeTable is used to ensure no duplicate edges are inserted. 
//     (3)  ghostCells compared to UpdateGhostLevel instead of 0. 

// .SECTION See Also
// vtkFeatureVertices

#ifndef __vtkUniqueFeatureEdges_h
#define __vtkUniqueFeatureEdges_h
#include <visit_vtk_exports.h>

#include <vtkPolyDataAlgorithm.h>

class vtkPointLocator;

// ****************************************************************************
//  Class: vtkUniqueFeatureEdges
//
//  Modifications:
//    Eric Brugger, Wed Jan  9 12:28:48 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

class VISIT_VTK_API vtkUniqueFeatureEdges : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkUniqueFeatureEdges,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Construct object with feature angle = 30; all types of edges extracted
  // and colored.
  static vtkUniqueFeatureEdges *New();

  // Description:
  // Turn on/off the extraction of boundary edges.
  vtkSetMacro(BoundaryEdges,bool);
  vtkGetMacro(BoundaryEdges,bool);
  vtkBooleanMacro(BoundaryEdges,bool);

  // Description:
  // Turn on/off the extraction of feature edges.
  vtkSetMacro(FeatureEdges,bool);
  vtkGetMacro(FeatureEdges,bool);
  vtkBooleanMacro(FeatureEdges,bool);

  // Description:
  // Specify the feature angle for extracting feature edges.
  vtkSetClampMacro(FeatureAngle,double,0.0,180.0);
  vtkGetMacro(FeatureAngle,double);

  // Description:
  // Turn on/off the extraction of non-manifold edges.
  vtkSetMacro(NonManifoldEdges,bool);
  vtkGetMacro(NonManifoldEdges,bool);
  vtkBooleanMacro(NonManifoldEdges,bool);

  // Description:
  // Turn on/off the extraction of manifold edges.
  vtkSetMacro(ManifoldEdges,bool);
  vtkGetMacro(ManifoldEdges,bool);
  vtkBooleanMacro(ManifoldEdges,bool);

  // Description:
  // Set / get a spatial locator for merging points. By
  // default an instance of vtkMergePoints is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified.
  void CreateDefaultLocator();

  // Description:
  // Return MTime also considering the locator.
  vtkMTimeType GetMTime() override;

protected:
  vtkUniqueFeatureEdges();
  ~vtkUniqueFeatureEdges();

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int RequestUpdateExtent(vtkInformation *,
                                  vtkInformationVector **,
                                  vtkInformationVector *) override;
  
  double FeatureAngle;
  bool BoundaryEdges;
  bool FeatureEdges;
  bool NonManifoldEdges;
  bool ManifoldEdges;
  vtkPointLocator *Locator;

private:
  vtkUniqueFeatureEdges(const vtkUniqueFeatureEdges&);
  void operator=(const vtkUniqueFeatureEdges&);
};

#endif
