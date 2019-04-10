/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                              avtVTKWriter.h                               //
// ************************************************************************* //

#ifndef AVT_VTK_WRITER_H
#define AVT_VTK_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>
#include <vector>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtVTKWriter
//
//  Purpose:
//      A module that writes out VTK files.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Better support for single-block.
//
//    Jeremy Meredith, Tue Mar 27 11:39:24 EDT 2007
//    Added numblocks to the OpenFile method, and save off the actual
//    encountered mesh types, because we cannot trust the metadata.
//
//    Kathleen Biagas, Thu Dec 18 14:19:13 PST 2014
//    Added doXML.
//
//    Kathleen Biagas, Wed Feb 25 13:25:07 PST 2015
//    Added meshName.
//
//    Kathleen Biagas, Tue Sep  1 11:27:23 PDT 2015
//    Add storage for fileNames, used when exporting multi-block XML.
//
//    Mark C. Miller, Tue Apr  9 18:44:50 PDT 2019
//    Add tetrahedralize option.
// ****************************************************************************

class
avtVTKWriter : public virtual avtDatabaseWriter
{
  public:
                   avtVTKWriter(DBOptionsAttributes *);
    virtual       ~avtVTKWriter() {;};

  protected:
    static int            INVALID_CYCLE;
    static double         INVALID_TIME;

    std::string    stem;
    std::string    meshName;
    std::string    mbDirName;
    double         time;
    int            cycle;
    bool           doBinary;
    bool           doXML;
    bool           tetrahedralize;
    int            nblocks;
    std::vector<std::string> fileNames;

    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
    virtual void   WriteRootFile();
};


#endif


