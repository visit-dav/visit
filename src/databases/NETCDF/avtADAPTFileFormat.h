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

#ifndef AVT_ADAPT_FILE_FORMAT_H
#define AVT_ADAPT_FILE_FORMAT_H
#include <avtSTSDFileFormat.h>

class NETCDFFileObject;
class avtFileFormatInterface;

// ****************************************************************************
// Class: avtADAPTFileFormat
//
// Purpose:
//   Reads ADAPT data from a NETCDF file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 09:51:09 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class avtADAPTFileFormat : public avtSTSDFileFormat
{
  public:
    static bool        Identify(NETCDFFileObject *);
    static avtFileFormatInterface *CreateInterface(
                       NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtADAPTFileFormat(const char *filename,
                                          NETCDFFileObject *);
                       avtADAPTFileFormat(const char *filename);
    virtual           ~avtADAPTFileFormat();

    virtual const char    *GetType(void) { return "ADAPT"; }
    virtual void           ActivateTimestep(void);
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    std::string            GetGridFileName();
    NETCDFFileObject      *GetMeshFile();

    vtkDataSet            *CreateMain2D(const float *, int, const float *, int);
    vtkDataSet            *CreateMain(const float *, int, const float *, int);
  private:
    NETCDFFileObject      *fileObject;
    NETCDFFileObject      *meshFile;
};

#endif
