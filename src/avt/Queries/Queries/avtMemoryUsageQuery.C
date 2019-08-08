// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtMemoryUsageQuery.C                             //
// ************************************************************************* //

#include <avtMemoryUsageQuery.h>

#include <avtParallel.h>
#include <QueryAttributes.h>

#include <avtMemory.h>

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
    unsigned long m_size, m_rss;
    
    avtMemory::GetMemorySize(m_size, m_rss);
    
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
    for (size_t i=0;i< memSizeVals.size();i++)
        total_mem_size += memSizeVals[i];
    
    std::string float_format = queryAtts.GetFloatFormat();
    
    int nproc = (int)memSizeVals.size();
    if (  nproc == 1 )
    {
        std::string format = "Engine Memory Usage = " 
                         + float_format + " megabytes.\n";
        snprintf(msg_buff,1024,format.c_str(), total_mem_size);
        msg += msg_buff;
    }
    else
    {
        std::string format = "Total Engine Memory Usage = " 
                         + float_format + " megabytes.\n";
        snprintf(msg_buff,1024,format.c_str(), total_mem_size);
        msg += msg_buff;
        for ( int i=0; i<nproc; i++)
        {
            format = " Usage for Process %d = " + float_format
                     + " megabytes.\n";
            snprintf(msg_buff,1024,format.c_str(), i, memSizeVals[i]);
            msg +=msg_buff;
        }    
    }
    
    return msg;
}


