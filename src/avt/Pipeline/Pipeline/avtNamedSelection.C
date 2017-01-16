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
//                             avtNamedSelection.C                           //
// ************************************************************************* //

#include <avtNamedSelection.h>

#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;

#include <avtIdentifierSelection.h>
#include <avtParallel.h>

#include <visitstream.h>
#include <VisItException.h>
#include <InvalidVariableException.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkVisItCellLocator.h>
#include <vtkGenericCell.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtNamedSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtNamedSelection::avtNamedSelection(const std::string &n) : name(n), idVar()
{
}

// ****************************************************************************
//  Method: avtNamedSelection destructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtNamedSelection::~avtNamedSelection()
{
}

// ****************************************************************************
// Method: avtNamedSelection::SetIdVariable
//
// Purpose: 
//   Set the id variable that we're using for the named selection.
//
// Arguments:
//   id : The new id variable.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:40:04 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
avtNamedSelection::SetIdVariable(const std::string &id)
{
    idVar = id;
}

// ****************************************************************************
// Method: avtNamedSelection::GetIdVariable
//
// Purpose: 
//   Get the id variable used in this named selection.
//
// Returns:    The id variable.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:40:30 PST 2011
//
// Modifications:
//   
// ****************************************************************************

const std::string &
avtNamedSelection::GetIdVariable() const
{
    return idVar;
}

// ****************************************************************************
// Method: avtNamedSelection::MaximumSelectionSize
//
// Purpose: 
//   Return the largest allowable selection size.
//
// Returns:    The largest selection size.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:40:56 PST 2011
//
// Modifications:
//   
// ****************************************************************************

int
avtNamedSelection::MaximumSelectionSize()
{
    return 50000000;
}

// ****************************************************************************
// Method:  avtNamedSelection::CheckValid
//
// Purpose: Validate that a selection can be done on this dataset.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

bool
avtNamedSelection::CheckValid(vtkDataSet *ds)
{
    return GetIDArray(ds) != NULL;
}

// ****************************************************************************
// Method:  avtNamedSelection::GetIDArray
//
// Purpose: Pick off ID array from dataset, or NULL if not found.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

vtkDataArray *
avtNamedSelection::GetIDArray(vtkDataSet *ds)
{
    vtkDataArray *arr = NULL;
    
    arr = ds->GetCellData()->GetArray(GetIdVariable().c_str());
    if(arr == NULL && ds->GetNumberOfCells() == ds->GetNumberOfPoints())
        arr = ds->GetPointData()->GetArray(GetIdVariable().c_str());
    
    if (arr == NULL)
        return NULL;
    else if (arr->GetVoidPointer(0) == NULL)
        return NULL;

    return arr;
}

// ****************************************************************************
// Method:  avtNamedSelection::GetMatchingLocations
//
// Purpose:
//   
// Programmer:  Dave Pugmire
// Creation:    March 14, 2012
//
// ****************************************************************************

void
avtNamedSelection::GetMatchingLocations(std::vector<avtVector> &)
{
    EXCEPTION1(VisItException, "Locations not supported");
}

// ****************************************************************************
// ****************************************************************************
// ***
// *** avtZoneIdNamedSelection
// ***
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//  Method: avtZoneIdNamedSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtZoneIdNamedSelection::avtZoneIdNamedSelection(const std::string &n)
    : avtNamedSelection(n)
{
    SetIdVariable("avtOriginalCellNumbers");
}

// ****************************************************************************
//  Method: avtZoneIdNamedSelection destructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtZoneIdNamedSelection::~avtZoneIdNamedSelection()
{
}


// ****************************************************************************
//  Method: avtZoneIdNamedSelection::Write
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//    Kathleen Bonnell, Thur Mar 26 08:16:23 MST 2009
//    Use size_t for loop variable to remove signed/unsigned int comparison
//    warnings.
//
// ****************************************************************************

