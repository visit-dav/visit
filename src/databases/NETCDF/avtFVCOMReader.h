// ************************************************************************* //
//                            avtFVCOMReader.h                               //
// ************************************************************************* //

#ifndef AVT_FVCOM_READER_H
#define AVT_FVCOM_READER_H
#include <vectortypes.h>

class vtkDataArray;
class vtkDataSet;

class avtDatabaseMetaData;
class avtFileFormatInterface;

class NETCDFFileObject;

// ****************************************************************************
//  Class: avtFVCOMReader
//
//  Purpose:
//      Reads in FVCOM files as a plugin to VisIt.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

class avtFVCOMReader
{
  public:
   static bool        Identify(NETCDFFileObject *); 
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtFVCOMReader(const char *, NETCDFFileObject *);
                       avtFVCOMReader(const char *);
    virtual           ~avtFVCOMReader();

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    void           GetCycles(std::vector<int> &);
    void           GetTimes(std::vector<double> &);
    int            GetNTimesteps(void);

    void           FreeUpResources();
    vtkDataSet    *GetMesh(int, const char *);
    vtkDataArray  *GetVar(int, const char *);
    vtkDataArray  *GetVectorVar(int, const char *);
    void           PopulateDatabaseMetaData(avtDatabaseMetaData *,
                                            const int ts, const char *dbtype);

  protected:
    // DATA MEMBERS
    NETCDFFileObject      *fileObject;
};


#endif
