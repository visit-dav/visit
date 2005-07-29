
// ************************************************************************* //
//                            avtPATRANFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_PATRAN_FILE_FORMAT_H
#define AVT_PATRAN_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <vectortypes.h>

// ****************************************************************************
//  Class: avtPATRANFileFormat
//
//  Purpose:
//      Reads in PATRAN files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 28 13:51:07 PST 2005
//
//  Modifications:
//
// ****************************************************************************

class avtPATRANFileFormat : public avtSTSDFileFormat
{
public:
                       avtPATRANFileFormat(const char *filename);
    virtual           ~avtPATRANFileFormat();

    virtual void      *GetAuxiliaryData(const char *var, const char *type,
                                        void *args, DestructorFunction &);

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

    virtual const char    *GetType(void)   { return "PATRAN Neutral"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    bool ReadFile(const char *, int nLines);

    // DATA MEMBERS
    vtkDataSet            *meshDS;
    vtkDataArray          *elementMats;
    vtkDataArray          *elementIds;
    std::string            title;

    int                   *componentList;
    int                    componentListSize;
    stringVector           componentNames;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
