/*****************************************************************************
*
* Copyright (c) 2011, Allinea
* All rights reserved.
*
* This file has been contributed to the VisIt project, which is
* Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
* https://visit.llnl.gov/.  The full copyright notice is contained in the 
* file COPYRIGHT located at the root of the VisIt distribution or at 
* http://www.llnl.gov/visit/copyright.html.
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

#ifndef VISITVARIABLE_H
#define VISITVARIABLE_H

#include "ddtsim_constants.h"
#include "VisItInterfaceTypes_V2.h"
#include "libsimlibrary.h"
#include "visitmesh.h"

class VisItVariable
{
public:
    VisItVariable();
    VisItVariable(const VisItVariable&);

    bool operator==(const VisItVariable&) const;
    bool isNamed(const char*) const;
    void setName(const char *);
    void setMeshName(const char*);

    void applyVariable(LibsimLibrary&, visit_handle &var);
    void applyMetadata(LibsimLibrary&, visit_handle &metadata);

private:
    char mName[MAX_NAME_LENGTH];    //!< Name of this variable
    char mMeshName[MAX_NAME_LENGTH];//!< Name of the mesh this variable is drawn on
    VisItVarType mVarType;          //!< Type of this variable: scalar, vector, etc
    VisItCenteringType mCentering;  //!< How variable is centered on the mesh: zone or node
    VisItDataType mDataType;        //!< The datatype of this variable: float, int, etc
    int mNumComponents;             //!< The number of array elements (in mData) that comprise a single tuple
    int mNumTuples;                 //!< The number of tuples in the array mData
    void* mData;                    //!< Address of the start of the array to visualise
};

#endif // VISITVARIABLE_H
