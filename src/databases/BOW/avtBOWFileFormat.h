// ************************************************************************* //
//                            avtBOWFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_WPP_FILE_FORMAT_H
#define AVT_WPP_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vectortypes.h>


// ****************************************************************************
//  Class: avtBOWFileFormat
//
//  Purpose:
//      Reads in BOW (brick of wavelet) files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Mar 30 11:27:47 PDT 2006
//
// ****************************************************************************

class avtBOWFileFormat : public avtMTMDFileFormat
{
public:
                       avtBOWFileFormat(const char *);
    virtual           ~avtBOWFileFormat();

    virtual void           ActivateTimestep(int ts);

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);
    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "BOW files"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

protected:
    class HeaderData
    {
    public:
        HeaderData();
        ~HeaderData();
        int Read(const char *filename);

        int nx, ny, nz;
        int dx, dy, dz;
        int topdirmin;
        int topdirmod;
        int numblocks;
        double q;
        char *filenamepat;
        char varname[100];
        bool delogify;

        int   timeMethod;
        char  *m1_file;
        int    m2_nStates;
        int    m2_startCycle;
        int    m2_deltaCycle;
        double m2_startTime;
        double m2_deltaTime;
    };

    bool         headerRead;
    HeaderData   header;
    bool         cyclesAndTimesDetermined;
    intVector    cycles;
    doubleVector times;
    bool         domainConnectivtyComputed;
    int         *domainConnectivity;

    bool DetermineCyclesAndTimes();
    void ReadDomainConnectivity();

    void GetFilenameForDomain(char *filename, int len, int dom, int ts);
    char *GetBOWBytesForDomainAtTime(int dom, int ts, bool infoOnly);
    bool ReadSingleBOFDimensions(int dom, int ts, int *dimsize);


    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
