// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtShapefileFileFormat.h                       //
// ************************************************************************* //

#ifndef AVT_Shapefile_FILE_FORMAT_H
#define AVT_Shapefile_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <esriShapefile.h>
#include <dbfFile.h>
#include <vector>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtShapefileFileFormat
//
//  Purpose:
//      Reads in ESRI Shapefile files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 24 12:18:02 PDT 2005
//
//  Modifications:
//    Brad Whitlock, Fri Apr 1 23:41:51 PST 2005
//    Added GetNumRepeats.
//
//    Brad Whitlock, Tue Feb 27 11:45:59 PDT 2007
//    Added CountShapeTypes.
//
//    Brad Whitlock, Wed Mar 7 11:42:07 PDT 2007
//    Added support for tessellation.
//
// ****************************************************************************

class avtShapefileFileFormat : public avtSTSDFileFormat
{
public:
                       avtShapefileFileFormat(const char *filename,
                           const DBOptionsAttributes *rdopts);
    virtual           ~avtShapefileFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      &GetAuxiliaryData(const char *var, const char *type,
    //                                  void *args, DestructorFunction &);
    //

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    // virtual int       GetCycle(void);
    // virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "ESRI Shapefile"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    struct esriShape
    {
        esriShapeType_t  shapeType;
        void            *shape;
        int              nRepeats;
    };

    typedef std::vector<esriShape> esriShapeVector;

    void                   Initialize();

    int                    CountMemberPoints(esriShapeType_t) const;
    int                    CountShapes(esriShapeType_t) const;
    int                    CountCellsForShape(esriShapeType_t) const;
    int                    GetNumRepeats(const esriShape &shape, bool) const;
    int                    CountShapeTypes() const;
    vtkDataSet            *GetMesh_TessellatedPolygon();

    bool                   initialized;
    esriShapeVector        shapes;
    int                    numShapeTypes;
    dbfFile_t             *dbfFile;

    // Options.
    bool                   polygonsAsLines;
    bool                   tessellatePolygons;
    bool                   esriLogging;
    bool                   dbfLogging;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