void
avtZoneIdNamedSelection::Write(const std::string &fname)
{
    ofstream ofile(fname.c_str());
    ofile << ZONE_ID << endl;
    ofile << domId.size() << endl;
    for (size_t i = 0 ; i < domId.size() ; i++)
    {
        ofile << domId[i] << " " << zoneId[i] << endl;
    }
}

// ****************************************************************************
//  Method: avtZoneIdNamedSelection::Read
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

void
avtZoneIdNamedSelection::Read(const std::string &fname)
{
    ifstream ifile(fname.c_str());
    if (ifile.fail())
    {
        EXCEPTION1(VisItException, "Cannot read named selection");
    }

    int nsType;
    ifile >> nsType;
    if (nsType != ZONE_ID)
    {
        EXCEPTION1(VisItException, "Internal error reading named selection");
    }

    int nvals;
    ifile >> nvals;
    if (nvals < 0)
    {
        EXCEPTION1(VisItException, "Invalid named selection");
    }
  
    domId.resize(nvals);
    zoneId.resize(nvals);
    for (int i = 0 ; i < nvals ; i++)
    {
        ifile >> domId[i];
        ifile >> zoneId[i];
    }
}

// ****************************************************************************
// Method: avtZoneIdNamedSelection::ModifyContract
//
// Purpose: 
//   Returns a new version of the contract, including any changes this named
//   selection requires.
//
// Arguments:
//   contract : The input contract.
//
// Returns:    A modified contract.
//
// Note:       The output contract turns on the original zone numbers and
//             tries to do some domain restriction.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:29:29 PST 2011
//
// Modifications:
//   
// ****************************************************************************

avtContract_p
avtZoneIdNamedSelection::ModifyContract(avtContract_p contract) const
{
    avtContract_p rv = new avtContract(contract);
    rv->GetDataRequest()->TurnZoneNumbersOn();

    std::vector<int> domains;
    if (GetDomainList(domains))
    {
        rv->GetDataRequest()->GetRestriction()->RestrictDomains(domains);
    }

    return rv;
}

// ****************************************************************************
//  Method: avtZoneIdNamedSelection::CreateSelection
//
//  Purpose:
//      Creates an avtDataSelection for the database to read in only the data
//      corresponding to this selection.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2009
//
//  Modifications:
//    Brad Whitlock, Thu Mar 15 14:15:13 PDT 2012
//    Store the id variable in the data selection.
//
// ****************************************************************************

avtDataSelection *
avtZoneIdNamedSelection::CreateSelection(void)
{
    avtIdentifierSelection *rv = new avtIdentifierSelection;
    rv->SetIdVariable(GetIdVariable());

    // NOTE: the domains are not sent.
    std::vector<double> ids(zoneId.size());    
    for (size_t i = 0 ; i < zoneId.size() ; i++)
      ids[i] = zoneId[i];
    
    rv->SetIdentifiers(ids);
    return rv;
}


// ****************************************************************************
//  Method: avtZoneIdNamedSelection::GetDomainList
//
//  Purpose:
//      Gets the list of domains from the identifiers in this selection.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2009
//
//  Modifications:
//    Kathleen Bonnell, Thur Mar 26 08:16:23 MST 2009
//    Use size_t for loop variable to remove signed/unsigned int comparison
//    warnings.
//
// ****************************************************************************

bool
avtZoneIdNamedSelection::GetDomainList(std::vector<int> &domains) const
{
    int maxDomain = 0;
    for (size_t i = 0 ; i < domId.size() ; i++)
    {
        if (domId[i] > maxDomain)
            maxDomain = domId[i];
    }

    std::vector<bool> useDomain(maxDomain+1, false);
    int numDoms = 0;
    for (size_t i = 0 ; i < domId.size() ; i++)
    {
        if (! useDomain[domId[i]])
        {
            useDomain[domId[i]] = true;
            numDoms++;
        }
    }

    domains.resize(numDoms);
    int curDom = 0;
    for (int i = 0 ; i < maxDomain+1 ; i++)
        if (useDomain[i])
            domains[curDom++] = i;

    return true;
}


