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
//                             avtPixieOptions.C                             //
// ************************************************************************* //

#include <avtPixieOptions.h>

#include <DBOptionsAttributes.h>

#include <string>
#include <vector>

// ****************************************************************************
//  Function: GetPixieReadOptions
//
//  Purpose:
//      Creates the options for Pixie readers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require read options, no modifications are 
//      necessary.
//
//  Programmer: Jean Favre
//  Creation:   Thu Jun 21 16:20:41 PDT 2012
//
// ****************************************************************************

DBOptionsAttributes *
GetPixieReadOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    std::vector<std::string> partition;
    partition.push_back("X Slab");  // 0
    partition.push_back("Y Slab");  // 1
    partition.push_back("Z Slab");  // 2
    partition.push_back("KD Tree"); // 3
    rv->SetEnum(PixieDBOptions::RDOPT_PARTITIONING, PixieDBOptions::ZSLAB );
    rv->SetEnumStrings(PixieDBOptions::RDOPT_PARTITIONING, partition);
    rv->SetBool(PixieDBOptions::RDOPT_DUPLICATE, false);

    return rv;
}


// ****************************************************************************
//  Function: GetPixieWriteOptions
//
//  Purpose:
//      Creates the options for Pixie writers.
//
//  Important Note:
//      The code below sets up empty options.  If your format 
//      does not require write options, no modifications are 
//      necessary.
//
//  Programmer: Jean Favre
//  Creation:   Thu Jun 21 16:20:41 PDT 2012
//
// ****************************************************************************

DBOptionsAttributes *
GetPixieWriteOptions(void)
{
    DBOptionsAttributes *rv = new DBOptionsAttributes;
    return rv;
}
