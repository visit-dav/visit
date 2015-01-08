/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#ifndef AVT_PLOT_META_DATA_H
#define AVT_PLOT_META_DATA_H
#include <plotter_exports.h>
#include <avtSILRestriction.h>
#include <avtTypes.h>
#include <avtDatabaseMetaData.h>
#include <string>

// ****************************************************************************
// Class: avtPlotMetaData
//
// Purpose:
//   This class contains the metadata that we use to initialize plot and 
//   operator plugins. We use this class so we can initialize both viewer and
//   engine plugins the same way without resorting to ViewerPlot.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 16 10:34:05 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class PLOTTER_API avtPlotMetaData
{
public:
    avtPlotMetaData(const avtDatabaseMetaData  *_md,
                    const std::string          &_variableName,
                    avtVarType                  _variableType,
                    avtSILRestriction_p         _silr);
    avtPlotMetaData(const avtPlotMetaData &obj);
    ~avtPlotMetaData();

    void operator = (const avtPlotMetaData &obj);

    const avtDatabaseMetaData *GetMetaData() const;
    const std::string         &GetVariableName() const;
    std::string                GetMeshName() const;
    avtVarType                 GetVariableType() const;
    avtSILRestriction_p        GetSILRestriction() const;

private:
    const avtDatabaseMetaData *md;
    std::string                variableName;
    avtVarType                 variableType;
    avtSILRestriction_p        silr;
};

#endif
