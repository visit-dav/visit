/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <visitstream.h>
#include <VisItException.h>


// ****************************************************************************
//  Method: avtNamedSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtNamedSelection::avtNamedSelection(const std::string &n)
{
    name = n;
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
//  Method: avtZoneIdNamedSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
// ****************************************************************************

avtZoneIdNamedSelection::avtZoneIdNamedSelection(const std::string &n)
    : avtNamedSelection(n)
{
}


// ****************************************************************************
//  Method: avtZoneIdNamedSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2009
//
// ****************************************************************************

avtZoneIdNamedSelection::avtZoneIdNamedSelection(const std::string &n, int num,
                                                 const int *dom, const int *zon)
    : avtNamedSelection(n)
{
    domId.resize(num);
    zoneId.resize(num);
    for (int i = 0 ; i < num ; i++)
    {
        domId[i]  = dom[i];
        zoneId[i] = zon[i];
    }
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
// ****************************************************************************

void
avtZoneIdNamedSelection::Write(const std::string &fname)
{
    ofstream ofile(fname.c_str());
    ofile << ZONE_ID << endl;
    ofile << domId.size() << endl;
    for (int i = 0 ; i < domId.size() ; i++)
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
//  Method: avtZoneIdNamedSelection::GetDomainList
//
//  Purpose:
//      Gets the list of domains from the identifiers in this selection.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2009
//
// ****************************************************************************

bool
avtZoneIdNamedSelection::GetDomainList(std::vector<int> &domains)
{
    int  i;

    int maxDomain = 0;
    for (i = 0 ; i < domId.size() ; i++)
    {
        if (domId[i] > maxDomain)
            maxDomain = domId[i];
    }

    std::vector<bool> useDomain(maxDomain+1, false);
    int numDoms = 0;
    for (i = 0 ; i < domId.size() ; i++)
    {
        if (! useDomain[domId[i]])
        {
            useDomain[domId[i]] = true;
            numDoms++;
        }
    }

    domains.resize(numDoms);
    int curDom = 0;
    for (i = 0 ; i < maxDomain+1 ; i++)
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
// ****************************************************************************

void
avtZoneIdNamedSelection::GetMatchingIds(unsigned int *pts, int nvals, 
                                        std::vector<int> &ids)
{
    if (lookupSet.size() == 0)
    {
        for (int i = 0 ; i < domId.size() ; i++)
        {
            IntPair ip;
            ip.d = domId[i];
            ip.z = zoneId[i];
            lookupSet.insert(ip);
        }
    }

    for (int i = 0 ; i < nvals ; i++)
    {
        IntPair ip;
        ip.d = (int) pts[2*i];
        ip.z = (int) pts[2*i+1];
        std::set<IntPair>::iterator it;
        it = lookupSet.find(ip);
        if (it != lookupSet.end())
            ids.push_back(i);
    }
}


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
// ****************************************************************************

void
avtFloatingPointIdNamedSelection::Write(const std::string &fname)
{
    ofstream ofile(fname.c_str());
    ofile << FLOAT_ID << endl;
    ofile << ids.size() << endl;
    for (int i = 0 ; i < ids.size() ; i++)
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


