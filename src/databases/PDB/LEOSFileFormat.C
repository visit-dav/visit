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

#include <LEOSFileFormat.h>
#include <avtSTSDFileFormatInterface.h>
#include <VisItException.h>

// ****************************************************************************
// Method: LEOSFileFormat::CreateInterface
//
// Purpose: 
//   Creates a file format interface that works for LEOSFileFormat files.
//
// Arguments:
//   pdb       : The PDB file object that we're using to determine the file
//               type.
//   filename  : the name of the file 
//
// Returns:    A file format interface object that lets us access the files
//             in the filename list.
//
// Programmer: Mark C. Miller 
// Creation:   February 10, 2004 
//
// ****************************************************************************

avtFileFormatInterface *
LEOSFileFormat::CreateInterface(PDBFileObject *pdb,
    const char *filename)
{
    avtFileFormatInterface *inter = 0;

    // Create a PF3D file that uses the pdb file but does not own it.
    LEOSFileFormat *ff = new LEOSFileFormat(pdb);

    // If the file format is an LEOS file then
    if(ff->Identify())
    {
        avtSTSDFileFormat ***ffl = new avtSTSDFileFormat**[1];
        ffl[0] = new avtSTSDFileFormat*[1];
        ffl[0][0] = new LEOSFileFormat(filename);

        //
        // Try to create a file format interface compatible with the LEOS 
        // file format.
        //
        inter = new avtSTSDFileFormatInterface(ffl, 1, 1);

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
// Method: LEOSFileFormat::LEOSFileFormat
//
// Purpose: 
//   Constructor for the LEOSFileFormat class.
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

LEOSFileFormat::LEOSFileFormat(const char *filename) : 
    avtSTSDFileFormat(filename), reader(filename)
{
}

// ****************************************************************************
// Method: LEOSFileFormat::LEOSFileFormat
//
// Purpose: 
//   Constructor for the LEOSFileFormat class.
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

LEOSFileFormat::LEOSFileFormat(PDBFileObject *p) :
    avtSTSDFileFormat(p->GetName().c_str()), reader(p)
{
}

// ****************************************************************************
// Method: LEOSFileFormat::~LEOSFileFormat
//
// Purpose: 
//   Destructor for the LEOSFileFormat class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 15:07:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

LEOSFileFormat::~LEOSFileFormat()
{
}

//
// Mimic PDBReader interface.
//
bool
LEOSFileFormat::Identify()
{
    return reader.Identify();
}

void
LEOSFileFormat::SetOwnsPDBFile(bool val)
{
    reader.SetOwnsPDBFile(val);
}

//
// Methods overrides for an STSD file format.
//

const char *
LEOSFileFormat::GetType()
{
    return "LEOS File Format";
}

void
LEOSFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader.PopulateDatabaseMetaData(md);
}

int
LEOSFileFormat::GetCycle(void)
{
    return 0;
}

double
LEOSFileFormat::GetTime(void)
{
    return 0;
}

vtkDataSet *
LEOSFileFormat::GetMesh(const char *var)
{
    return reader.GetMesh(0, var);
}

vtkDataArray *
LEOSFileFormat::GetVar(const char *var)
{
    return reader.GetVar(0, var);
}
