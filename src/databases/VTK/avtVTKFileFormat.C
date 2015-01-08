/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <avtVTKFileFormat.h>

avtVTK_STSDFileFormat::avtVTK_STSDFileFormat(const char *filename, DBOptionsAttributes *opts) : 
    avtSTSDFileFormat(filename)
{
    reader = new avtVTKFileReader(filename, opts);
}

avtVTK_STSDFileFormat::avtVTK_STSDFileFormat(const char *filename, 
    DBOptionsAttributes *opts, avtVTKFileReader *r) : 
    avtSTSDFileFormat(filename)
{
    reader = r;
}

avtVTK_STSDFileFormat::~avtVTK_STSDFileFormat()
{
    delete reader;
}

const char *
avtVTK_STSDFileFormat::GetType(void)
{
    return "VTK File Format";
}

void
avtVTK_STSDFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

int
avtVTK_STSDFileFormat::GetCycle(void)
{
    return reader->GetCycle();
}

double
avtVTK_STSDFileFormat::GetTime(void)
{
    return reader->GetTime();
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetCycleFromFilename
//
//  Purpose: Try to get a cycle number from a file name
//
//  Notes: Although all this method does is simply call the format's base
//         class implementation of GuessCycle, doing this is a way for the
//         VTK format to "bless" the guesses that that method makes.
//         Otherwise, VisIt wouldn't know that VTK thinks those guesses are
//         good. See notes in avtSTXXFileFormatInterface::SetDatabaseMetaData
//         for further explanation.
//
//  Programmer: Eric Brugger
//  Creation:   August 12, 2005
//
// ****************************************************************************

int
avtVTK_STSDFileFormat::GetCycleFromFilename(const char *f) const
{
    return GuessCycle(f);
}

void
avtVTK_STSDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtVTK_STSDFileFormat::GetMesh(const char *name)
{
    return reader->GetMesh(0, name);
}

vtkDataArray *
avtVTK_STSDFileFormat::GetVar(const char *name)
{
    return reader->GetVar(0, name);
}

vtkDataArray *
avtVTK_STSDFileFormat::GetVectorVar(const char *name)
{
    return reader->GetVectorVar(0, name);
}

void *
avtVTK_STSDFileFormat::GetAuxiliaryData(const char *var, 
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, 0, type, d, df);
}

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

avtVTK_STMDFileFormat::avtVTK_STMDFileFormat(const char *filename, DBOptionsAttributes *opts) : 
    avtSTMDFileFormat(&filename, 1)
{
    reader = new avtVTKFileReader(filename, opts);
}

avtVTK_STMDFileFormat::avtVTK_STMDFileFormat(const char *filename,
    DBOptionsAttributes *opts, avtVTKFileReader *r) : 
    avtSTMDFileFormat(&filename, 1)
{
    reader = r;
}

avtVTK_STMDFileFormat::~avtVTK_STMDFileFormat()
{
    delete reader;
}

const char *
avtVTK_STMDFileFormat::GetType(void)
{
    return "VTK MD File Format";
}

void
avtVTK_STMDFileFormat::FreeUpResources(void)
{
    reader->FreeUpResources();
}

int
avtVTK_STMDFileFormat::GetCycle(void)
{
    return reader->GetCycle();
}

double
avtVTK_STMDFileFormat::GetTime(void)
{
    return reader->GetTime();
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetCycleFromFilename
//
//  Purpose: Try to get a cycle number from a file name
//
//  Notes: Although all this method does is simply call the format's base
//         class implementation of GuessCycle, doing this is a way for the
//         VTK format to "bless" the guesses that that method makes.
//         Otherwise, VisIt wouldn't know that VTK thinks those guesses are
//         good. See notes in avtSTXXFileFormatInterface::SetDatabaseMetaData
//         for further explanation.
//
//  Programmer: Eric Brugger
//  Creation:   August 12, 2005
//
// ****************************************************************************

int
avtVTK_STMDFileFormat::GetCycleFromFilename(const char *f) const
{
    return GuessCycle(f);
}

void
avtVTK_STMDFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    reader->PopulateDatabaseMetaData(md);
}

vtkDataSet *
avtVTK_STMDFileFormat::GetMesh(int domain, const char *name)
{
    return reader->GetMesh(domain, name);
}

vtkDataArray *
avtVTK_STMDFileFormat::GetVar(int domain, const char *name)
{
    return reader->GetVar(domain, name);
}

vtkDataArray *
avtVTK_STMDFileFormat::GetVectorVar(int domain, const char *name)
{
    return reader->GetVectorVar(domain, name);
}

void *
avtVTK_STMDFileFormat::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *d, DestructorFunction &df)
{
    return reader->GetAuxiliaryData(var, domain, type, d, df);
}
