/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLReader.h,v $
  Language:  C++
  Date:      $Date: 2003/06/16 17:59:34 $
  Version:   $Revision: 1.9 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLReader - Superclass for VTK's XML format readers.
// .SECTION Description
// vtkVisItXMLReader uses vtkVisItXMLDataParser to parse a VTK XML input file.
// Concrete subclasses then traverse the parsed file structure and
// extract data.

#ifndef __vtkVisItXMLReader_h
#define __vtkVisItXMLReader_h

#include "vtkSource.h"

class vtkCallbackCommand;
class vtkDataArray;
class vtkDataArraySelection;
class vtkDataSet;
class vtkDataSetAttributes;
class vtkVisItXMLDataElement;
class vtkVisItXMLDataParser;

class vtkVisItXMLReader : public vtkSource
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLReader,vtkSource);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  
  // Description:
  // Test whether the file with the given name can be read by this
  // reader.
  virtual int CanReadFile(const char* name);
  
  // Description:
  // Get the output as a vtkDataSet pointer.
  vtkDataSet* GetOutputAsDataSet();
  vtkDataSet* GetOutputAsDataSet(int index);
  
  // Description:
  // Get the data array selection tables used to configure which data
  // arrays are loaded by the reader.
  vtkGetObjectMacro(PointDataArraySelection, vtkDataArraySelection);
  vtkGetObjectMacro(CellDataArraySelection, vtkDataArraySelection);
  
  // Description:  
  // Get the number of point or cell arrays available in the input.
  int GetNumberOfPointArrays();
  int GetNumberOfCellArrays();
  
  // Description:
  // Get the name of the point or cell array with the given index in
  // the input.
  const char* GetPointArrayName(int index);
  const char* GetCellArrayName(int index);
  
  // Description:
  // Get/Set whether the point or cell array with the given name is to
  // be read.
  int GetPointArrayStatus(const char* name);
  int GetCellArrayStatus(const char* name);
  void SetPointArrayStatus(const char* name, int status);  
  void SetCellArrayStatus(const char* name, int status);  
  
protected:
  vtkVisItXMLReader();
  ~vtkVisItXMLReader();
  
  // Standard pipeline exectution methods.
  void ExecuteInformation();
  void ExecuteData(vtkDataObject* output);
  
  // Pipeline execution methods to be defined by subclass.  Called by
  // corresponding Execute methods after appropriate setup has been
  // done.
  virtual void ReadXMLInformation();
  virtual void ReadXMLData();
  
  // Get the name of the data set being read.
  virtual const char* GetDataSetName()=0;
  
  // Test if the reader can read a file with the given version number.
  virtual int CanReadFileVersion(int major, int minor);
  
  // Setup the output with no data available.  Used in error cases.
  virtual void SetupEmptyOutput()=0;
  
  // Setup the output's information and data without allocation.
  virtual void SetupOutputInformation();
  
  // Setup the output's information and data with allocation.
  virtual void SetupOutputData();
  
  // Read the primary element from the file.  This is the element
  // whose name is the value returned by GetDataSetName().
  virtual int ReadPrimaryElement(vtkVisItXMLDataElement* ePrimary);
  
  // Read the top-level element from the file.  This is always the
  // VTKFile element.
  int ReadVTKFile(vtkVisItXMLDataElement* eVTKFile);  
  
  // Create a vtkDataArray from its cooresponding XML representation.
  // Does not allocate.
  vtkDataArray* CreateDataArray(vtkVisItXMLDataElement* da);
  
  // Internal utility methods.
  int OpenVTKFile();
  void CloseVTKFile();
  virtual void CreateXMLParser();
  virtual void DestroyXMLParser();
  void SetupCompressor(const char* type);
  int CanReadFileVersionString(const char* version);
  
  // Utility methods for subclasses.
  int IntersectExtents(int* extent1, int* extent2, int* result);
  int Min(int a, int b);
  int Max(int a, int b);
  void ComputePointDimensions(int* extent, int* dimensions);
  void ComputeCellDimensions(int* extent, int* dimensions);
  void ComputePointIncrements(int* extent, int* increments);
  void ComputeCellIncrements(int* extent, int* increments);
  unsigned int GetStartTuple(int* extent, int* increments,
                             int i, int j, int k);
  void ReadAttributeIndices(vtkVisItXMLDataElement* eDSA,
                            vtkDataSetAttributes* dsa);
  char** CreateStringArray(int numStrings);
  void DestroyStringArray(int numStrings, char** strings);  
  
  // Setup the data array selections for the input's set of arrays.
  void SetDataArraySelections(vtkVisItXMLDataElement* eDSA,
                              vtkDataArraySelection* sel);
  
  // Check whether the given array element is an enabled array.
  int PointDataArrayIsEnabled(vtkVisItXMLDataElement* ePDA);
  int CellDataArrayIsEnabled(vtkVisItXMLDataElement* eCDA);
  
  // Callback registered with the SelectionObserver.
  static void SelectionModifiedCallback(vtkObject* caller, unsigned long eid,
                                        void* clientdata, void* calldata);
  
  // The vtkVisItXMLDataParser instance used to hide XML reading details.
  vtkVisItXMLDataParser* XMLParser;
  
  // The input file's name.
  char* FileName;
  
  // The stream used to read the input.
  istream* Stream;
  
  // The array selections.
  vtkDataArraySelection* PointDataArraySelection;
  vtkDataArraySelection* CellDataArraySelection;
  
  // The observer to modify this object when the array selections are
  // modified.
  vtkCallbackCommand* SelectionObserver;
  
  // Whether there was an error reading the file in ExecuteInformation.
  int InformationError;
  
  // Whether there was an error reading the file in ExecuteData.
  int DataError;
  
  // The index of the output on which ExecuteData is currently
  // running.
  int CurrentOutput;
  
  // The current range over which progress is moving.  This allows for
  // incrementally fine-tuned progress updates.
  virtual void GetProgressRange(float* range);
  virtual void SetProgressRange(float* range, int curStep, int numSteps);
  virtual void SetProgressRange(float* range, int curStep, float* fractions);
  virtual void UpdateProgressDiscrete(float progress);
  float ProgressRange[2];

  int AxesEmpty[3];

private:
  // The stream used to read the input if it is in a file.
  ifstream* FileStream;  
  
private:
  vtkVisItXMLReader(const vtkVisItXMLReader&);  // Not implemented.
  void operator=(const vtkVisItXMLReader&);  // Not implemented.
};

#endif
