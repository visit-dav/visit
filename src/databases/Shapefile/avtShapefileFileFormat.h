
// ************************************************************************* //
//                            avtShapefileFileFormat.h                       //
// ************************************************************************* //

#ifndef AVT_Shapefile_FILE_FORMAT_H
#define AVT_Shapefile_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <esriShapefile.h>
#include <dbfFile.h>
#include <vector>

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
// ****************************************************************************

class avtShapefileFileFormat : public avtSTSDFileFormat
{
public:
                       avtShapefileFileFormat(const char *filename);
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
    // virtual bool      ReturnsValidCycle() const { return true; };
    // virtual int       GetCycle(void);
    // virtual bool      ReturnsValidTime() const { return true; };
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
    };

    typedef std::vector<esriShape> esriShapeVector;

    void                   Initialize();

    int                    CountMemberPoints(esriShapeType_t) const;
    int                    CountShapes(esriShapeType_t) const;
    int                    CountCellsForShape(esriShapeType_t) const;
    int                    GetNumRepeats(void *shape, esriShapeType_t shapeType) const;

    bool                   initialized;
    esriShapeVector        shapes;
    int                    numShapeTypes;
    dbfFile_t              *dbfFile;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
