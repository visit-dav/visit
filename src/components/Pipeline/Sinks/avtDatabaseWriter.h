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

// ************************************************************************* //
//                            avtDatabaseWriter.h                            //
// ************************************************************************* //

#ifndef AVT_DATABASE_WRITER_H
#define AVT_DATABASE_WRITER_H

#include <avtOriginatingDatasetSink.h>

#include <string>
#include <vector>

#include <pipeline_exports.h>


class     avtDatabaseMetaData;


// ****************************************************************************
//  Class: avtDatabaseWriter
//
//  Purpose:
//      This is a general, abstract interface for writing out databases.  It is
//      designed to be utilized by the plugin writers.
//      This differs from avtDatasetFileWriter, in that avtDatasetFileWriter is
//      specialized to specific polygonal file formats.  That writer should
//      probably be renamed.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Hank Childs, Sat Sep 11 10:03:49 PDT 2004
//    Add support for repartitioning the chunks.  Also support reading only
//    one variable.
//
//    Brad Whitlock, Mon Nov 1 15:51:32 PST 2004
//    I replaced long long with VISIT_LONG_LONG so the code can build 
//    on Windows using the MSVC 6.0 compiler.
//
//    Jeremy Meredith, Wed Feb 16 15:01:40 PST 2005
//    Added ability to disable MIR and Expressions.
//
//    Hank Childs, Thu May 26 15:45:48 PDT 2005
//    Add an "all-variables" option.
//
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
// ****************************************************************************

class PIPELINE_API avtDatabaseWriter : public virtual avtOriginatingDatasetSink
{
  public:
                       avtDatabaseWriter();
    virtual           ~avtDatabaseWriter();
   
    void               Write(const std::string &, const avtDatabaseMetaData *);
    void               Write(const std::string &, const avtDatabaseMetaData *,
                             std::vector<std::string> &, bool allVars = true);

    void               SetShouldAlwaysDoMIR(bool s)
                             { shouldAlwaysDoMIR = s; };

    void               SetShouldNeverDoMIR(bool s)
                             { shouldNeverDoMIR = s; };

    void               SetShouldNeverDoExpressions(bool s)
                             { shouldNeverDoExpressions = s; };

    bool               SetTargetChunks(int nChunks);
    bool               SetTargetZones(VISIT_LONG_LONG nTotalZones);
    void               SetVariableList(std::vector<std::string> &);

  protected:
    bool               shouldAlwaysDoMIR;
    bool               shouldNeverDoMIR;
    bool               shouldNeverDoExpressions;
    bool               mustGetMaterialsAdditionally;
    bool               hasMaterialsInProblem;

    bool               shouldChangeChunks;
    bool               shouldChangeTotalZones;
    int                nTargetChunks;
    VISIT_LONG_LONG    targetTotalZones;

    virtual bool       CanHandleMaterials(void) { return false; };

    virtual void       OpenFile(const std::string &) = 0;
    virtual void       WriteHeaders(const avtDatabaseMetaData *, 
                           std::vector<std::string>&,std::vector<std::string>&,
                           std::vector<std::string> &) = 0;
    virtual void       WriteChunk(vtkDataSet *, int) = 0;
    virtual void       CloseFile(void) = 0;

    virtual bool       SupportsTargetChunks(void) { return false; };
    virtual bool       SupportsTargetZones(void) { return false; };
};


#endif


