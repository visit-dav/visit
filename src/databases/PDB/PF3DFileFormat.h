#ifndef PF3DFILEFORMAT_H
#define PF3DFILEFORMAT_H
#include <PDBReader.h>
#include <avtSTSDFileFormat.h>

class avtFileFormatInterface;
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
// Class: PF3DFileFormat
//
// Purpose:
//   Reads the PF3D file format.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 10:28:16 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class PF3DFileFormat : public PDBReader, public avtSTSDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *const *filenames, int nList, int nBlocks);

    PF3DFileFormat(const char *filename);
    PF3DFileFormat(PDBFileObject *p);
    virtual ~PF3DFileFormat();

    // Methods overrides for an STSD file format.
    virtual const char   *GetType();
    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual int           GetCycle(void);
    virtual vtkDataSet   *GetMesh(const char *var);
    virtual vtkDataArray *GetVar(const char *var);
protected:
    virtual bool IdentifyFormat();

private:
    void GetMeshInfo(const char *meshName, float extents[6], int nNodes[3],
                     bool &ghostPresent, int first[3], int last[3]);

    // Values that are read in when the file is opened.
    int    cycle;
    double dtime;
    double dx, dy, dz;       // Cell size
    double lx, ly, lz;       // Dimensions used in calculating extents.
    int    nx, ny, nz;       // Number of cells in each dimension for entire problem.
    int    nxg0, nyg0, nzg0; // Starting cell index for domain.
    int    nxg1, nyg1, nzg1; // End cell index for domain.
    int    nxl, nyl, nzl;    // Number of cells in each dimension for domain.
    int    ipol;             // Determines if certain variables exist on light mesh.
    int    domain;           // The domain index.
    int    nDomains;         // Number of domains for entire problem.

    int    kludgeOffset;     // Hack
};

#endif
