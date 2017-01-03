/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                         avtNamedSelectionManager.C                        //
// ************************************************************************* //

#include <cstring>
#include <sstream>
#include <avtNamedSelectionManager.h>
#include <avtNamedSelectionExtension.h>

#include <avtDataObjectSource.h>
#include <avtDataset.h>
#include <avtNamedSelection.h>

#include <DebugStream.h>
#include <snprintf.h>
#include <InstallationFunctions.h>
#include <StackTimer.h>
#include <visitstream.h>
#include <VisItException.h>


avtNamedSelectionManager *avtNamedSelectionManager::instance = NULL;


// ****************************************************************************
//  Method: avtNamedSelectionManager constructor
//
//  Programmer: Hank Childs
//  Creation:   January 30, 2009
//
// ****************************************************************************

avtNamedSelectionManager::avtNamedSelectionManager(void)
{
}


// ****************************************************************************
//  Method: avtNamedSelectionManager destructor
//
//  Programmer: Hank Childs
//  Creation:   January 30, 2009
//
//  Modifications:
//    Brad Whitlock, Tue Sep  6 16:00:38 PDT 2011
//    Clear the cache.
//
// ****************************************************************************

avtNamedSelectionManager::~avtNamedSelectionManager()
{
    cache.clear();
}


// ****************************************************************************
//  Method: avtNamedSelectionManager::GetInstance
//
//  Purpose:
//      Gets the singleton n.s.m. 
//
//  Programmer: Hank Childs
//  Creation:   January 30, 2009
//
// ****************************************************************************

avtNamedSelectionManager *
avtNamedSelectionManager::GetInstance(void)
{
    if (instance == NULL)
        instance = new avtNamedSelectionManager;

    return instance;
}

// ****************************************************************************
//  Method: avtNamedSelectionManager::CreateNamedSelection
//
//  Purpose:
//      Creates a named selection from a data object.
//
//  Arguments:
//    dob      : The data object used to create the named selection.
//    selProps : The named selection properties.
//    ext      : The named selection extension object that helps set up the 
//               pipeline and contract.
//
//  Programmer: Hank Childs
//  Creation:   January 30, 2009
//
//  Modifications:
//
//    Hank Childs, Sun Apr 19 18:40:32 PDT 2009
//    Fix problem with named selections on non-FastBit files.
//
//    Hank Childs, Sun Apr 19 22:42:09 PDT 2009
//    Fix problem with named selections not being overwritten.
//
//    Hank Childs, Mon Jul 13 15:53:54 PDT 2009
//    Automatically save out an internal named selection, for fault tolerance
//    and for save/restore sessions.
//
//    Brad Whitlock, Mon Dec 13 15:59:51 PST 2010
//    I added support for named selection "extensions" that add more stuff
//    to the pipeline before we reexecute it. I also changed things so we pass
//    in selection properties.
//
//    Brad Whitlock, Tue Jun 14 16:58:15 PST 2011
//    I fixed a memory corruption problem that caused bad selections to be
//    generated in parallel.
//
//    Brad Whitlock, Tue Sep  6 16:01:52 PDT 2011
//    I moved most of the code to do the selection into the extension.
//
//    Brad Whitlock, Fri Oct 28 09:57:55 PDT 2011
//    I changed avtNamedSelectionExtension and moved more code into it.
//
// ****************************************************************************

void
avtNamedSelectionManager::CreateNamedSelection(avtDataObject_p dob, 
    const SelectionProperties &selProps, avtNamedSelectionExtension *ext)
{
    StackTimer t0("CreateNamedSelection");

    if (strcmp(dob->GetType(), "avtDataset") != 0)
    {
        EXCEPTION1(VisItException, "Named selections only work on data sets");
    }

    // Save the selection properties.
    AddSelectionProperties(selProps);

    // Augment the contract based on the selection properties.
    avtContract_p c0 = dob->GetContractFromPreviousExecution();
    bool needsUpdate = false;
    avtContract_p contract = ext->ModifyContract(c0, selProps, needsUpdate);

    // 
    // Let the input try to create the named selection ... some have special
    // logic, for example the parallel coordinates filter.
    //
    const std::string &selName = selProps.GetName();
    avtNamedSelection *ns = NULL;
    if(selProps.GetSelectionType() == SelectionProperties::BasicSelection)
    {
        ns = dob->GetSource()->CreateNamedSelection(contract, selName);
        if (ns != NULL)
        {
            size_t curSize = selList.size();
            selList.resize(curSize+1);
            selList[curSize] = ns;

            //
            // Save out the named selection in case of engine crash / 
            // save/restore session, etc.
            //  
            SaveNamedSelection(selName, true);
            return;
        }
    }

    //
    // Call into the extension to get the per-processor named selection.
    //
    TimedCodeBlock("Creating selection in extension",
        ns = ext->GetSelection(dob, selProps, cache);
    );

    //
    // Save the selection
    //
    DeleteNamedSelection(selName, false); // Remove sel if it already exists.
    if(ns != NULL)
    {
        size_t curSize = selList.size();
        selList.resize(curSize+1);
        selList[curSize] = ns;
 
        //
        // Save out the named selection in case of engine crash / 
        // save/restore session, etc.
        //  
        SaveNamedSelection(selName, true);
    }
}


