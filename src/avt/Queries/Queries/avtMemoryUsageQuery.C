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
//                         avtMemoryUsageQuery.C                             //
// ************************************************************************* //

#include <avtMemoryUsageQuery.h>

#include <avtParallel.h>
#include <snprintf.h>
#include <QueryAttributes.h>

#include <Utility.h>


#ifdef PARALLEL
#include <mpi.h>
#endif

// ****************************************************************************
//  Method: avtMemoryUsageQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the 
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   March 5, 2008
//
// ****************************************************************************

avtMemoryUsageQuery::avtMemoryUsageQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtMemoryUsageQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the 
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   March 5, 2008
//
// ****************************************************************************

avtMemoryUsageQuery::~avtMemoryUsageQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtMemoryUsageQuery::PerformQuery
//
//  Purpose:
//      Gets the memory usage for all processors.
//
//  Programmer: Cyrus Harrison
//  Creation:   March 5, 2008
//
//  Modifications:
//    Cyrus Harrison,Wed Apr  1 12:08:22 PDT 2009
//    Modified to support reporting up to 4 gigabytes per proc.
//
// ****************************************************************************

void
avtMemoryUsageQuery::PerformQuery(QueryAttributes *atts)
{
    // grab memory usage per engine process
    unsigned int m_size = 0;
    unsigned int m_rss  = 0;
    
    GetMemorySize(m_size,m_rss);
    
    if(m_size == 0 || m_rss == 0)
    {
        memSizeVals.clear();
        atts->SetResultsValue(memSizeVals);
        atts->SetResultsMessage("The Memory Usage Query is not supported on "
                                "this platform");
        return;
    }
    
    // convert to megabytes    
    double m_size_mb = ( (double)m_size / 1048576.0);
    double m_rss_mb  = ( (double)m_rss  / 1048576.0);
    
    int nprocs = PAR_Size();
    int rank   = PAR_Rank();

    memSizeVals.resize(nprocs);
    for (int i= 0; i < nprocs; i++)
        memSizeVals[i] = 0.0;

    memSizeVals[rank] = m_size_mb;

#ifdef PARALLEL
    // get values from other procs to the root
    if (nprocs > 1 )
    {
        MPI_Gather(&m_size_mb, 1, MPI_DOUBLE, 
                   &memSizeVals[0], 1, MPI_DOUBLE, 
                   0, VISIT_MPI_COMM);
    }
#endif

    atts->SetResultsValue(memSizeVals);
    queryAtts = *atts;
    std::string msg = GetResultMessage();
    atts->SetResultsMessage(msg);
}


// ****************************************************************************
//  Method: avtMemoryUsageQuery::GetResultMessage
//
//  Purpose:
//      Creates the result message.
//
//  Programmer: Cyrus Harrison
//  Creation:   March 5, 2008
//
//  Modifications:
//
// ****************************************************************************

std::string
avtMemoryUsageQuery::GetResultMessage()
{
    
    std::string msg = "";
    char msg_buff[1024];
    double total_mem_size= 0.0;
    for (int i=0;i< memSizeVals.size();i++)
        total_mem_size += memSizeVals[i];
    
    std::string float_format = queryAtts.GetFloatFormat();
    
    int nproc = memSizeVals.size();
    if (  nproc == 1 )
    {
        std::string format = "Engine Memory Usage = " 
                         + float_format + " megabytes.\n";
        SNPRINTF(msg_buff,1024,format.c_str(), total_mem_size);
        msg += msg_buff;
    }
    else
    {
        std::string format = "Total Engine Memory Usage = " 
                         + float_format + " megabytes.\n";
        SNPRINTF(msg_buff,1024,format.c_str(), total_mem_size);
        msg += msg_buff;
        for ( int i=0; i<nproc; i++)
        {
            format = " Usage for Process %d = " + float_format
                     + " megabytes.\n";
            SNPRINTF(msg_buff,1024,format.c_str(), i, memSizeVals[i]);
            msg +=msg_buff;
        }    
    }
    
    return msg;
}


