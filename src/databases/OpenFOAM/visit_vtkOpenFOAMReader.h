/*=========================================================================

  Program:   Visualization Toolkit
  Module:    visit_vtkOpenFOAMReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME visit_vtkOpenFOAMReader - reads a dataset in OpenFOAM format
// .SECTION Description
// visit_vtkOpenFOAMReader creates a multiblock dataset. It reads mesh
// information and time dependent data.  The polyMesh folders contain
// mesh information. The time folders contain transient data for the
// cells. Each folder can contain any number of data files.

// .SECTION Thanks
// Thanks to Terry Jordan of SAIC at the National Energy
// Technology Laboratory who developed this class.
// Please address all comments to Terry Jordan (terry.jordan@sa.netl.doe.gov).
// GUI Based selection of mesh regions and fields available in the case,
// minor bug fixes, strict memory allocation checks,
// minor performance enhancements by Philippose Rajan (sarith@rocketmail.com).
// Token-based FoamFile format lexer/parser,
// performance/stability/compatibility enhancements, gzipped file
// support, lagrangian field support, variable timestep support,
// builtin cell-to-point filter, pointField support, polyhedron
// decomposition support, OF 1.5 extended format support, multiregion
// support, old mesh format support, parallelization support for
// decomposed cases in conjunction with vtkPOpenFOAMReader, et. al. by
// Takuya Oshima of Niigata University, Japan (oshima@eng.niigata-u.ac.jp).

#ifndef __visit_vtkOpenFOAMReader_h
#define __visit_vtkOpenFOAMReader_h

#include <vtkMultiBlockDataSetAlgorithm.h>
#include <map>
#include <vector>

class vtkCollection;
class vtkCharArray;
class vtkDataArraySelection;
class vtkDoubleArray;
class vtkStdString;
class vtkStringArray;

class visit_vtkOpenFOAMReaderPrivate;

class visit_vtkOpenFOAMReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static visit_vtkOpenFOAMReader *New();
  vtkTypeMacro(visit_vtkOpenFOAMReader, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream &, vtkIndent) override;

  // Description:
  // Determine if the file can be readed with this reader.
  int CanReadFile(const char *);

  // Description:
  // Set/Get the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the number of cell arrays available in the input.
  int GetNumberOfCellArrays(void)
  { return this->GetNumberOfSelectionArrays(this->CellDataArraySelection); }

  // Description:
  // Get/Set whether the cell array with the given name is to
  // be read.
  int GetCellArrayStatus(const char *name)
  { return this->GetSelectionArrayStatus(this->CellDataArraySelection, name); }
  void SetCellArrayStatus(const char *name, int status)
  { this->SetSelectionArrayStatus(this->CellDataArraySelection, name, status); }

  // Description:
  // Get the name of the  cell array with the given index in
  // the input.
  const char *GetCellArrayName(int index)
  { return this->GetSelectionArrayName(this->CellDataArraySelection, index); }

  // Description:
  // Return whether a cell array with the given name exists.
  // the input.
  int GetCellArrayExists(const char *name);

  // Description:
  // Turn on/off all cell arrays.
  void DisableAllCellArrays()
  { this->DisableAllSelectionArrays(this->CellDataArraySelection); }
  void EnableAllCellArrays()
  { this->EnableAllSelectionArrays(this->CellDataArraySelection); }

  // Description:
  // Get the number of point arrays available in the input.
  int GetNumberOfPointArrays(void)
  { return this->GetNumberOfSelectionArrays(this->PointDataArraySelection); }

  // Description:
  // Get/Set whether the point array with the given name is to
  // be read.
  int GetPointArrayStatus(const char *name)
  { return this->GetSelectionArrayStatus(this->PointDataArraySelection, name); }
  void SetPointArrayStatus(const char *name, int status)
  { this->SetSelectionArrayStatus(this->PointDataArraySelection,
    name, status); }

  // Description:
  // Get the name of the  point array with the given index in
  // the input.
  const char *GetPointArrayName(int index)
  { return this->GetSelectionArrayName(this->PointDataArraySelection, index); }

  // Description:
  // Return whether a point array with the given name exists.
  // the input.
  int GetPointArrayExists(const char *name);

  // Description:
  // Turn on/off all point arrays.
  void DisableAllPointArrays()
  { this->DisableAllSelectionArrays(this->PointDataArraySelection); }
  void EnableAllPointArrays()
  { this->EnableAllSelectionArrays(this->PointDataArraySelection); }

  // Description:
  // Get the number of Lagrangian arrays available in the input.
  int GetNumberOfLagrangianArrays(void)
  { return this->GetNumberOfSelectionArrays(
    this->LagrangianDataArraySelection); }

  // Description:
  // Get/Set whether the Lagrangian array with the given name is to
  // be read.
  int GetLagrangianArrayStatus(const char *name)
  { return this->GetSelectionArrayStatus(this->LagrangianDataArraySelection,
    name); }
  void SetLagrangianArrayStatus(const char *name, int status)
  { this->SetSelectionArrayStatus(this->LagrangianDataArraySelection, name,
    status); }

  // Description:
  // Get the name of the  Lagrangian array with the given index in
  // the input.
  const char* GetLagrangianArrayName(int index)
  { return this->GetSelectionArrayName(this->LagrangianDataArraySelection,
    index); }

  // Description:
  // Return whether a lagrangian array with the given name exists.
  // the input.
  int GetLagrangianArrayExists(const char *name);

  // Description:
  // Turn on/off all Lagrangian arrays.
  void DisableAllLagrangianArrays()
  { this->DisableAllSelectionArrays(this->LagrangianDataArraySelection); }
  void EnableAllLagrangianArrays()
  { this->EnableAllSelectionArrays(this->LagrangianDataArraySelection); }

  // Description:
  // Get the number of Patches (including Internal Mesh) available in the input.
  int GetNumberOfPatchArrays(void)
  { return this->GetNumberOfSelectionArrays(this->PatchDataArraySelection); }

  // Description:
  // Get/Set whether the Patch with the given name is to
  // be read.
  int GetPatchArrayStatus(const char *name)
  { return this->GetSelectionArrayStatus(this->PatchDataArraySelection, name); }
  void SetPatchArrayStatus(const char *name, int status)
  { this->SetSelectionArrayStatus(this->PatchDataArraySelection, name,
    status); }

  // Description:
  // Get the name of the Patch with the given index in
  // the input.
  const char *GetPatchArrayName(int index)
  { return this->GetSelectionArrayName(this->PatchDataArraySelection, index); }

  // Description:
  // Turn on/off all Patches including the Internal Mesh.
  void DisableAllPatchArrays()
  { this->DisableAllSelectionArrays(this->PatchDataArraySelection); }
  void EnableAllPatchArrays()
  { this->EnableAllSelectionArrays(this->PatchDataArraySelection); }

  // Description:
  // Set/Get whether to create cell-to-point translated data for cell-type data
  vtkSetMacro(CreateCellToPoint,bool);
  vtkGetMacro(CreateCellToPoint,bool);
  vtkBooleanMacro(CreateCellToPoint, bool);

  // Description:
  // Set/Get whether mesh is to be cached.
  vtkSetMacro(CacheMesh, bool);
  vtkGetMacro(CacheMesh, bool);
  vtkBooleanMacro(CacheMesh, bool);

  // Description:
  // Set/Get whether polyhedra are to be decomposed.
  vtkSetMacro(DecomposePolyhedra, bool);
  vtkGetMacro(DecomposePolyhedra, bool);
  vtkBooleanMacro(DecomposePolyhedra, bool);

  // Option for reading old binary lagrangian/positions format
  // Description:
  // Set/Get whether the lagrangian/positions is in OF 1.3 format
  vtkSetMacro(PositionsIsIn13Format, bool);
  vtkGetMacro(PositionsIsIn13Format, bool);
  vtkBooleanMacro(PositionsIsIn13Format, bool);

  // Description:
  // Determine if time directories are to be listed according to controlDict
  vtkSetMacro(ListTimeStepsByControlDict, bool);
  vtkGetMacro(ListTimeStepsByControlDict, bool);
  vtkBooleanMacro(ListTimeStepsByControlDict, bool);

  // Description:
  // Add dimensions to array names
  vtkSetMacro(AddDimensionsToArrayNames, bool);
  vtkGetMacro(AddDimensionsToArrayNames, bool);
  vtkBooleanMacro(AddDimensionsToArrayNames, bool);

  // Description:
  // Set/Get whether zones will be read.
  vtkSetMacro(ReadZones, bool);
  vtkGetMacro(ReadZones, bool);
  vtkBooleanMacro(ReadZones, bool);

  void SetRefresh() { this->Refresh = true; this->Modified(); }

  void SetParent(visit_vtkOpenFOAMReader *parent) { this->Parent = parent; }
  int MakeInformationVector(vtkInformationVector *, const vtkStdString &);
  bool SetTimeValue(const double);
  vtkDoubleArray *GetTimeValues();
  int MakeMetaDataAtTimeStep(const bool);

  //BTX
  friend class visit_vtkOpenFOAMReaderPrivate;
  //ETX

  vtkStdString GetCellArrayClassName(const char *name);
  vtkStdString GetPointArrayClassName(const char *name);
  vtkStdString GetLagrangianArrayClassName(const char *name);

  int GetNumberOfCellZones() 
    { return static_cast<int>(this->CellZones.size()); }
  int GetNumberOfFaceZones()
    { return static_cast<int>(this->FaceZones.size()); }
  int GetNumberOfPointZones()
    { return static_cast<int>(this->PointZones.size()); }

  vtkStdString GetCellZoneName(int);
  vtkStdString GetFaceZoneName(int);
  vtkStdString GetPointZoneName(int);

protected:
  // refresh flag
  bool Refresh;

  // for creating cell-to-point translated data
  bool CreateCellToPoint;

  // for caching mesh
  bool CacheMesh;

  // for decomposing polyhedra on-the-fly
  bool DecomposePolyhedra;

  // for reading old binary lagrangian/positions format
  bool PositionsIsIn13Format;

  // for reading point/face/cell-Zones
  bool ReadZones;

  // determine if time directories are listed according to controlDict
  bool ListTimeStepsByControlDict;

  // add dimensions to array names
  bool AddDimensionsToArrayNames;

  char *FileName;
  vtkCharArray *CasePath;
  vtkCollection *Readers;

  // DataArraySelection for Patch / Region Data
  vtkDataArraySelection *PatchDataArraySelection;
  vtkDataArraySelection *CellDataArraySelection;
  vtkDataArraySelection *PointDataArraySelection;
  vtkDataArraySelection *LagrangianDataArraySelection;
  std::vector<vtkStdString> CellZones;
  std::vector<vtkStdString> FaceZones;
  std::vector<vtkStdString> PointZones;

  // old selection status
  vtkMTimeType PatchSelectionMTimeOld;
  vtkMTimeType CellSelectionMTimeOld;
  vtkMTimeType PointSelectionMTimeOld;
  vtkMTimeType LagrangianSelectionMTimeOld;

  // preserved old information
  vtkStdString *FileNameOld;
  bool ListTimeStepsByControlDictOld;
  bool CreateCellToPointOld;
  bool DecomposePolyhedraOld;
  bool PositionsIsIn13FormatOld;
  bool AddDimensionsToArrayNamesOld;
  bool ReadZonesOld;

  // paths to Lagrangians
  vtkStringArray *LagrangianPaths;

  std::map<vtkStdString, vtkStdString> CellArrayClassName;
  std::map<vtkStdString, vtkStdString> PointArrayClassName;
  std::map<vtkStdString, vtkStdString> LagrangianArrayClassName;

  // number of reader instances
  int NumberOfReaders;
  // index of the active reader
  int CurrentReaderIndex;

  visit_vtkOpenFOAMReader();
  ~visit_vtkOpenFOAMReader();
  int RequestInformation(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *) override;

  void CreateCasePath(vtkStdString &, vtkStdString &);
  void SetTimeInformation(vtkInformationVector *, vtkDoubleArray *);
  void CreateCharArrayFromString(vtkCharArray *, const char *, vtkStdString &);
  void UpdateStatus();
  void UpdateProgress(double);

private:
  visit_vtkOpenFOAMReader *Parent;

  visit_vtkOpenFOAMReader(const visit_vtkOpenFOAMReader&);  // Not implemented.
  void operator=(const visit_vtkOpenFOAMReader&);  // Not implemented.

  int GetNumberOfSelectionArrays(vtkDataArraySelection *);
  int GetSelectionArrayStatus(vtkDataArraySelection *, const char *);
  void SetSelectionArrayStatus(vtkDataArraySelection *, const char *, int);
  const char *GetSelectionArrayName(vtkDataArraySelection *, int);
  void DisableAllSelectionArrays(vtkDataArraySelection *);
  void EnableAllSelectionArrays(vtkDataArraySelection *);

  void AddSelectionNames(vtkDataArraySelection *, vtkStringArray *);
};

#endif
