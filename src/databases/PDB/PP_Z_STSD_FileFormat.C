#include <PP_Z_STSD_FileFormat.h>
#include <avtSTSDFileFormatInterface.h>
#include <VisItException.h>

avtFileFormatInterface *
PP_Z_STSD_FileFormat::CreateInterface(PDBFileObject *pdb,
    const char *const *filenames, int nList, int nBlock)
{
    avtFileFormatInterface *inter = 0;

    // Create a PF3D file that uses the pdb file but does not own it.
    PP_Z_STSD_FileFormat *ff = new PP_Z_STSD_FileFormat(pdb);

    // If the file format is a PF3D file then
    if(ff->Identify())
    {
        //
        // Create an array of STMD file formats since that's what the PF3D
        // file format is.
        //
        int i, j;
        avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[nList];
        for (i = 0 ; i < nList ; i++)
            ffl[i] = 0;
        int nTimestep = nList / nBlock;
        TRY
        {
            for (i = 0 ; i < nTimestep ; i++)
            {
                ffl[i] = new avtSTSDFileFormat*[nBlock];
                for (j = 0 ; j < nBlock ; j++)
                    ffl[i][j] = 0;
                for (j = 0 ; j < nBlock ; j++)
                {
                    if(i == 0 && j == 0)
                        ffl[i][j] = ff;
                    else
                        ffl[i][j] = new PP_Z_STSD_FileFormat(filenames[i*nBlock + j]);
                }
            }

            //
            // Try to create a file format interface compatible with the PF3D
            // file format.
            //
            inter = new avtSTSDFileFormatInterface(ffl, nTimestep, nBlock);
        }
        CATCH(VisItException)
        {
            for (i = 0 ; i < nTimestep ; i++)
            {
                if(ffl[i])
                {
                    for (j = 0 ; j < nBlock ; j++)
                         delete ffl[i][j];
                    delete [] ffl[i];
                }
            }
            delete [] ffl;
            RETHROW;
        }
        ENDTRY

        //
        // Since at this point, we successfully created a file format interface, we
        // can let the first file format keep the PDB file object.
        //
        ff->SetOwnsPDBFile(true);
    }
    else
        delete ff;

    return inter;
}

PP_Z_STSD_FileFormat::PP_Z_STSD_FileFormat(const char *filename) : 
    avtSTSDFileFormat(filename), reader(filename)
{
}

PP_Z_STSD_FileFormat::PP_Z_STSD_FileFormat(PDBFileObject *p) :
    avtSTSDFileFormat(p->GetName().c_str()), reader(p)
{
}

PP_Z_STSD_FileFormat::~PP_Z_STSD_FileFormat()
{
}

//
// Mimic PDBReader interface.
//
bool
PP_Z_STSD_FileFormat::Identify()
{
    return (reader.Identify() && reader.GetNumTimeSteps() == 1);
}

void
PP_Z_STSD_FileFormat::SetOwnsPDBFile(bool val)
{
    reader.SetOwnsPDBFile(val);
}

//
// Methods overrides for an STSD file format.
//

const char *
PP_Z_STSD_FileFormat::GetType()
{
    return "PPZ STSD File Format";
}

void
PP_Z_STSD_FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader.PopulateDatabaseMetaData(md);
}

int
PP_Z_STSD_FileFormat::GetCycle(void)
{
    const int *c = reader.GetCycles();
    return (c != 0) ? c[0] : 0;
}

vtkDataSet *
PP_Z_STSD_FileFormat::GetMesh(const char *var)
{
    return reader.GetMesh(0, var);
}

vtkDataArray *
PP_Z_STSD_FileFormat::GetVar(const char *var)
{
    return reader.GetVar(0, var);
}

void *
PP_Z_STSD_FileFormat::GetAuxiliaryData(const char *var, const char *type,
    void *args, DestructorFunction &df)
{
     return reader.GetAuxiliaryData(0, var, type, args, df);
}
