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

#ifndef AVT_DEBUG_DUMP_OPTIONS_H
#define AVT_DEBUG_DUMP_OPTIONS_H

#include <pipeline_exports.h>
#include <visitstream.h>
#include <string>

// ****************************************************************************
// Class: avtDebugDumpOptions
//
// Purpose:
//     Provides static members that define global debug dump options.
//     These options were migrated from the avtFilter, avtTerminatingSink
//     and avtDataRepresentation classes b/c of growing redundancy. 
//
//
// Programmer: Cyrus Harrison
// Creation:   Feburary 13, 2009
//
// Modifications: 
//
// ****************************************************************************

class PIPELINE_API avtDebugDumpOptions
{
  public:
    virtual                  ~avtDebugDumpOptions();
    
    static void               EnableDump();
    static void               DisableDump();
    static bool               DumpEnabled() {return doDump;}
    
    static void               EnableDatasetDump();
    static void               DisableDatasetDump();
    static bool               DatasetDumpEnabled() {return doDatasetDump;}
    
    static const std::string &GetDumpDirectory() {return outputDir;}
    static void               SetDumpDirectory(const std::string &);
    
  private:
    // we dont want anyone creating an instance of this class, so make
    // its constructor private.
    avtDebugDumpOptions();
    
    // static members
    static std::string   outputDir;
    static bool          doDump;
    static bool          doDatasetDump;
    
};


#endif


