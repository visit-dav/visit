// ************************************************************************* //
//                             avtMTMDFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MTMD_FILE_FORMAT_H
#define AVT_MTMD_FILE_FORMAT_H

#include <database_exports.h>

#include <avtFileFormat.h>

#include <vector>

#include <void_ref_ptr.h>


class     vtkDataSet;
class     vtkDataArray;


// ****************************************************************************
//  Class: avtMTMDFileFormat
//
//  Purpose:
//      This is an implementation of avtFileFormat for file formats that have
//      a Multiple Timesteps and Multiple Domains.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
// ****************************************************************************

class DATABASE_API avtMTMDFileFormat : public avtFileFormat
{
  public:
                           avtMTMDFileFormat(const char *);
    virtual               ~avtMTMDFileFormat();

    virtual void          *GetAuxiliaryData(const char *var, int, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    virtual void           GetCycles(std::vector<int> &);
    virtual int            GetNTimesteps(void);

    virtual const char    *GetFilename(void) { return filename; };

    virtual vtkDataSet    *GetMesh(int, int, const char *) = 0;
    virtual vtkDataArray  *GetVar(int, int, const char *) = 0;
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    char                  *filename;
};


#endif


