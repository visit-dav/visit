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

#ifndef PP_Z_STSD_FILE_FORMAT_H
#define PP_Z_STSD_FILE_FORMAT_H
#include <avtSTSDFileFormat.h>
#include <PP_ZFileReader.h>

class avtFileFormatInterface;

// ****************************************************************************
// Class: PP_Z_STSD_FileFormat
//
// Purpose:
//   This file format uses
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 15:27:25 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Oct 13 14:58:12 PST 2003
//   Added methods that indicate that the cycle and time that the file format
//   returns can be believed.
//
//   Brad Whitlock, Thu Sep 2 00:12:33 PDT 2004
//   Added override for FreeUpResources.
//
// ****************************************************************************

class PP_Z_STSD_FileFormat : public avtSTSDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *const *filenames, int nList, int nBlocks);

    PP_Z_STSD_FileFormat(const char *filename);
    PP_Z_STSD_FileFormat(PDBFileObject *p);
    virtual ~PP_Z_STSD_FileFormat();

    // Mimic PDBReader interface.
    bool Identify();
    void SetOwnsPDBFile(bool);

    // Methods overrides for an STSD file format.
    virtual const char   *GetType();
    virtual bool          ReturnsValidCycle() const { return true; }
    virtual int           GetCycle(void);
    virtual bool          ReturnsValidTime() const { return true; }
    virtual double        GetTime(void);
    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual vtkDataSet   *GetMesh(const char *var);
    virtual vtkDataArray *GetVar(const char *var);
    virtual void         *GetAuxiliaryData(const char *var, const char *type,
                                           void *args, DestructorFunction &);
    virtual void          FreeUpResources();
protected:
    PP_ZFileReader        reader;
};

#endif