// ****************************************************************************
//  Method: avtZoneIdNamedSelection::GetMatchingIds
//
//  Purpose:
//      Get the zone indices that match the named selection.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2009
//
//  Modifications:
//    Tom Fogal, Sat Feb  7 17:13:46 EST 2009
//    Fix declaration of iterator, and make it const.
//
//    Kathleen Bonnell, Thur Mar 26 08:16:23 MST 2009
//    Use size_t for loop variable to remove signed/unsigned int comparison
//    warnings.  Added PairCompare to iterator specification for compilation
//    on windows.
//
//    Brad Whitlock, Thu Oct 27 16:07:38 PDT 2011
//    I rewrote it to use vtkDataArray.
//
//    Dave Pugmire, Thu Mar 15 10:55:22 EDT 2012
//    Pass in vtkDataSet and use GetIDArray to get array.
//
// ****************************************************************************

void
avtZoneIdNamedSelection::GetMatchingIds(vtkDataSet *ds,
                                        std::vector<vtkIdType> &cellids)
{
    vtkDataArray *idData = GetIDArray(ds);
    if (idData == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    if (lookupSet.size() == 0)
    {
        for (size_t i = 0 ; i < domId.size() ; i++)
        {
            IntPair ip;
            ip.d = domId[i];
            ip.z = zoneId[i];
            lookupSet.insert(ip);
        }
    }

    vtkIdType nvals = idData->GetNumberOfTuples();
    unsigned int *ptr = (unsigned int *)idData->GetVoidPointer(0);
    if(ptr == NULL || idData->GetNumberOfComponents() != 2)
    {
        EXCEPTION0(ImproperUseException);
    }

    IntPair ip;
    std::set<IntPair, PairCompare>::const_iterator it;
    for (vtkIdType cellid = 0 ; cellid < nvals ; cellid++)
    {
        ip.d = (int) *ptr++;
        ip.z = (int) *ptr++;
        it = lookupSet.find(ip);
        if (it != lookupSet.end())
            cellids.push_back(cellid);
    }
}

// ****************************************************************************
// Method: avtZoneIdNamedSelection::Globalize
//
// Purpose: 
//   Combines the selections from each processor and makes sure they all have 
//   the same global selection.
//
// Programmer: Hank Childs
// Creation:   Fri Oct 28 09:36:39 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 09:46:26 PDT 2011
//   I moved the code from the NSM into this method so derived types can do
//   their own globalization step.
//
// ****************************************************************************

void
avtZoneIdNamedSelection::Globalize()
{
#ifdef PARALLEL
    // Note the poor use of MPI below, coded for expediency, as I believe all
    // of the named selections will be small.
    int *numPerProcIn = new int[PAR_Size()];
    int *numPerProc   = new int[PAR_Size()];
    for (int i = 0 ; i < PAR_Size() ; i++)
        numPerProcIn[i] = 0;
    numPerProcIn[PAR_Rank()] = domId.size();
    SumIntArrayAcrossAllProcessors(numPerProcIn, numPerProc, PAR_Size());
    int numTotal = 0;
    for (int i = 0 ; i < PAR_Size() ; i++)
        numTotal += numPerProc[i];
    if (numTotal > MaximumSelectionSize())
    {
        delete [] numPerProcIn;
        delete [] numPerProc;
        EXCEPTION1(VisItException, "You have selected too many zones in your "
                   "named selection.  Disallowing ... no selection created");
    }

    int myStart = 0;
    for (int i = 0 ; i < PAR_Rank()-1 ; i++)
        myStart += numPerProc[i];
    delete [] numPerProcIn;
    delete [] numPerProc;

    int *buffer = new int[numTotal];
    memset(buffer, 0, sizeof(int) * numTotal);
    for (size_t i = 0 ; i < domId.size() ; i++)
        buffer[myStart+i] = domId[i];
    domId.resize(numTotal);
    SumIntArrayAcrossAllProcessors(buffer, &domId[0], numTotal);

    memset(buffer, 0, sizeof(int) * numTotal);
    for (size_t i = 0 ; i < zoneId.size() ; i++)
        buffer[myStart+i] = zoneId[i];
    zoneId.resize(numTotal);
    SumIntArrayAcrossAllProcessors(buffer, &zoneId[0], numTotal);
    delete [] buffer;
#endif
}

// ****************************************************************************
// Method: avtZoneIdNamedSelection::Allocate
//
// Purpose: 
//   Make sure that there is enough room in the internal vectors that we're
//   going to use to store the selection.
//
// Arguments:
//   nvals : The largest selection size we'll need.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:42:41 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
avtZoneIdNamedSelection::Allocate(size_t nvals)
{
    domId.clear();
    domId.reserve(nvals);

    zoneId.clear();
    zoneId.reserve(nvals);
}

// ****************************************************************************
// Method: avtZoneIdNamedSelection::Append
//
// Purpose: 
//   Append the "ids" from the input array into the selection.
//
// Arguments:
//   arr : The array that contains the ids we're adding to the selection.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:43:32 PST 2011
//
// Modifications:
//
//    Dave Pugmire, Thu Mar 15 10:55:22 EDT 2012
//    Pass in vtkDataSet and use GetIDArray to get array.
//   
// ****************************************************************************

void
avtZoneIdNamedSelection::Append(vtkDataSet *ds)
{
    vtkDataArray *arr = GetIDArray(ds);
    if (arr == NULL)
        return;

    vtkIdType nvals = arr->GetNumberOfTuples();
    unsigned int *ptr = (unsigned int *)arr->GetVoidPointer(0);

    // We have original cell numbers so add them to the selection.
    for (vtkIdType j = 0 ; j < nvals ; j++)
    {
        domId.push_back(*ptr++);
        zoneId.push_back(*ptr++);
    }
}

// ****************************************************************************
// Method: avtZoneIdNamedSelection::SetIdentifiers
//
// Purpose: 
//   Set the selection's ids from external arrays.
//
// Arguments:
//   nvals : The new number of ids.
//   doms  : The domain ids.
//   zones : The zone ids.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:44:19 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
avtZoneIdNamedSelection::SetIdentifiers(int nvals, const int *doms, const int *zones)
{
    domId.resize(nvals);
    zoneId.resize(nvals);
    memcpy(&domId[0], doms, nvals * sizeof(int));
    memcpy(&zoneId[0], zones, nvals * sizeof(int));
}

// ****************************************************************************
// ****************************************************************************
// ***
// *** avtFloatingPointIdNamedSelection
// ***
// ****************************************************************************
// ****************************************************************************

// ****************************************************************************
//  Method: avtFloatingPointIdNamedSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtFloatingPointIdNamedSelection::avtFloatingPointIdNamedSelection(
                                                          const std::string &n)
    : avtNamedSelection(n)
{
}

