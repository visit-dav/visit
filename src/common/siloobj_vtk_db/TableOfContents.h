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

// ************************************************************************* //
//                           TableOfContents.h                               //
// ************************************************************************* //

#ifndef TABLE_OF_CONTENTS_H
#define TABLE_OF_CONTENTS_H
#include <siloobj_vtk_exports.h>

#include <silo.h>

#include <Field_VTK.h>
#include <IntervalTree_VTK.h>
#include <Value_VTK.h>


// ****************************************************************************
//  Class: TableOfContents
//
//  Purpose:
//      Reads in the meshtvprep SILO format from a set of files.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

class SILOOBJ_VTK_API TableOfContents
{
  public:
                                TableOfContents();
    virtual                    ~TableOfContents();

    virtual Field_VTK          *GetField(const char *) = 0;
    virtual IntervalTree_VTK   *GetIntervalTree(const char *) = 0;
    virtual Value_VTK          *GetValue(const char *) = 0;

    void                        Read(int, const char * const *);
    void                        PrintSelf();

    void                        CloseSILOFile();
    DBfile                     *GetSILOFile(const char *);

  protected:
    int                         nFiles;
    char                      **files;

    DBfile                     *openFile;
    char                       *openFileName;

    // Protected Methods
    virtual void                ReadFile(const char *) = 0;
    virtual void                UpdateReferences(void) = 0;
};


#endif


