#ifndef AVT_LODI_PARTICLE_FILE_FORMAT_H
#define AVT_LODI_PARTICLE_FILE_FORMAT_H
#include <avtMTSDFileFormat.h>
#include <vectortypes.h>

class NETCDFFileObject;
class avtFileFormatInterface;

// ****************************************************************************
// Class: avtLODIParticleFileFormat
//
// Purpose:
//   Reads LODI particle data from a NETCDF file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 09:51:09 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class avtLODIParticleFileFormat : public avtMTSDFileFormat
{
public:
    static bool        Identify(NETCDFFileObject *);
    static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f,
                       const char *const *list, int nList, int nBlock);

                       avtLODIParticleFileFormat(const char *filename,
                                                 NETCDFFileObject *);
                       avtLODIParticleFileFormat(const char *filename);
    virtual           ~avtLODIParticleFileFormat();

    virtual const char    *GetType(void) { return "LODI Particle"; }
    virtual void           ActivateTimestep(int ts);
    virtual void           FreeUpResources(void);

    virtual int            GetNTimesteps(void);
    virtual void           GetTimes(std::vector<double> &);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual void          *GetAuxiliaryData(const char *var,
                                            int timeState,
                                            const char *type,
                                            void *args,
                                            DestructorFunction &);
  private:
    bool                   ReadTimes();
    bool                   GetFillValue(float &fill_value);
    bool                  *CreateParticleMask(int ts);
    bool                  *GetParticleMask(int ts);
    static void            ParticleMaskDestruct(void *ptr);

    NETCDFFileObject      *fileObject;
    doubleVector           times;
    bool                   timesRead;
    stringVector           sourceids;
};

#endif