// ****************************************************************************
//  Method: avtFloatingPointIdNamedSelection destructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtFloatingPointIdNamedSelection::~avtFloatingPointIdNamedSelection()
{
}

// ****************************************************************************
//  Method: avtFloatingPointIdNamedSelection::Write
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//    Kathleen Bonnell, Thur Mar 26 08:16:23 MST 2009
//    Use size_t for loop variable to remove signed/unsigned int comparison
//    warnings.
//
//    Gunther H. Weber, Mon Apr  6 20:17:40 PDT 2009
//    Increased precision for floating point output.
//
//    Brad Whitlock, Fri Oct 28 10:17:49 PDT 2011
//    Write the name of the id variable.
//
// ****************************************************************************

void
avtFloatingPointIdNamedSelection::Write(const std::string &fname)
{
    // Sort the ids so they look nice in the output.
    std::sort(ids.begin(), ids.end());

    ofstream ofile(fname.c_str());
    ofile << FLOAT_ID << endl;
    ofile << GetIdVariable() << endl;
    ofile << ids.size() << endl;
    ofile << std::setprecision(32);
    for (size_t i = 0 ; i < ids.size() ; i++)
    {
        ofile << ids[i] << endl;
    }
}

// ****************************************************************************
//  Method: avtFloatingPointIdNamedSelection::Read
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//    Brad Whitlock, Fri Oct 28 10:17:20 PDT 2011
//    Read the name of the id variable.
//
// ****************************************************************************

