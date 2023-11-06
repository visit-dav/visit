// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtCurve3DFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_CURVE3D_FILE_FORMAT_H
#define AVT_CURVE3D_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>

class     DBOptionsAttributes;
class     vtkUnstructuredGrid;

// ****************************************************************************
//  Class: avtCurve3DFileFormat
//
//  Purpose:
//      A file format reader for 3D curves.
//
//  Copied from Curve2D plugin as of Aug 31, 2018 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
// ****************************************************************************

typedef enum
{
    VALID_POINT       = 0,
    HEADER,          /* 1 */
    WHITESPACE,      /* 2 */
    INVALID_POINT,   /* 3 */
    VALID_XVALUE
} Curve3DToken;


class avtCurve3DFileFormat : public avtSTSDFileFormat
{
  public:
                          avtCurve3DFileFormat(const char *, const DBOptionsAttributes *);
    virtual              ~avtCurve3DFileFormat();
    
    virtual void         *GetAuxiliaryData(const char *var, const char *type,
                                      void *args, DestructorFunction &);

    virtual const char   *GetType(void) { return "Curve3D File Format"; };

    virtual double        GetTime(void);
    virtual int           GetCycle(void);
    
    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    std::string           filename;
    bool                  readFile;
    bool                  gnuplotStyle;

    std::vector<vtkUnstructuredGrid *> curves;
    std::vector<std::string>   curveNames;
    std::vector<double>        zExtents;
    std::vector<double>        yExtents;
    std::vector<double>        xExtents;
    double                     curveTime;
    int                        curveCycle;
    double range[6] ; // geometrical mesh range

    void                  ReadFile(bool const clearData);
    Curve3DToken          GetPoint(istream &, double &, double &, double &,
                                   std::string &);
};


#endif
