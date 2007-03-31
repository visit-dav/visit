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

  protected:
    void                       ReadTOC(void);

    char                      *path;
    char                      *file_pattern;
    int                        cycle;
    int                        full_size[3];
    int                        bricklet_size[3];
    char                      *varname;
    bool                       hasBoundaries;
    bool                       littleEndian;
    float                      min, max;
    float                      origin[3];
    float                      dimensions[3];
    float                     *var_brick_min;
    float                     *var_brick_max;
};


#endif


