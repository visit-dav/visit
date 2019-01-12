// ************************************************************************* //
//                            avtFVCOMReader.h                               //
// ************************************************************************* //

#ifndef AVT_FVCOM_READER_H
#define AVT_FVCOM_READER_H
#include <vectortypes.h>
#include <avtVariableCache.h>
#include <netcdf.h>
#include <map>
#include <DebugStream.h>

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
    void               SetKeySuffixForCaching(const char *filename) {keysuffix = filename; }


    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    void           GetCycles(std::vector<int> &);
    void           GetTimes(std::vector<double> &);
    int            GetNTimesteps(void);

    void           FreeUpResources();
    void           MTMDFreeUpResources();
    vtkDataSet    *GetMesh(int, const char *, avtVariableCache *);
    vtkDataArray  *GetVar(int, const char *, avtVariableCache *);
    vtkDataArray  *GetVectorVar(int, const char *, avtVariableCache *);
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
    std::string keysuffix;

  private:

     // Pass timestate to these methods!
    vtkDataArray          *DENS3(int, avtVariableCache *);
    //    vtkDataArray  *DENS2(int, avtVariableCache *);
    vtkDataArray          *DENS(int, avtVariableCache *);
    vtkDataArray          *THETA(int, avtVariableCache *);
    // Pass (S,T,P) to these methods!
    double                ATG(double, double, double);
    double                SVAN(double, double, double);
    double                Dens3helper(double,double,double);
    double                Thetahelper(double,double,double);

    virtual void          InitializeReader(void);

    virtual void          GetDimensions(void);
    bool NeedDimensions;
    virtual void          GetStaticGridVariables(void);
    bool NeedGridVariables;  
    void                  SphereVel2Cart(float *,int);
    void                  Sphere2Cart(float *);
    bool IsGeoRef;

    int status, ncid, nDims, nVars, nGlobalAtts, unlimitedDimension;
    int nScalarID, nNodeID, nElemID, nSiglayID, nSiglevID, 
      nThreeID, nFourID, nMaxnodeID, nMaxelemID, nTimeID, dimID;
    int  nScalar, nNode, nElem, nSiglay, nSiglev, 
      nThree, nFour, nMaxnode, nMaxelem, nTime;
    int VarnDims, VarnAtts, VarDimIDs[NC_MAX_VAR_DIMS], VarID;


    bool xstate,ystate, hstate, zstate, latstate, lonstate;
    bool nodestate, elemstate, siglaystate, siglevstate; 
    bool mesh1, mesh2, mesh3, mesh4, mesh5;

    char   DimName[NC_MAX_NAME+1],VarName[NC_MAX_NAME+1];            
    std::string SigLayCoordType,SigLevCoordType;

    float *xvals, *yvals, *zvals, *SigLayers, *SigLevels, *latvals, *lonvals;
    int *nvvals, *egid, *ngid, *dimSizes;

};


#endif
