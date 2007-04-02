/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef PP_Z_MTSD_FILE_FORMAT_H
#define PP_Z_MTSD_FILE_FORMAT_H
#include <avtMTSDFileFormat.h>

class avtFileFormatInterface;
class PP_ZFileReader;
class PDBFileObject;

// ****************************************************************************
// Class: PP_Z_MTSD_FileFormat
//
// Purpose:
//   This subclass of PP_ZFileFormat only works for PPZ files that have
//   multiple time states per file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 13:41:07 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Sep 1 23:52:14 PST 2004
//   Added FreeUpResourcesForTimeStep method, lastTimeState, and 
//   timeFlowsForward members.
//
// ****************************************************************************

class PP_Z_MTSD_FileFormat : public avtMTSDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *const *filenames, int nList);

    PP_Z_MTSD_FileFormat(const char *filename, const char * const *list, int nList);
    PP_Z_MTSD_FileFormat(PDBFileObject *p, const char * const *list, int nList);
    virtual ~PP_Z_MTSD_FileFormat();

    // Mimic PDBReader interface.
    virtual bool Identify();
    void SetOwnsPDBFile(bool);

    // MTSD file format methods.
    virtual const char   *GetType();
    virtual void          GetCycles(std::vector<int> &cycles);
    virtual void          GetTimes(std::vector<double> &times);
    virtual int           GetNTimesteps();
    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual void          GetTimeVaryingInformation(avtDatabaseMetaData *);
    virtual void         *GetAuxiliaryData(const char *var,
                                           int timeState,
                                           const char *type,
                                           void *args,
                                           DestructorFunction &);

    virtual vtkDataSet   *GetMesh(int ts, const char *var);
    virtual vtkDataArray *GetVar(int ts, const char *var);

    void FreeUpResourcesForTimeStep(int ts);
private:
    void DetermineTimeFlow(int);
    int  GetReaderIndexAndTimeStep(int ts, int &localTimeState);

    int                    nReaders;
    PP_ZFileReader       **readers;
    int                    nTotalTimeSteps;
    int                    lastTimeState;
    bool                   timeFlowsForward;
};

#endif
