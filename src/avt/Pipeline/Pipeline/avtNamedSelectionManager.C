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

// ************************************************************************* //
//                         avtNamedSelectionManager.C                        //
// ************************************************************************* //

#include <sstream>
#include <avtNamedSelectionManager.h>
#include <avtNamedSelectionExtension.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <avtDataObjectSource.h>
#include <avtDataset.h>
#include <avtNamedSelection.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <snprintf.h>
#include <InstallationFunctions.h>
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
// ****************************************************************************

avtNamedSelectionManager::~avtNamedSelectionManager()
{
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
// ****************************************************************************

void
avtNamedSelectionManager::CreateNamedSelection(avtDataObject_p dob, 
    const SelectionProperties &selProps, avtNamedSelectionExtension *ext)
{
    int   i;

    if (strcmp(dob->GetType(), "avtDataset") != 0)
    {
        EXCEPTION1(VisItException, "Named selections only work on data sets");
    }

    // Save the selection properties.
    AddSelectionProperties(selProps);

    avtContract_p c1 = dob->GetContractFromPreviousExecution();
    avtContract_p contract;
    if (c1->GetDataRequest()->NeedZoneNumbers() == false)
    {
        // If we don't have zone numbers, then get them, even if we have
        // to re-execute the whole darn pipeline.
        contract = new avtContract(c1);
        contract->GetDataRequest()->TurnZoneNumbersOn();
    }
    else
    {
        contract = c1;
    }

    // 
    // Let the input try to create the named selection ... some have special
    // logic, for example the parallel coordinates filter.
    //
    const std::string &selName = selProps.GetName();
    avtNamedSelection *ns = dob->GetSource()->CreateNamedSelection(contract, 
                                                                   selName);
    if (ns != NULL)
    {
        int curSize = selList.size();
        selList.resize(curSize+1);
        selList[curSize] = ns;

        //
        // Save out the named selection in case of engine crash / 
        // save/restore session, etc.
        //  
        SaveNamedSelection(selName, true);

        return;
    }

    bool needZoneNumbers = c1->GetDataRequest()->NeedZoneNumbers() == false;
    avtDataset_p ds;
    if(ext != 0)
    {
        // Perform additional setup using the extension.
        avtDataObject_p newdob = ext->GetSelectedData(dob, contract, selProps);

        debug5 << "Must execute the pipeline to create the named selection" << endl;
        newdob->Update(contract);
        debug5 << "Done executing the pipeline to create the named selection" << endl;

        CopyTo(ds, newdob);
    }
    else
    {
        if (needZoneNumbers)
        {
            debug1 << "Must re-execute pipeline to create named selection" << endl;
            dob->Update(contract);
            debug1 << "Done re-executing pipeline to create named selection" << endl;
        }
        CopyTo(ds, dob);
    }

    avtDataTree_p tree = ds->GetDataTree();
    std::vector<int> doms;
    std::vector<int> zones;
    int nleaves = 0;
    vtkDataSet **leaves = tree->GetAllLeaves(nleaves);
    for (i = 0 ; i < nleaves ; i++)
    {
        if (leaves[i]->GetNumberOfCells() == 0)
            continue;

        vtkDataArray *ocn = leaves[i]->GetCellData()->
                                            GetArray("avtOriginalCellNumbers");
        if (ocn == NULL)
        {
            delete [] leaves;
            EXCEPTION0(ImproperUseException);
        }
        unsigned int *ptr = (unsigned int *) ocn->GetVoidPointer(0);
        if (ptr == NULL)
        {
            delete [] leaves;
            EXCEPTION0(ImproperUseException);
        }

        int ncells = leaves[i]->GetNumberOfCells();
        int curSize = doms.size();
        doms.resize(curSize+ncells);
        zones.resize(curSize+ncells);
        for (int j = 0 ; j < ncells ; j++)
        {
            doms[curSize+j]  = ptr[2*j];
            zones[curSize+j] = ptr[2*j+1];
        }
    }
    delete [] leaves;

    // Let the extension free its resources. (We could just do this later...)
    if(ext != 0)
        ext->FreeUpResources();

    // Note the poor use of MPI below, coded for expediency, as I believe all
    // of the named selections will be small.
    int *numPerProcIn = new int[PAR_Size()];
    int *numPerProc   = new int[PAR_Size()];
    for (i = 0 ; i < PAR_Size() ; i++)
        numPerProcIn[i] = 0;
    numPerProcIn[PAR_Rank()] = doms.size();
    SumIntArrayAcrossAllProcessors(numPerProcIn, numPerProc, PAR_Size());
    int numTotal = 0;
    for (i = 0 ; i < PAR_Size() ; i++)
        numTotal += numPerProc[i];
    if (numTotal > 1000000)
    {
        EXCEPTION1(VisItException, "You have selected too many zones in your "
                   "named selection.  Disallowing ... no selection created");
    }
    int myStart = 0;
    for (i = 0 ; i < PAR_Rank()-1 ; i++)
        myStart += numPerProc[i];

    int *selForDomsIn = new int[numTotal];
    int *selForDoms   = new int[numTotal];
    for (i = 0 ; i < doms.size() ; i++)
        selForDomsIn[myStart+i] = doms[i];
    SumIntArrayAcrossAllProcessors(selForDomsIn, selForDoms, numTotal);

    int *selForZonesIn = new int[numTotal];
    int *selForZones   = new int[numTotal];
    for (i = 0 ; i < zones.size() ; i++)
        selForZonesIn[myStart+i] = zones[i];
    SumIntArrayAcrossAllProcessors(selForZonesIn, selForZones, numTotal);

    //
    // Now construct the actual named selection and add it to our internal
    // data structure for tracking named selections.
    //
    ns = new avtZoneIdNamedSelection(selName,numTotal,selForDoms,selForZones);
    DeleteNamedSelection(selName, false); // Remove sel if it already exists.
    int curSize = selList.size();
    selList.resize(curSize+1);
    selList[curSize] = ns;

    delete [] numPerProcIn;
    delete [] numPerProc;
    delete [] selForDomsIn;
    delete [] selForDoms;
    delete [] selForZonesIn;
    delete [] selForZones;

    //
    // Save out the named selection in case of engine crash / 
    // save/restore session, etc.
    //  
    SaveNamedSelection(selName, true);
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
    int i;

    int numToRemove = -1;
    for (i = 0 ; i < selList.size() ; i++)
    {
        if (selList[i]->GetName() == name)
        {
            numToRemove = i;
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
    for (i = 0 ; i < numToRemove ; i++)
        newList[i] = selList[i];
    for (i = numToRemove+1 ; i < selList.size() ; i++)
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

    int curSize = selList.size();
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
    for (int i = 0 ; i < selList.size() ; i++)
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
