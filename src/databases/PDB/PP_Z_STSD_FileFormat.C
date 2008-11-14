/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <PP_Z_STSD_FileFormat.h>
#include <avtSTSDFileFormatInterface.h>
#include <VisItException.h>
#include <DebugStream.h>

// Turn on streak plot support
#define PDB_STREAK_PLOTS

#ifdef PDB_STREAK_PLOTS
#include <Streaker.h>
#endif

// ****************************************************************************
// Class: AlwaysReadCyclesAndTimes_STSD_FFI
//
// Purpose:
//   A STSD file format interface that always reads cycles and times, 
//   regardless of how the "try harder to read cycles and times" flag is set.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  1 10:47:47 PDT 2008
//
// Modifications:
//   Brad Whitlock, Fri Nov  7 10:22:16 PST 2008
//   Added support for streak plots.
//
// ****************************************************************************

class AlwaysReadCyclesAndTimes_STSD_FFI : public avtSTSDFileFormatInterface
{
public:
    AlwaysReadCyclesAndTimes_STSD_FFI(avtSTSDFileFormat ***a, int b, int c) :
        avtSTSDFileFormatInterface(a, b, c)
    {
    }

    virtual ~AlwaysReadCyclesAndTimes_STSD_FFI() { }

    virtual void SetDatabaseMetaData(avtDatabaseMetaData *md, 
                                     int timeState = 0, bool = false)
    {
        // Always read cycles and times
        avtSTSDFileFormatInterface::SetDatabaseMetaData(md, timeState, true);
#ifndef PDB_STREAK_PLOTS
    }
#else
        // Optionally tack on streak plot data.
        Streaker::PDBFileObjectVector pdbs;
        for(int b = 0; b < nBlocks; ++b)
            for(int t = 0; t < nTimesteps; ++t)
            {
                PP_Z_STSD_FileFormat *ff = (PP_Z_STSD_FileFormat *)timesteps[t][b];
                pdbs.push_back(ff->PDB());
            }
        streaker.ReadStreakFile(std::string(timesteps[0][0]->GetFilename()) + ".streak", pdbs[0]);
        streaker.PopulateDatabaseMetaData(md);
    }

    // Override to provide streak plot support.
    virtual vtkDataSet *
    GetMesh(int ts, int dom, const char *mesh)
    {
        Streaker::PDBFileObjectVector pdbs;
        for(int b = 0; b < nBlocks; ++b)
            for(int t = 0; t < nTimesteps; ++t)
            {
                PP_Z_STSD_FileFormat *ff = (PP_Z_STSD_FileFormat *)timesteps[t][b];
                pdbs.push_back(ff->PDB());
            }
        vtkDataSet *retval = streaker.GetMesh(mesh, pdbs);

        if(retval == 0)
            retval = avtSTSDFileFormatInterface::GetMesh(ts, dom, mesh);
        return retval;
    }

    virtual vtkDataArray *
    GetVar(int ts, int dom, const char *var)
    {
        Streaker::PDBFileObjectVector pdbs;
        for(int b = 0; b < nBlocks; ++b)
            for(int t = 0; t < nTimesteps; ++t)
            {
                PP_Z_STSD_FileFormat *ff = (PP_Z_STSD_FileFormat *)timesteps[t][b];
                pdbs.push_back(ff->PDB());
            }
        vtkDataArray *retval = streaker.GetVar(var, pdbs);

        if(retval == 0)
            retval = avtSTSDFileFormatInterface::GetVar(ts, dom, var);
        return retval;
    }

    Streaker streaker;
#endif
};

// ****************************************************************************
// Method: PP_Z_STSD_FileFormat::CreateInterface
//
// Purpose: 
//   Creates a file format interface that works for PP_Z_STSD_FileFormat files.
//
// Arguments:
//   pdb       : The PDB file object that we're using to determine the file
//               type.
//   filenames : The list of files that make up the database.
//   nList     : The number of files in the database.
//   nBlock    : The number of block files per time step.
//
// Returns:    A file format interface object that lets us access the files
//             in the filename list.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 15:03:57 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug  1 10:48:39 PDT 2008
//   Made it use AlwaysReadCyclesAndTimes_STSD_FFI.
//
// ****************************************************************************

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
            inter = new AlwaysReadCyclesAndTimes_STSD_FFI(ffl, nTimestep, nBlock);
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

// ****************************************************************************
// Method: PP_Z_STSD_FileFormat::PP_Z_STSD_FileFormat
//
// Purpose: 
//   Constructor for the PP_Z_STSD_FileFormat class.
//
// Arguments:
//   filename : The name of the file for which we'll access data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 15:06:34 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PP_Z_STSD_FileFormat::PP_Z_STSD_FileFormat(const char *filename) : 
    avtSTSDFileFormat(filename), reader(filename)
{
}

// ****************************************************************************
// Method: PP_Z_STSD_FileFormat::PP_Z_STSD_FileFormat
//
// Purpose: 
//   Constructor for the PP_Z_STSD_FileFormat class.
//
// Arguments:
//   p : The PDB file object that we'll use to access data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 15:06:34 PST 2003
//
// Modifications:
//   
// ****************************************************************************

PP_Z_STSD_FileFormat::PP_Z_STSD_FileFormat(PDBFileObject *p) :
    avtSTSDFileFormat(p->GetName().c_str()), reader(p)
{
}

// ****************************************************************************
// Method: PP_Z_STSD_FileFormat::~PP_Z_STSD_FileFormat
//
// Purpose: 
//   Destructor for the PP_Z_STSD_FileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 15:07:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

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

PDBFileObject *
PP_Z_STSD_FileFormat::PDB()
{
    return reader.PDB();
}

//
// Methods overrides for an STSD file format.
//

const char *
PP_Z_STSD_FileFormat::GetType()
{
    return "Z File Format";
}

void
PP_Z_STSD_FileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader.PopulateDatabaseMetaData(timestep, md);
}

int
PP_Z_STSD_FileFormat::GetCycle(void)
{
    const int *c = reader.GetCycles();
    return (c != 0) ? c[0] : 0;
}

double
PP_Z_STSD_FileFormat::GetTime(void)
{
    const double *t = reader.GetTimes();
    return (t != 0) ? t[0] : 0;
}

vtkDataSet *
PP_Z_STSD_FileFormat::GetMesh(const char *var)
{
    reader.SetCache(cache);
    return reader.GetMesh(0, var);
}

vtkDataArray *
PP_Z_STSD_FileFormat::GetVar(const char *var)
{
    reader.SetCache(cache);
    return reader.GetVar(0, var);
}

void *
PP_Z_STSD_FileFormat::GetAuxiliaryData(const char *var, const char *type,
    void *args, DestructorFunction &df)
{
    reader.SetCache(cache);
    return reader.GetAuxiliaryData(0, var, type, args, df);
}

// ****************************************************************************
// Method: PP_Z_STSD_FileFormat::FreeUpResources
//
// Purpose: 
//   Frees the data and file descriptor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 2 00:14:28 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
PP_Z_STSD_FileFormat::FreeUpResources()
{
    debug4 << "PP_Z_STSD_FileFormat::FreeUpResources" << endl;
    reader.FreeUpResources();
}
