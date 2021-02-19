/*=========================================================================

  Program:   Visualization Toolkit
  Module:    visit_vtkPOpenFOAMReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME visit_vtkPOpenFOAMReader - reads a decomposed dataset in OpenFOAM format
// .SECTION Description
// visit_vtkPOpenFOAMReader creates a multiblock dataset. It reads
// parallel-decomposed mesh information and time dependent data.  The
// polyMesh folders contain mesh information. The time folders contain
// transient data for the cells. Each folder can contain any number of
// data files.

// .SECTION Thanks
// This class was developed by Takuya Oshima at Niigata University,
// Japan (oshima@eng.niigata-u.ac.jp).

#ifndef __visit_vtkPOpenFOAMReader_h
#define __visit_vtkPOpenFOAMReader_h

#include "visit_vtkOpenFOAMReader.h"

class vtkDataArraySelection;

class visit_vtkPOpenFOAMReader : public visit_vtkOpenFOAMReader
{
public:
  //BTX
  enum caseType { DECOMPOSED_CASE = 0, RECONSTRUCTED_CASE = 1 };
  //ETX
  static visit_vtkPOpenFOAMReader *New();
  vtkTypeMacro(visit_vtkPOpenFOAMReader, visit_vtkOpenFOAMReader);

  void PrintSelf(ostream &os, vtkIndent indent) override;

  // Description:
  // Set and get case type. 0 = decomposed case, 1 = reconstructed case.
  void SetCaseType(const int t);
  vtkGetMacro(CaseType, caseType);

protected:
  visit_vtkPOpenFOAMReader();
  ~visit_vtkPOpenFOAMReader();

  int RequestInformation(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *) override;

private:
  caseType CaseType;
  vtkMTimeType MTimeOld;
  int NumProcesses;
  int ProcessId;

  visit_vtkPOpenFOAMReader(const visit_vtkPOpenFOAMReader &); // Not implemented.
  void operator=(const visit_vtkPOpenFOAMReader &); // Not implemented.

  void GatherMetaData();
  void BroadcastStatus(int &);
  void Broadcast(vtkStringArray *);
  void AllGather(vtkStringArray *);
  void AllGather(vtkDataArraySelection *);
};

#endif
