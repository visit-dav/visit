// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_NAMED_SELECTION_MANAGER_H
#define AVT_NAMED_SELECTION_MANAGER_H

#include <pipeline_exports.h>

#include <vector>

#include <avtDataObject.h>
#include <avtNamedSelection.h>
#include <avtNamedSelectionExtension.h>

#include <MRUCache.h>
#include <SelectionProperties.h>
#include <visitstream.h>

// ****************************************************************************
//  Class: avtNamedSelectionManager
//
//  Purpose:
//      Manager for named selections.
//
//  Programmer: Hank Childs
//  Creation:   January 30, 2009
//
//  Modifications:
//
//    Hank Childs, Sun Apr 19 22:42:09 PDT 2009
//    Add argument to DeleteNamedSelection.
//
//    Hank Childs, Mon Jul 13 15:53:54 PDT 2009
//    Add arguments to [Get|Save]NamedSelection for automatic saving and
//    loading of selections to help with fault tolerance, save/restore 
//    sessions, etc.
//
//    Brad Whitlock, Mon Dec 13 16:15:41 PST 2010
//    I added an avtNamedSelectionExtension argument to CreateNamedSelection,
//    which lets us pass in an object that can perform additional setup for a
//    selection based on its properties. This class manages named selections,
//    which are the actual resulting data that is used to restrict selections.
//    I also added selection properties which describe how the selection was 
//    defined. These properties can be queried in the event that we need to
//    create a selection using some other method.
//
//    Brad Whitlock, Tue Sep  6 15:15:53 PDT 2011
//    I added a cache.
//
// ****************************************************************************

class PIPELINE_API avtNamedSelectionManager
{
  public:
                  avtNamedSelectionManager();
    virtual      ~avtNamedSelectionManager();
    
    static avtNamedSelectionManager *
                  GetInstance(void);

    avtNamedSelection *
                  GetNamedSelection(const std::string &);

    void          CreateNamedSelection(avtDataObject_p,
                                       const SelectionProperties &,
                                       avtNamedSelectionExtension *);

    void          DeleteNamedSelection(const std::string &, 
                                       bool expectThisSelToBeThere);
    bool          LoadNamedSelection(const std::string &, bool = false);
    void          SaveNamedSelection(const std::string &, bool = false);

    void          ClearCache(const std::string &selName = std::string());

    const SelectionProperties *GetSelectionProperties(const std::string &selName) const;

    static int MaximumSelectionSize();

  protected:
    static avtNamedSelectionManager    *instance;
    std::vector<avtNamedSelection *>    selList;

    void          AddSelectionProperties(const SelectionProperties &);
    std::vector<SelectionProperties>    properties;

    avtNamedSelectionCache              cache;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtNamedSelectionManager(const avtNamedSelectionManager &) {;};
    avtNamedSelectionManager          &operator=(const avtNamedSelectionManager &) { return *this; };

    std::string          CreateQualifiedSelectionName(const std::string &, bool);
    avtNamedSelection   *IterateOverNamedSelections(const std::string &);
};


#endif


