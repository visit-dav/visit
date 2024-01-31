// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkDataSetRemoveGhostCells - Removes ghost cells whose ghost level 
// is above a specified value

#ifndef __vtkDataSetRemoveGhostCells_h
#define __vtkDataSetRemoveGhostCells_h
#include <visit_vtk_exports.h>

#include "vtkDataSetAlgorithm.h"

// ****************************************************************************
//  Class: vtkDataSetRemoveGhostCells
//
//  Modifications: 
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002
//    Removed FindCellMins and FindCellMax methods.
//
//    Hank Childs, Mon Aug 30 16:27:16 PDT 2004
//    Removed GhostLevels data member.
//
//    Hank Childs, Thu Mar  2 11:14:53 PST 2006
//    Add GenericExecute.
//
//    Hank Childs, Sun Oct 28 10:48:50 PST 2007
//    Added GhostZoneTypesToRemove
//
//    Eric Brugger, Wed Jan  9 14:56:34 PST 2013
//    Modified to inherit from vtkDataSetAlgorithm.
//
//    Kevin Griffin, Thu Jul 30 08:32:43 PDT 2020
//    Add ForceConfirmRegion
//
// ****************************************************************************
class VISIT_VTK_API vtkDataSetRemoveGhostCells : public vtkDataSetAlgorithm
{
public:
  static vtkDataSetRemoveGhostCells *New();
  vtkTypeMacro(vtkDataSetRemoveGhostCells, vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  
  vtkSetMacro(GhostNodeTypesToRemove,unsigned char);
  vtkGetMacro(GhostNodeTypesToRemove,unsigned char);

  vtkSetMacro(GhostZoneTypesToRemove,unsigned char);
  vtkGetMacro(GhostZoneTypesToRemove,unsigned char);
    
  vtkSetMacro(ForceConfirmRegion,bool);
  vtkGetMacro(ForceConfirmRegion,bool);

protected:
  vtkDataSetRemoveGhostCells();
  ~vtkDataSetRemoveGhostCells() {};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;

  // Specific data generation methods
  void StructuredGridExecute();
  void UnstructuredGridExecute();
  void RectilinearGridExecute();
  void PolyDataExecute();
  void GenericExecute();

  vtkDataSet *input;
  vtkDataSet *output;

  unsigned char GhostNodeTypesToRemove;
  unsigned char GhostZoneTypesToRemove;
    
  bool ForceConfirmRegion;

private:
  vtkDataSetRemoveGhostCells(const vtkDataSetRemoveGhostCells&);
  void operator=(const vtkDataSetRemoveGhostCells&);

  void ConfirmRegion(unsigned char *ghosts, int *dims, int *voi);
};

#endif