void
avtFloatingPointIdNamedSelection::Read(const std::string &fname)
{
    ifstream ifile(fname.c_str());
    if (ifile.fail())
    {
        EXCEPTION1(VisItException, "Cannot read named selection");
    }

    int nsType;
    ifile >> nsType;
    if (nsType != FLOAT_ID)
    {
        EXCEPTION1(VisItException, "Internal error reading named selection");
    }

    std::string id;
    ifile >> id;
    SetIdVariable(id);

    int nvals;
    ifile >> nvals;
    if (nvals < 0)
    {
        EXCEPTION1(VisItException, "Invalid named selection");
    }
  
    ids.resize(nvals);
    for (int i = 0 ; i < nvals ; i++)
    {
        ifile >> ids[i];
    }
}

// ****************************************************************************
// Method: avtFloatingPointIdNamedSelection::ModifyContract
//
// Purpose: 
//   Returns a new version of the contract, including any changes this named
//   selection requires.
//
// Arguments:
//   contract : The input contract.
//
// Returns:    A modified contract.
//
// Note:       The output contract turns on the original zone numbers and
//             tries to do some domain restriction.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:29:29 PST 2011
//
// Modifications:
//   
// ****************************************************************************

avtContract_p
avtFloatingPointIdNamedSelection::ModifyContract(avtContract_p contract) const
{
    avtContract_p rv = new avtContract(contract);

    if(idVar == "avtGlobalZoneNumbers")
    {
        rv->GetDataRequest()->TurnGlobalZoneNumbersOn();
    }
    else
    {
        // Make sure that we request the id variable.
        if(!rv->GetDataRequest()->HasSecondaryVariable(idVar.c_str()))
        {
            rv->GetDataRequest()->AddSecondaryVariable(idVar.c_str());
        }
    }

    return rv;
}

// ****************************************************************************
//  Method: avtFloatingPointIdNamedSelection::CreateSelection
//
//  Purpose:
//      Creates an avtDataSelection for the database to read in only the data
//      corresponding to this selection.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2009
//
//  Modifications:
//    Brad Whitlock, Thu Mar 15 14:15:13 PDT 2012
//    Store the id variable in the data selection.
//
// ****************************************************************************

avtDataSelection *
avtFloatingPointIdNamedSelection::CreateSelection(void)
{
    avtIdentifierSelection *rv = new avtIdentifierSelection;
    rv->SetIdVariable(GetIdVariable());
    rv->SetIdentifiers(ids);
    return rv;
}


// ****************************************************************************
//  Method: avtFloatingPointIdNamedSelection::CreateConditionString
//
//  Purpose:
//      Creates a condition string that can be fed to the database.
//
//  Notes:      This is probably a bad design, because it assumes FastBit.
//              This should be revisited soon.
//
//  Programmer: Hank Childs
//  Creation:   April 6, 2009
//
// Modifications:
//   Brad Whitlock, Mon Dec 12 15:03:34 PST 2011
//   Don't hard code the id variable.
//
// ****************************************************************************

static std::string
stringify(double x)
{
    std::ostringstream o;
    if (!(o << setprecision(32) << x))
    {
        EXCEPTION1(InvalidVariableException, "string conversion");
    }
    return o.str();
}

std::string
avtFloatingPointIdNamedSelection::CreateConditionString(void)
{
    std::string condition;
    if (!ids.empty())
    {
        // convert all the identifiers into a string...
        std::string id_string(std::string("( ") + idVar + std::string(" in ( "));

        for (size_t j=0; j<ids.size()-1; j++)
            id_string = id_string + stringify(ids[j]) + ", ";
        id_string = id_string + stringify(ids[ids.size()-1]) + " ))";

        condition = id_string;
    }
    return condition;
}

