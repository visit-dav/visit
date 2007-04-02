// ************************************************************************* //
//                              avtBOVFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_BOV_FILE_FORMAT_H
#define AVT_BOV_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>


// ****************************************************************************
//  Class: avtBOVFileFormat
//
//  Purpose:
//      Reads "brick of float" files.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Mar  4 16:02:03 PST 2005
//    Added support for deferring reading in all of the TOC.
//
//    Brad Whitlock, Thu Mar 16 14:54:01 PST 2006
//    Added byte offset so we can skip past a header. I also added a new
//    flag for dividing a brick into bricklets.
//
// ****************************************************************************

class avtBOVFileFormat : public avtSTMDFileFormat
{
  public:
                               avtBOVFileFormat(const char *);
    virtual                   ~avtBOVFileFormat();

    virtual const char        *GetType(void) { return "Brick of floats"; };

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    virtual void              *GetAuxiliaryData(const char *var, int,
                                                const char *type, void *args,
                                                DestructorFunction &);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual int                GetCycle(void) { return cycle; };
    virtual bool               ReturnsValidCycle(void) { return haveReadTOC; };
    void                       ActivateTimestep(void);

  protected:
    void                       ReadTOC(void);

    bool                       haveReadTOC;
    char                      *path;
    char                      *file_pattern;
    int                        cycle;
    int                        full_size[3];
    int                        bricklet_size[3];
    int                        byteOffset;
    char                      *varname;
    bool                       hasBoundaries;
    bool                       declaredEndianess;
    bool                       littleEndian;
    bool                       nodalCentering;
    bool                       byteData;
    bool                       divideBrick;
    float                      min, max;
    float                      origin[3];
    float                      dimensions[3];
    float                     *var_brick_min;
    float                     *var_brick_max;
};


#endif


