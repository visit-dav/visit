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
#ifndef AVT_NETCDF_READER_BASE_H
#define AVT_NETCDF_READER_BASE_H
#include <map>
#include <string>
#include <vectortypes.h>

class NETCDFFileObject;
class avtDatabaseMetaData;
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
// Class: avtNETCDFReaderBase
//
// Purpose:
//   This class is a base class for "Reader" classes. which I mean to distinguish
//   as the class that does the actual NETCDF reading. The various FileFormat 
//   classes call a reader to do their work. There are MTSD and STSD versions
//   of the FileFormat which call the same reader class. This permits the plugin 
//   to serve up the best avtFileFormatInterface for the data with respect to 
//   whether it is "MT" or "ST". Thus, the FileFormat classes become a very
//   thin veneer on top of the Reader class. We do this mostly to work around a
//   file grouping issue in avtDatabase where it can't group MT files. The other
//   reason is that we don't want to have an ST or MT file format because we
//   don't know which it is until the file is opened.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 27 14:39:32 PDT 2009
//
// Modifications:
//   Eric Brugger, Mon Nov 16 07:59:52 PST 2009
//   I added ReadTimeAttribute and ReadCycleAttribute.
//   
// ****************************************************************************

class avtNETCDFReaderBase
{
public:
                   avtNETCDFReaderBase(const char *);
                   avtNETCDFReaderBase(const char *, NETCDFFileObject *);
    virtual       ~avtNETCDFReaderBase();

    void           GetCycles(std::vector<int> &);
    void           GetTimes(std::vector<double> &);

    int            GetNTimesteps();
    void           FreeUpResources(); 

    static bool GetTimeDimension(NETCDFFileObject *, int &ncdim, int &nts, std::string &name);
protected:
    float *ReadTimeAttribute();
    int    ReadCycleAttribute();
    float *ReadArray(const char *varname);

    typedef std::map<std::string, intVector>   StringIntVectorMap;
    typedef std::map<std::string, std::string> StringStringMap;

    // DATA MEMBERS
    NETCDFFileObject      *fileObject;
};

#endif
