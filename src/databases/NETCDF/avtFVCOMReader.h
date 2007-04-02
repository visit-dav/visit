// ************************************************************************* //
//                            avtFVCOMReader.h                               //
// ************************************************************************* //

#ifndef AVT_FVCOM_READER_H
#define AVT_FVCOM_READER_H
#include <vectortypes.h>
#include <avtVariableCache.h>

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

    void               SetDomainIndexForCaching(int dom) {CacheDomainIndex = dom; }


    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    void           GetCycles(std::vector<int> &);
    void           GetTimes(std::vector<double> &);
    int            GetNTimesteps(void);

    void           FreeUpResources();
    vtkDataSet    *GetMesh(int, const char *, avtVariableCache *);
    vtkDataArray  *GetVar(int, const char *, avtVariableCache *);
    vtkDataArray  *GetVectorVar(int, const char *);
    void           PopulateDatabaseMetaData(avtDatabaseMetaData *,
                                            const int ts, const char *dbtype);

    virtual void          *GetAuxiliaryData(const char *var,
                                            int timeState,
                                            const char *type,
                                            void *args,
                                            DestructorFunction &);


  protected:
    // DATA MEMBERS
    NETCDFFileObject      *fileObject;

    int  CacheDomainIndex;

  private:
    // Pass timestate to these methods!
    vtkDataArray  *DENS3(int, avtVariableCache *);
    //    vtkDataArray  *DENS2(int, avtVariableCache *);
    vtkDataArray  *DENS(int, avtVariableCache *);
    vtkDataArray  *THETA(int, avtVariableCache *);
    // Pass (S,T,P) to these methods!
    double        ATG(double, double, double);
    double        SVAN(double, double, double);


};


#endif
