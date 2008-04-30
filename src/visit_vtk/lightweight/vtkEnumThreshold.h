/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkThreshold.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkThreshold - extracts cells where scalar value in cell satisfies threshold criterion
// .SECTION Description
// vtkThreshold is a filter that extracts cells from any dataset type that
// satisfy a threshold criterion. A cell satisfies the criterion if the
// scalar value of (every or any) point satisfies the criterion. The
// criterion can take three forms: 1) greater than a particular value; 2)
// less than a particular value; or 3) between two values. The output of this
// filter is an unstructured grid.
//
// Note that scalar values are available from the point and cell attribute
// data.  By default, point data is used to obtain scalars, but you can
// control this behavior. See the AttributeMode ivar below.
//
// By default only the first scalar value is used in the decision. Use the ComponentMode
// and SelectedComponent ivars to control this behavior.

// .SECTION See Also
// vtkThresholdPoints vtkThresholdTextureCoords

// ----------------------------------------------------------------------------
//  Modifications:
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
//
//    Mark C. Miller and Jeremy Meredith, Tue Jul 10 08:45:44 PDT 2007
//    Added minEnumerationValue data member so this class can handle negative
//    enumeration values.
//    
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Added support for many new features; always include/exclude values,
//    partial cell modes and various enumeration modes.


#ifndef __vtkEnumThreshold_h
#define __vtkEnumThreshold_h
#include <visit_vtk_light_exports.h>

#include "vtkUnstructuredGridAlgorithm.h"

#include <vector>

using std::vector;

class vtkDataArray;

class VISIT_VTK_LIGHT_API vtkEnumThreshold : public vtkUnstructuredGridAlgorithm
{
  public:
    static vtkEnumThreshold *New();
    vtkTypeRevisionMacro(vtkEnumThreshold,vtkUnstructuredGridAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);

    void SetEnumerationRanges(const vector<double>&);
    void SetEnumerationValues(const vector<int> &vals);

    void SetEnumerationSelection(const vector<bool>&);

    void SetAlwaysExcludeRange(double min, double max);
    void SetAlwaysIncludeRange(double min, double max);
    void SetNAndMaxRForNChooseRMode(int n, int maxr);

    bool SetReturnEmptyIfAllCellsKept(bool ret);
    bool GetReturnEmptyIfAllCellsKept() const;
    bool GetAllCellsKeptInLastRequestData() const;

    typedef enum {
        Include,
        Exclude,
        Dissect
    } PartialCellMode;

    typedef enum {
        None,
        ByValue,
        ByRange,
        ByBitMask,
        ByNChooseR
    } EnumerationMode;

    PartialCellMode SetPartialCellMode(PartialCellMode m); // returns previous setting 
    EnumerationMode SetEnumerationMode(EnumerationMode em); // returns previous setting

  protected:
    vtkEnumThreshold();
    ~vtkEnumThreshold();

    // Usual data generation method
    virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

    virtual int FillInputPortInformation(int port, vtkInformation *info);
  
    int EvaluateComponents( vtkDataArray *scalars, vtkIdType id );
    bool IsInEnumerationRanges(double val);
    bool HasBitsSetInEnumerationMask(double val);
    bool HasValuesInEnumerationMap(double val);
  
    PartialCellMode   partialCellMode;
    EnumerationMode   enumMode;
    double            maxEnumerationValue;
    double            minEnumerationValue;
    double            alwaysExcludeMin;
    double            alwaysExcludeMax;
    double            alwaysIncludeMin;
    double            alwaysIncludeMax;

    vector<double>    enumerationRanges;
    int               lastRangeBin;

    vector<int>       enumerationValues;
    unsigned char    *enumerationMap;

    unsigned long long selectedEnumMask;

    int                  pascalsTriangleN;
    int                  pascalsTriangleR;
    vector<vector<int> > pascalsTriangleMap;

    bool              returnEmptyIfAllCellsKept;
    bool              allCellsKeptInLastRequestData;

  private:
    vtkEnumThreshold(const vtkEnumThreshold&);  // Not implemented.
    void operator=(const vtkEnumThreshold&);  // Not implemented.
};

#endif