// ****************************************************************************
//  Method: avtNamedSelectionManager::DeleteNamedSelection
//
//  Purpose:
//      Deletes a named selection.
//
//  Programmer: Hank Childs
//  Creation:   January 30, 2009
//
//  Modifications:
//
//    Hank Childs, Sun Apr 19 22:42:09 PDT 2009
//    Add a default argument for whether or not the named selection should
//    be in the list.
//
// ****************************************************************************

void
avtNamedSelectionManager::DeleteNamedSelection(const std::string &name,
                                               bool shouldExpectSel)
{
    int numToRemove = -1;
    for (size_t i = 0 ; i < selList.size() ; i++)
    {
        if (selList[i]->GetName() == name)
        {
            numToRemove = (int)i;
            break;
        }
    }

    if (numToRemove == -1)
    {
        if (! shouldExpectSel)
            return;

        EXCEPTION1(VisItException, "Cannot delete selection; does not exist");
    }

    delete selList[numToRemove];

    std::vector<avtNamedSelection *> newList(selList.size()-1);
    for (int i = 0 ; i < numToRemove ; i++)
        newList[i] = selList[i];
    for (int i = numToRemove+1 ; i < (int)selList.size() ; i++)
        newList[i-1] = selList[i];

    selList = newList;
}


// ****************************************************************************
//  Method: avtNamedSelectionManager::LoadNamedSelection
//
//  Purpose:
//      Loads a named selection.
//
//  Programmer: Hank Childs
//  Creation:   January 30, 2009
//
//  Modifications:
//  
//    Tom Fogal, Sun May  3 19:21:44 MDT 2009
//    Fix string formatting when an exception occurs.
//
//    Hank Childs, Mon Jul 13 15:53:54 PDT 2009
//    Automatically save out an internal named selection, for fault tolerance
//    and for save/restore sessions.
//
// ****************************************************************************

bool
avtNamedSelectionManager::LoadNamedSelection(const std::string &name, 
                                             bool lookForInternalVar)
{
    std::string qualName = CreateQualifiedSelectionName(name, 
                                                        lookForInternalVar);
    ifstream ifile(qualName.c_str());
    if (ifile.fail())
    {
        if (lookForInternalVar)
            return false;
        std::ostringstream msg;
        msg << "Unable to load named selection from file: '"
            << qualName << "'";
        EXCEPTION1(VisItException, msg.str().c_str());
    }

    int fileType;
    ifile >> fileType;
    avtNamedSelection *ns = NULL;
    if (fileType == avtNamedSelection::ZONE_ID)
    {
        avtZoneIdNamedSelection *zins = new avtZoneIdNamedSelection(name);
        zins->Read(qualName);
        ns = zins;
    }
    else if (fileType == avtNamedSelection::FLOAT_ID)
    {
        avtFloatingPointIdNamedSelection *fins = 
                                   new avtFloatingPointIdNamedSelection(name);
        fins->Read(qualName);
        ns = fins;
    }
    else
    {
        std::ostringstream msg;
        msg << "Problem reading named selection from file: '"
            << qualName << "'";
        EXCEPTION1(VisItException, msg.str().c_str());
    }

    size_t curSize = selList.size();
    selList.resize(curSize+1);
    selList[curSize] = ns;

    return true;
}


// ****************************************************************************
//  Method: avtNamedSelectionManager::SaveNamedSelection
//
//  Purpose:
//      Saves a named selection.
//
//  Programmer: Hank Childs
//  Creation:   January 30, 2009
//
//  Modifications:
//  
//    Hank Childs, Mon Jul 13 15:53:54 PDT 2009
//    Automatically save out an internal named selection, for fault tolerance
//    and for save/restore sessions.
//
// ****************************************************************************

