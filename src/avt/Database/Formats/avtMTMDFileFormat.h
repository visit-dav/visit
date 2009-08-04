/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

// ************************************************************************* //
//                             avtMTMDFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MTMD_FILE_FORMAT_H
#define AVT_MTMD_FILE_FORMAT_H

#include <database_exports.h>

#include <avtFileFormat.h>

#include <vector>

#include <void_ref_ptr.h>


class     vtkDataSet;
class     vtkDataArray;

class     avtIOInformation;


// ****************************************************************************
//  Class: avtMTMDFileFormat
//
//  Purpose:
//      This is an implementation of avtFileFormat for file formats that have
//      a Multiple Timesteps and Multiple Domains.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Brad Whitlock, Mon Oct 13 14:17:30 PST 2003
//    Added a method to get the times.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Tue Mar 16 14:28:42 PST 2004
//    Added method, PopulateIOInformation
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Moved GetCycles/Times to base class, avtFileFormat
//    Added SetDatabaseMetaData (moved it down from base class)
//    Added time-qualified and non-time-qualified PopulateDatabaseMetaData
//    methods, the later for backward compatibility
//
// ****************************************************************************

class DATABASE_API avtMTMDFileFormat : public avtFileFormat
{
  public:
                           avtMTMDFileFormat(const char *);
    virtual               ~avtMTMDFileFormat();

    virtual void          *GetAuxiliaryData(const char *var, int, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetFilename(void) { return filename; };

    virtual vtkDataSet    *GetMesh(int, int, const char *) = 0;
    virtual vtkDataArray  *GetVar(int, int, const char *) = 0;
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

    virtual void           ActivateTimestep(int ts)
                               { avtFileFormat::ActivateTimestep(); };
    virtual void           PopulateIOInformation(int ts, avtIOInformation& ioInfo)
                               { avtFileFormat::PopulateIOInformation(ioInfo); };
    virtual void           SetDatabaseMetaData(avtDatabaseMetaData *md, int ts = 0)
                               { metadata = md; PopulateDatabaseMetaData(metadata, ts); };
  protected:
    char                  *filename;

    // The second of these should really be pure virtual and the first
    // non-existant. However, both are just virtual to maintain 
    // backward compatibility with older MTXX plugins and to allow 
    // MTXX plugins to implement a time-qualified request to populate
    // database metadata.
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *md);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *md, int);

};


#endif