// ****************************************************************************
// Method: avtFloatingPointIdNamedSelection::GetMatchingIds
//
// Purpose: 
//   Return the cell ids in the var array that match the values stored in the 
//   selection.
//
// Arguments:
//   var : The input id array.
//   ids : The output vector of indices that match values from the selection.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:22:59 PDT 2011
//
// Modifications:
//
//    Dave Pugmire, Thu Mar 15 10:55:22 EDT 2012
//    Pass in vtkDataSet and use GetIDArray to get array.
//   
// ****************************************************************************

void
avtFloatingPointIdNamedSelection::GetMatchingIds(vtkDataSet *ds,
                                                 std::vector<vtkIdType> &cellids)
{
    vtkDataArray *idData = GetIDArray(ds);
    if (idData == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    // Convert ids to a set so lookups will be faster.
    std::set<double> lookupSet;
    for (size_t i = 0 ; i < ids.size() ; i++)
        lookupSet.insert(ids[i]);

    vtkIdType nvals = idData->GetNumberOfTuples();
    std::set<double>::const_iterator it;
    for (vtkIdType cellid = 0 ; cellid < nvals ; cellid++)
    {
        it = lookupSet.find(idData->GetTuple1(cellid));
        if (it != lookupSet.end())
            cellids.push_back(cellid);
    }
}

// ****************************************************************************
// Method: avtFloatingPointIdNamedSelection::Globalize
//
// Purpose: 
//   Combines the selections from each processor and makes sure they all have 
//   the same global selection.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 28 09:36:39 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
avtFloatingPointIdNamedSelection::Globalize()
{
#ifdef PARALLEL
    // Note the poor use of MPI below, coded for expediency, as I believe all
    // of the named selections will be small.
    int *numPerProcIn = new int[PAR_Size()];
    int *numPerProc   = new int[PAR_Size()];
    for (int i = 0 ; i < PAR_Size() ; i++)
        numPerProcIn[i] = 0;
    numPerProcIn[PAR_Rank()] = ids.size();
    SumIntArrayAcrossAllProcessors(numPerProcIn, numPerProc, PAR_Size());
    int numTotal = 0;
    for (int i = 0 ; i < PAR_Size() ; i++)
        numTotal += numPerProc[i];
    if (numTotal > MaximumSelectionSize())
    {
        delete [] numPerProcIn;
        delete [] numPerProc;
        EXCEPTION1(VisItException, "You have selected too many zones in your "
                   "named selection.  Disallowing ... no selection created");
    }

    int myStart = 0;
    for (int i = 0 ; i < PAR_Rank()-1 ; i++)
        myStart += numPerProc[i];
    delete [] numPerProcIn;
    delete [] numPerProc;

    double *buffer = new double[numTotal];
    memset(buffer, 0, sizeof(double) * numTotal);
    for (size_t i = 0 ; i < ids.size() ; i++)
        buffer[myStart+i] = ids[i];
    ids.resize(numTotal);
    SumDoubleArrayAcrossAllProcessors(buffer, &ids[0], numTotal);
    delete [] buffer;
#endif
}

// ****************************************************************************
// Method: avtFloatingPointIdNamedSelection::Allocate
//
// Purpose: 
//   Make sure that the internal ids vector has enough entries.
//
// Arguments:
//   nvals : The new number of ids we'll be adding.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:45:27 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
avtFloatingPointIdNamedSelection::Allocate(size_t nvals)
{
    ids.clear();
    ids.reserve(nvals);
}

// ****************************************************************************
// Method: avtFloatingPointIdNamedSelection::Append
//
// Purpose: 
//   Append the "ids" in the data array to the selection.
//
// Arguments:
//   arr : The array that contains the "ids" we're adding to the selection.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:46:01 PST 2011
//
// Modifications:
//
//    Dave Pugmire, Thu Mar 15 10:55:22 EDT 2012
//    Pass in vtkDataSet and use GetIDArray to get array.
//   
// ****************************************************************************

void
avtFloatingPointIdNamedSelection::Append(vtkDataSet *ds)
{
    vtkDataArray *idData = GetIDArray(ds);
    if (idData == NULL)
        return;

    vtkIdType nvals = idData->GetNumberOfTuples();

    // We have our ids so add them to the selection.
    for (vtkIdType j = 0 ; j < nvals ; j++)
        ids.push_back(idData->GetTuple1(j));
}

// ****************************************************************************
// Method: avtFloatingPointIdNamedSelection::SetIdentifiers
//
// Purpose: 
//   Set the selection ids from an external vector.
//
// Arguments:
//   vals : The new selection ids.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov  7 13:46:38 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
avtFloatingPointIdNamedSelection::SetIdentifiers(const std::vector<double> &vals)
{
    ids = vals;
}


// ****************************************************************************
// ****************************************************************************
// ***
// *** avtLocationsNamedSelection
// ***
// ****************************************************************************
// ****************************************************************************


// ****************************************************************************
// Method:  avtLocationsNamedSelection::avtLocationsNamedSelection
//
// Purpose: Locations named selection constructor.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************


avtLocationsNamedSelection::avtLocationsNamedSelection(const std::string &n) : avtNamedSelection(n)
{
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::~avtLocationsNamedSelection
//
// Purpose: Locations named selection destructor.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

avtLocationsNamedSelection::~avtLocationsNamedSelection()
{
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::Read
//
// Purpose: Restore a locations named selection.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

void
avtLocationsNamedSelection::Read(const std::string &fname)
{
    ifstream ifile(fname.c_str());
    if (ifile.fail())
    {
        EXCEPTION1(VisItException, "Cannot read named selection");
    }

    int nsType;
    ifile>>nsType;
    if (nsType != LOCATIONS)
    {
        EXCEPTION1(VisItException, "Internal error reading named selection");
    }

    int nvals;
    ifile>>nvals;
    if (nvals < 0)
    {
        EXCEPTION1(VisItException, "Invalid named selection");
    }
  
    locations.resize(nvals);
    for (int i = 0; i < nvals; i++)
    {
        ifile>>locations[i].x;
        ifile>>locations[i].y;
        ifile>>locations[i].z;
    }
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::Write
//
// Purpose: Save off a locations named selection.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

void
avtLocationsNamedSelection::Write(const std::string &fname)
{
    ofstream ofile(fname.c_str());
    ofile<<GetType()<<endl;
    ofile<<locations.size()<<endl;
    for (size_t i = 0; i < locations.size(); i++)
    {
        ofile<<locations[i].x<<endl;
        ofile<<locations[i].y<<endl;
        ofile<<locations[i].z<<endl;
    }
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::ModifyContract
//
// Purpose: Save off a locations named selection.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

avtContract_p
avtLocationsNamedSelection::ModifyContract(avtContract_p c0) const
{
    return c0;
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::Allocate
//
// Purpose: Reserve space.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

void
avtLocationsNamedSelection::Allocate(size_t nvals)
{
    locations.clear();
    locations.reserve(nvals);
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::CheckValid
//
// Purpose: Validate that a selection can be done on this dataset.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

bool
avtLocationsNamedSelection::CheckValid(vtkDataSet *ds)
{
    return ds->GetNumberOfPoints() > 0;
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::Append
//
// Purpose: Add dataset to the selection.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

void
avtLocationsNamedSelection::Append(vtkDataSet *ds)
{
    vtkIdType nPts = ds->GetNumberOfPoints();
    double pt[3];
    for (vtkIdType i = 0; i < nPts; i++)
    {
        ds->GetPoint(i, pt);
        locations.push_back(avtVector(pt[0], pt[1], pt[2]));
    }
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::GetMatchingIds
//
// Purpose: Get cells coresponding to this selection. For locations, this returns
//          cells that contain the locations.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

void
avtLocationsNamedSelection::GetMatchingIds(vtkDataSet *ds, std::vector<vtkIdType> &ids)
{
    vtkVisItCellLocator *loc = vtkVisItCellLocator::New();
    loc->SetDataSet(ds);
    loc->CacheCellBoundsOn();
    loc->AutomaticOn();
    loc->BuildLocator();

    double p[3], cp[3], dist;
    int subId;
    vtkIdType cellId;
    
    vtkGenericCell *cell = vtkGenericCell::New();
    size_t nLocs = locations.size();
    std::map<vtkIdType, int> idMap;

    //Use a map to ensure we don't get duplicate cells.
    for (size_t i = 0; i < nLocs; i++)
    {
        p[0] = locations[i].x;
        p[1] = locations[i].y;
        p[2] = locations[i].z;
        if (loc->FindClosestPointWithinRadius(p, 1e-8, cp, cell, cellId, subId, dist))
        {
            if (idMap.find(cellId) == idMap.end())
                idMap[cellId] = 1;
            continue;
        }
    }
    
    std::map<vtkIdType, int>::const_iterator it;
    for (it = idMap.begin(); it != idMap.end(); it++)
        ids.push_back((*it).first);
    
    cell->Delete();
    loc->Delete();
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::Globalize
//
// Purpose: Sync selection when running in parallel.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

void
avtLocationsNamedSelection::Globalize()
{
#ifdef PARALLEL
    int *numPerProcIn = new int[PAR_Size()];
    int *numPerProc   = new int[PAR_Size()];
    for (int i = 0 ; i < PAR_Size() ; i++)
        numPerProcIn[i] = 0;
    numPerProcIn[PAR_Rank()] = locations.size();
    SumIntArrayAcrossAllProcessors(numPerProcIn, numPerProc, PAR_Size());
    
    int numTotalLocs = 0;
    for (int i = 0 ; i < PAR_Size() ; i++)
        numTotalLocs += numPerProc[i];
    if (numTotalLocs > MaximumSelectionSize())
    {
        delete [] numPerProcIn;
        delete [] numPerProc;
        EXCEPTION1(VisItException, "You have selected too many locations in your "
                   "named selection.  Disallowing ... no selection created");
    }

    int myStart = 0;
    for (int i = 0; i < PAR_Rank(); i++)
        myStart += numPerProc[i];
    delete [] numPerProcIn;
    delete [] numPerProc;

    int numTotalVals = numTotalLocs*3;

    double *buffer = new double[numTotalVals], *bufferOut = new double[numTotalVals];
    for (int i = 0; i < numTotalVals; i++)
        buffer[i] = 0.0;
    for (size_t i = 0; i < locations.size(); i++)
    {
        buffer[(myStart+i)*3+0] = locations[i].x;
        buffer[(myStart+i)*3+1] = locations[i].y;
        buffer[(myStart+i)*3+2] = locations[i].z;
    }

    SumDoubleArrayAcrossAllProcessors(buffer, bufferOut, numTotalVals);
    delete [] buffer;

    locations.resize(numTotalLocs);
    for (int i = 0; i < numTotalLocs; i++)
    {
        locations[i].x = bufferOut[(i*3)+0];
        locations[i].y = bufferOut[(i*3)+1];
        locations[i].z = bufferOut[(i*3)+2];
    }

    delete [] bufferOut;

    
#endif
}

// ****************************************************************************
// Method:  avtLocationsNamedSelection::GetMatchingLocations
//
// Purpose: Return locations.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 15, 2012
//
// ****************************************************************************

void
avtLocationsNamedSelection::GetMatchingLocations(std::vector<avtVector> &pts)
{
    size_t nPts = locations.size();
    for (size_t i = 0; i < nPts; i++)
        pts.push_back(locations[i]);
}
