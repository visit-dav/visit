// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


#ifndef __visit_vtkOpenFOAMReader_h
#define __visit_vtkOpenFOAMReader_h

#include <vtkOpenFOAMReader.h>

class vtkDataArraySelection;

class visit_vtkOpenFOAMReader : public vtkOpenFOAMReader
{
public:
  //BTX
  enum caseType { DECOMPOSED_CASE = 0, RECONSTRUCTED_CASE = 1 };
  //ETX
  static visit_vtkOpenFOAMReader *New();
  vtkTypeMacro(visit_vtkOpenFOAMReader, vtkOpenFOAMReader)

  void PrintSelf(ostream &os, vtkIndent indent) override;

  // Description:
  // Set and get case type. 0 = decomposed case, 1 = reconstructed case.
  void SetCaseType(const int t);
  vtkGetMacro(CaseType, caseType);

  void DecomposePolyhedraOn()
    {
    this->DecomposePolyhedra = 1;
    this->Refresh = true;
    this->Modified();
    }
  void DecomposePolyhedraOff()
    {
    this->DecomposePolyhedra = 0;
    this->Refresh = true;
    this->Modified();
    }

  int GetCellArrayExists(const char *name);
  int GetPointArrayExists(const char *name);
  int GetLagrangianArrayExists(const char *name);

protected:
  visit_vtkOpenFOAMReader();
  ~visit_vtkOpenFOAMReader();

  int RequestInformation(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *) override;

private:
  caseType CaseType;
  vtkMTimeType MTimeOld;

  visit_vtkOpenFOAMReader(const visit_vtkOpenFOAMReader &) = delete;
  void operator=(const visit_vtkOpenFOAMReader &) = delete;

};

#endif
