// ************************************************************************* //
//                            avtFVCOMParticleFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_FVCOM_FILE_FORMAT_H
#define AVT_FVCOM_FILE_FORMAT_H
#include <avtMTSDFileFormat.h>
#include <avtMTSDFileFormatInterface.h>


#include <vectortypes.h>

class NETCDFFileObject;
class avtFileFormatInterface;

// ****************************************************************************
//  Class: avtFVCOMParticleFileFormat
//
//  Purpose:
//      Reads in FVCOM files as a plugin to VisIt.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

class avtFVCOMParticleFileFormat : public avtMTSDFileFormat
{
  public:
   static bool        Identify(NETCDFFileObject *); 
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtFVCOMParticleFileFormat(const char *, NETCDFFileObject *);
                       avtFVCOMParticleFileFormat(const char *);
    virtual           ~avtFVCOMParticleFileFormat();

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void        GetCycles(std::vector<int> &);
    virtual void        GetTimes(std::vector<double> &);
    //

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "FVCOM"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    // DATA MEMBERS
    NETCDFFileObject      *fileObject;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
