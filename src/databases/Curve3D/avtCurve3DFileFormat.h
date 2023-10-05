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
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Aug  1 21:16:55 PDT 2003
//    Made the format be a STSD.
//
//    Kathleen Bonnell, Fri Oct 28 13:02:51 PDT 2005 
//    Added methods GetTime, GetCycle, and members curveTime, curveCycle.
//
//    Kathleen Bonnell, Mon Jul 31 10:15:00 PDT 2006 
//    Represent curve as 1D RectilinearGrid instead of PolyData. 
//
//    Kathleen Bonnell, Thu Aug  3 08:42:33 PDT 2006 
//    Added dataExtents. 
//
//    Mark C. Miller, Tue Oct 31 20:33:29 PST 2006
//    Added VALID_XVALUE token to support "zone-centered" curves
//
//    Kathleen Bonnell, Tue Jan 20 11:02:57 PST 2009
//    Added spatialExtents. 
//
//    Kathleen Biagas, Tue Jul 15 14:16:07 MST 2014
//    Change 'GetPoint' args from float to double.
//
//    Kathleen Biagas, Fri Aug 31 14:22:11 PDT 2018
//    Added DBOptionsAttributes (currently unused).
//
//    Olivier Cessenat, Thu Oct  5 18:52:48 CEST 2023
//    Go 3D and make a global mesh with materials out of the figures.
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

    std::vector<vtkUnstructuredGrid *> curves;
    std::vector<std::string>   curveNames;
    std::vector<double>        zExtents;
    std::vector<double>        yExtents;
    std::vector<double>        xExtents;
    double                     curveTime;
    int                        curveCycle;
    double range[6] ; // geometrical mesh range

    void                  ReadFile(void);
    Curve3DToken          GetPoint(istream &, double &, double &, double &,
                                   std::string &);
};


#endif