void
avtNamedSelectionManager::SaveNamedSelection(const std::string &name, 
                                             bool useInternalVarName)
{
    std::string qualName = CreateQualifiedSelectionName(name,
                                                 useInternalVarName);
    avtNamedSelection *ns = GetNamedSelection(name);
    if (ns == NULL)
    {
        EXCEPTION1(VisItException, "You have asked to save a named selection "
                                   "that does not exist.");
    }

    ns->Write(qualName);
}


// ****************************************************************************
//  Method: avtNamedSelectionManager::GetNamedSelection
//
//  Purpose:
//      Gets the named selection corresponding to a string.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//   
//    Hank Childs, Mon Jul 13 15:53:54 PDT 2009
//    Try to load a named selection from a file, assuming that we are in a
//    save/restore session situation or that the engine crashed.
//
// ****************************************************************************

avtNamedSelection *
avtNamedSelectionManager::GetNamedSelection(const std::string &name)
{
    avtNamedSelection *rv = IterateOverNamedSelections(name);
    if (rv != NULL)
        return rv;

    if (LoadNamedSelection(name, true))
    {
        return IterateOverNamedSelections(name);
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtNamedSelectionManager::IterateOverNamedSelections
//
//  Purpose:
//      An internal method that locates a named selection.  This method
//      exists so there isn't duplicated code floating around.
//
//  Programmer: Hank Childs
//  Creation:   July 13, 2009
//
// ****************************************************************************

avtNamedSelection *
avtNamedSelectionManager::IterateOverNamedSelections(const std::string &name)
{
    for (size_t i = 0 ; i < selList.size() ; i++)
    {
        if (selList[i]->GetName() == name)
            return selList[i];
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtNamedSelectionManager::CreateQualifiedSelectionName
//
//  Purpose:
//      An internal method that creates a (directory qualified) file name.  
//      This method exists so there isn't duplicated code floating around.
//
//  Programmer: Hank Childs
//  Creation:   July 13, 2009
//
// ****************************************************************************

std::string
avtNamedSelectionManager::CreateQualifiedSelectionName(const std::string &name,
                                                       bool useInternalVarName)
{
    std::string qualName;
    if (useInternalVarName)
        qualName = GetUserVisItDirectory() + "_internal_" + name + ".ns";
    else
        qualName = GetUserVisItDirectory() + name + ".ns";
    return qualName;
}

// ****************************************************************************
// Method: avtNamedSelectionManager::GetSelectionProperties
//
// Purpose: 
//   Gets the selection properties based on the selection name.
//
// Arguments:
//   selName : The name of the selection for which to get selection properties.
//
// Returns:    A pointer to the selection properties or NULL if not found.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 14:19:56 PST 2010
//
// Modifications:
//   
// ****************************************************************************

const SelectionProperties *
avtNamedSelectionManager::GetSelectionProperties(const std::string &selName) const
{
    for(size_t i = 0; i < properties.size(); ++i)
    {
        if(selName == properties[i].GetName())
            return &properties[i];
    }
    return NULL;
}

// ****************************************************************************
// Method: avtNamedSelectionManager::AddSelectionProperties
//
// Purpose: 
//   Adds the new selection properties to the list of selection properties, 
//   overwriting properties with the same name, if present.
//
// Arguments:
//   srcp : The new selection properties.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 14:21:14 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
avtNamedSelectionManager::AddSelectionProperties(const SelectionProperties &srcp)
{
    SelectionProperties *p = NULL;
    for(size_t i = 0; i < properties.size(); ++i)
    {
        if(srcp.GetName() == properties[i].GetName())
        {
            p = &properties[i];
            break;
        }
    }

    if(p != NULL)
        *p = srcp;
    else
        properties.push_back(srcp);
}

// ****************************************************************************
// Method: avtNamedSelectionManager::ClearCache
//
// Purpose: 
//   Clear the cache.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  7 13:32:46 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
avtNamedSelectionManager::ClearCache(const std::string &selName)
{
    if(selName.empty())
        cache.clear();
    else
    {
        avtNamedSelectionCache::iterator it = cache.begin();
        while(it != cache.end())
        {
            if(it->second->properties.GetName() == selName)
            {
                cache.remove(it->first);
                it = cache.begin();
            }
            else
                it++;
        }
    }
}
