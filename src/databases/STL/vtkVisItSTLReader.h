/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItSTLReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItSTLReader - read ASCII or binary stereo lithography files
// .SECTION Description
// vtkVisItSTLReader is a source object that reads ASCII or binary stereo 
// lithography files (.stl files). The FileName must be specified to
// vtkVisItSTLReader. The object automatically detects whether the file is
// ASCII or binary.
//
// .stl files are quite inefficient since they duplicate vertex 
// definitions. By setting the Merging boolean you can control whether the 
// point data is merged after reading. Merging is performed by default, 
// however, merging requires a large amount of temporary storage since a 
// 3D hash table must be constructed.

// .SECTION Caveats
// Binary files written on one system may not be readable on other systems.
// vtkSTLWriter uses VAX or PC byte ordering and swaps bytes on other systems.

#ifndef __vtkVisItSTLReader_h
#define __vtkVisItSTLReader_h

#include "vtkPolyDataSource.h"

class vtkCellArray;
class vtkFloatArray;
class vtkPointLocator;
class vtkPoints;

// Modifications:
//   Jeremy Meredith, Thu Jan  7 12:19:03 EST 2010
//   Added support for optional strict error checking.
//
class vtkVisItSTLReader : public vtkPolyDataSource 
{
public:
  vtkTypeRevisionMacro(vtkVisItSTLReader,vtkPolyDataSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with merging set to true.
  static vtkVisItSTLReader *New();

  // Description:
  // Overload standard modified time function. If locator is modified,
  // then this object is modified as well.
  unsigned long GetMTime();

  // Description:
  // Specify file name of stereo lithography file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Turn on/off merging of points/triangles.
  vtkSetMacro(Merging,int);
  vtkGetMacro(Merging,int);
  vtkBooleanMacro(Merging,int);

  // Description:
  // Turn on/off stricter file detection.
  vtkSetMacro(Strict,int);
  vtkGetMacro(Strict,int);
  vtkBooleanMacro(Strict,int);

  // Description:
  // Turn on/off tagging of solids with scalars.
  vtkSetMacro(ScalarTags,int);
  vtkGetMacro(ScalarTags,int);
  vtkBooleanMacro(ScalarTags,int);

  // Description:
  // Specify a spatial locator for merging points. By
  // default an instance of vtkMergePoints is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified.
  void CreateDefaultLocator();

protected:
  vtkVisItSTLReader();
  ~vtkVisItSTLReader();

  char *FileName;
  int Merging;
  int ScalarTags;
  int Strict;
  int CheckOnly;
  vtkPointLocator *Locator;

  void Execute();
  int ReadBinarySTL(FILE *fp, vtkPoints*, vtkCellArray*);
  int ReadASCIISTL(FILE *fp, vtkPoints*, vtkCellArray*, 
                   vtkFloatArray* scalars=0);
  int GetSTLFileType(FILE *fp);
private:
  vtkVisItSTLReader(const vtkVisItSTLReader&);  // Not implemented.
  void operator=(const vtkVisItSTLReader&);  // Not implemented.
};

#endif


