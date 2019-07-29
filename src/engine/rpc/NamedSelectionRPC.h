// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Brad Whitlock, Wed Sep  7 14:21:25 PDT 2011
//    Added UpdateNamedSelection.
//
// ****************************************************************************

class ENGINE_RPC_API NamedSelectionRPC : public NonBlockingRPC
{
public:
    typedef enum
    {
        NS_CREATE      = 0,
        NS_UPDATE,    /* 3 */
        NS_DELETE,    /* 2 */
        NS_LOAD,      /* 3 */
        NS_SAVE       /* 4 */
    } NamedSelectionOperation;

    NamedSelectionRPC();
    virtual ~NamedSelectionRPC();

    virtual const std::string TypeName() const { return "NamedSelectionRPC"; }

    // Invocation method
    const SelectionSummary &CreateNamedSelection(int id, const SelectionProperties &);
    const SelectionSummary &UpdateNamedSelection(int id, const SelectionProperties &, bool);
    void DeleteNamedSelection(const std::string &selName);
    void LoadNamedSelection(const std::string &selName);
    void SaveNamedSelection(const std::string &selName);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetPlotID(int);
    void SetSelectionName(const std::string &s);
    void SetNamedSelectionOperation(NamedSelectionOperation t);
    void SetSelectionProperties(const SelectionProperties &p);
    void SetAllowCache(bool);

    // Property getting methods
    int                             GetPlotID(void) const { return plotId; }
    const std::string              &GetSelectionName(void) const { return selName; }
    NamedSelectionOperation         GetNamedSelectionOperation(void) const { return selOperation; }
    const SelectionProperties      &GetSelectionProperties() const { return properties; }
    bool                            GetAllowCache() const { return allowCache; }
private:
    NamedSelectionOperation  selOperation;
    int                      plotId;
    std::string              selName;
    SelectionProperties      properties;
    bool                     allowCache;

    // Return values
    SelectionSummary         summary;
};

#endif


