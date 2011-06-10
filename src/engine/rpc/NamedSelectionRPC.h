/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

#ifndef NAMED_SELECTION_RPC_H
#define NAMED_SELECTION_RPC_H

#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <SelectionProperties.h>
#include <SelectionSummary.h>

// ****************************************************************************
//  Class:  NamedSelectionRPC
//
//  Purpose:
//    Implements an RPC for named selections.
//
//  Programmer:  Hank Childs 
//  Creation:    January 29, 2009
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 25 15:35:32 MST 2009
//    Renamed NamedSelectionType enum names to compile on windows.
//
//    Brad Whitlock, Tue Dec 14 12:13:28 PST 2010
//    I changed the invocation methods and added selection properties when we
//    create a selection. I also added a SelectionSummary return value.
//
//    Brad Whitlock, Wed Jun  8 16:44:15 PDT 2011
//    I made it be non-blocking.
//
// ****************************************************************************

class ENGINE_RPC_API NamedSelectionRPC : public NonBlockingRPC
{
public:
    typedef enum
    {
        NS_APPLY       = 0,
        NS_CREATE,    /* 1 */
        NS_DELETE,    /* 2 */
        NS_LOAD,      /* 3 */
        NS_SAVE       /* 4 */
    } NamedSelectionOperation;

    NamedSelectionRPC();
    virtual ~NamedSelectionRPC();

    virtual const std::string TypeName() const { return "NamedSelectionRPC"; }

    // Invocation method
    void ApplyNamedSelection(const std::vector<std::string> &ids, const std::string &selName);
    const SelectionSummary &CreateNamedSelection(int id, const SelectionProperties &);
    void DeleteNamedSelection(const std::string &selName);
    void LoadNamedSelection(const std::string &selName);
    void SaveNamedSelection(const std::string &selName);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetPlotNames(const std::vector<std::string> &ids);
    void SetPlotID(int);
    void SetSelectionName(const std::string &s);
    void SetNamedSelectionOperation(NamedSelectionOperation t);
    void SetSelectionProperties(const SelectionProperties &p);

    // Property getting methods
    const std::vector<std::string> &GetPlotNames(void) const { return plotNames; }
    int                             GetPlotID(void) const { return plotId; }
    const std::string              &GetSelectionName(void) const { return selName; }
    NamedSelectionOperation         GetNamedSelectionOperation(void) const { return selOperation; }
    const SelectionProperties      &GetSelectionProperties() const { return properties; }
private:
    std::vector<std::string> plotNames;
    int                      plotId;
    std::string              selName;
    NamedSelectionOperation  selOperation;
    SelectionProperties      properties;

    // Return values
    SelectionSummary         summary;
};

#endif


