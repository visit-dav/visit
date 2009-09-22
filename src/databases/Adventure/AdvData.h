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
#ifndef ADV_DATA_H
#define ADV_DATA_H
#include <Adv/AdvDocument.h>
#include <map>
#include <vector>
#include <string>

#include "AdvDataSet.h"

#define MAX_ADV_FILES 10

class vtkDataArray;
class vtkDataSet;

// ****************************************************************************
// Class: AdvData
//
// Purpose:
//   This class manages access to a set of ADV files as a multidomain object.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:08:33 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class AdvData
{
public:
    AdvData();
    ~AdvData();

    void ReleaseData();
    bool Open(const std::string &filename);
    int  GetNumDomains() const;
    void GetMetaData(AdvDataSet::VarInfoVector &vars, AdvDataSet::AdvElementType &et);

    vtkDataSet   *GetMesh(int domain);
    vtkDataArray *GetVar(int domain, const std::string &var);

private:
    AdvDocFile *OpenFile(const std::string &filename);
    void ReadMetaDataFromFile(AdvDocFile *f, AdvDataSet::VarInfoVector &, 
                              AdvDataSet::AdvElementType &);

    std::vector<AdvDataSet> domainRecords;
    std::map<int,int>       domainToRecord;
    AdvDocFile             *advFiles[MAX_ADV_FILES];

    AdvDataSet::AdvElementType elementType;
    AdvDataSet::VarInfoVector  modelVars;
    AdvDataSet::VarInfoVector  resultVars;
};

#endif
