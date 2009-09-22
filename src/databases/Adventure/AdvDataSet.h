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
#ifndef ADV_DATA_SET_H
#define ADV_DATA_SET_H
#include <string>
#include <map>
#include <vector>
#include <Adv/AdvDocument.h>

class vtkDataArray;
class vtkDataSet;

// ****************************************************************************
// Class: AdvDataSet
//
// Purpose:
//   This class manages the conversion of ADV data to VTK data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 17 16:31:10 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class AdvDataSet
{
public:
    struct VarInfo
    {
        std::string label;
        std::string fega_type;
        std::string format;

        bool CheckFormat(int &ncomps) const;
    };

    typedef std::vector<VarInfo> VarInfoVector;

    typedef enum
    { 
        ADVENTURE_ELEMENT_TET4,
        ADVENTURE_ELEMENT_TET10,
        ADVENTURE_ELEMENT_HEX8
    } AdvElementType;

    AdvDataSet();
    ~AdvDataSet();

    void SetDomainToSubDomain(int domainID, int subDomain);
    int  GetNumSubDomains() const;

    vtkDataSet   *GetMesh(AdvDocFile *f, int domainID, AdvElementType);
    vtkDataArray *GetNodeVar(AdvDocFile *f, AdvDocument *nodeIndexDoc, 
                             int domainID, const VarInfo &var);
    vtkDataArray *GetElementVar(AdvDocFile *f, int domainID, 
                                const VarInfo &var);

    std::string        modelFile;
    std::string        resultFile;
private:
    vtkDataArray *GetAllElementVariable(AdvDocFile *f, int subDomain,
                                        const VarInfo &var);
    vtkDataArray *GetAllConstant(AdvDocFile *f, int subDomain, 
                                 const VarInfo &var, int ntuples);
    vtkDataArray *GetAllNodeVariable(AdvDocFile *f, AdvDocument *nodeIndexDoc, 
                                     int subDomain, const VarInfo &var);
    void AddGlobalNodeIds(vtkDataSet *ds, AdvDocFile *f);

    std::map<int,int>  domainToSubDomain;
    int                nnodes;
    int                ncells;
};

#endif
