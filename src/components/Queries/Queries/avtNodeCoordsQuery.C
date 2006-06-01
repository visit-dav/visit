/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtNodeCoordsQuery.C                           //
// ************************************************************************* //

#include <avtNodeCoordsQuery.h>
#include <avtTerminatingSource.h>
#include <avtSILRestrictionTraverser.h>
#include <vector>
#include <snprintf.h>
#include <avtParallel.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtNodeCoordsQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtNodeCoordsQuery::avtNodeCoordsQuery() : avtDatasetQuery() 
{
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtNodeCoordsQuery::~avtNodeCoordsQuery() 
{
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery::PerformQuery
//
//  Purpose:
//    Perform the requested query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Kathleen Bonnell, Tue Jul  6 15:20:56 PDT 2004 
//    Removed MPI calls, use GetFloatArrayToRootProc. 
//
//    Kathleen Bonnell, Thu Dec 16 17:16:33 PST 2004 
//    Moved code that actually finds zone center to FindLocalCenter and
//    FindGlobalCenter. 
//
//    Kathleen Bonnell, Tue Dec 28 14:52:22 PST 2004 
//    Add 'global' to output string as necessary. 
//
// ****************************************************************************

void
avtNodeCoordsQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    UpdateProgress(0, 0);

    bool singleDomain = false;
    if (!queryAtts.GetUseGlobalId())
    {
        intVector dlist;
        avtDataSpecification_p dspec = 
            GetInput()->GetTerminatingSource()->GetFullDataSpecification();
        dspec->GetSIL().GetDomainList(dlist);

        if (dlist.size() == 1 && dspec->UsesAllDomains())
        {
            singleDomain = true;
        }
    }
    else
    {
        singleDomain = true;
    }


    double coord[3] = {0., 0., 0.};

    bool success;
    if (queryAtts.GetUseGlobalId())
    {
        success = FindGlobalCoord(coord);
    }
    else 
    {
        success = FindLocalCoord(coord);
    }
    
    GetDoubleArrayToRootProc(coord, 3, success);

    if (PAR_Rank() != 0)
        return;

    char msg[120];

    if (success)
    {
        int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
        if (singleDomain)
        {
            string global;
            if (qA->GetUseGlobalId())
                global = "global";
            if (dim == 2)
            {
                SNPRINTF(msg, 120, "The coords of %s node %d are (%g, %g).", 
                         global.c_str(), qA->GetElement(), coord[0], coord[1]);
            }
            else 
            {
                SNPRINTF(msg, 120, "The coords of %s node %d are (%g, %g, %g).", 
                         global.c_str(), qA->GetElement(), 
                         coord[0], coord[1], coord[2]);
            }
        }
        else
        {
            avtTerminatingSource *src = GetInput()->GetTerminatingSource();
            int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
            int domain      = qA->GetDomain()  - blockOrigin;
            int ts          = qA->GetTimeStep();
            string var      = qA->GetVariables()[0];
            string domainName;
            src->GetDomainName(var, ts, domain, domainName);
            if (dim == 2)
            {
                SNPRINTF(msg, 120, "The coords of node %d (%s) are (%g, %g).", 
                         qA->GetElement(), domainName.c_str(),
                         coord[0], coord[1]);
            }
            else 
            {
                SNPRINTF(msg, 120, "The coords of node %d (%s) are (%g, %g, %g).", 
                         qA->GetElement(), domainName.c_str(),
                         coord[0], coord[1], coord[2]);
            }
        }
  
        doubleVector c;
        c.push_back((double)coord[0]);
        c.push_back((double)coord[1]);
        c.push_back((double)coord[2]);
        qA->SetResultsValue(c);
    }
    else
    {
        if (singleDomain)
        {
            SNPRINTF(msg, 120, "The coords of node %d could not be determined.",
                     qA->GetElement());
        }
        else
        {
            avtTerminatingSource *src = GetInput()->GetTerminatingSource();
            int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
            int domain      = qA->GetDomain()  - blockOrigin;
            int ts          = qA->GetTimeStep();
            string var      = qA->GetVariables()[0];
            string domainName;
            src->GetDomainName(var, ts, domain, domainName);
            SNPRINTF(msg, 120, "The coords of node %d (%s) could not be determined.",
                     qA->GetElement(), domainName.c_str());
        }
    }

    qA->SetResultsMessage(msg);
    UpdateProgress(1, 0);
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery::FindLocalCoord
//
//  Purpose:
//    Find the coordinats of node specified in queryAtts in domain specified
//    by queryAtts. 
//
//  Returns:
//    true upon succesful location of node and determination of its 
//    coordinates, false otherwise.
//
//  Arguments:
//    coord     A place to store the coordinates of the node.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 16, 2004 (moved from method PerformQuery)
//
//  Modifications:
//
// ****************************************************************************

bool
avtNodeCoordsQuery::FindLocalCoord(double coord[3])
{
    intVector dlist;

    int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
    int domain      = queryAtts.GetDomain()  - blockOrigin;
    int node        = queryAtts.GetElement(); 
    int ts          = queryAtts.GetTimeStep();
    string var      = queryAtts.GetVariables()[0];

    coord[0] = 0.;
    coord[1] = 0.;
    coord[2] = 0.;

    domain = (domain < 0 ? 0 : domain);

    avtSILRestrictionTraverser trav(querySILR);
    trav.GetDomainList(dlist);
    bool success = false;

    //
    //  See if any processor is working with this domain.
    //
    intVector dAllProc;
    trav.GetDomainListAllProcs(dAllProc);
    bool domainUsed = false;
    for (int j = 0; j < dAllProc.size() && !domainUsed; j++)
    {
        if (dAllProc[j] == domain)
            domainUsed = true;
    }

    avtTerminatingSource *src = GetInput()->GetTerminatingSource();
    if (domainUsed)
    {
        for (int i = 0; i < dlist.size() && !success; ++i) 
        {
            if (dlist[i] == domain)
            {
                success = src->QueryCoords(var, domain, node, ts, coord, false, false);
            }
        }
    }
    else if (PAR_Rank() == 0)
    {
        success = src->QueryCoords(var, domain, node, ts, coord, false, false);
    }
    return success; 
}


// ****************************************************************************
//  Method: avtNodeCoordsQuery::FindGlobalCoord
//
//  Purpose:
//    Find the coordinates of global node specified in queryAtts.  Must search 
//    all domains.
//
//  Returns:
//    true upon succesful location of node and determination of its 
//    coordinates, false otherwise.
//
//  Arguments:
//    coord     A place to store the coordinates of the node.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 16, 2004 
//
//  Modifications:
//
// ****************************************************************************

bool
avtNodeCoordsQuery::FindGlobalCoord(double coord[3])
{
    int node        = queryAtts.GetElement(); 
    int ts          = queryAtts.GetTimeStep();
    string var      = queryAtts.GetVariables()[0];

    coord[0] = 0.;
    coord[1] = 0.;
    coord[2] = 0.;

    intVector dlist;
    avtSILRestrictionTraverser trav(querySILR);
    trav.GetDomainList(dlist);
    bool success = false;

    avtTerminatingSource *src = GetInput()->GetTerminatingSource();
    for (int i = 0; i < dlist.size() && !success; ++i) 
    {
        success = src->QueryCoords(var, dlist[i], node, ts, coord, false, true);
    }
    return success;
}

