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
//                           avtSingleFilterFacade.h                         //
// ************************************************************************* //

#ifndef AVT_SINGLE_FILTER_FACADE_H
#define AVT_SINGLE_FILTER_FACADE_H

#include <pipeline_exports.h>

#include <avtFacadeFilter.h>


// ****************************************************************************
//  Class: avtSingleFilterFacade
//
//  Purpose:
//      This is for a class that wants to acts as a facade, but will only be
//      a facade to a single filter (so that class probably is trying to change
//      the interface, like make it a plugin filter).  This class' only 
//      purpose is to decrease the number of virtual functions that the derived
//      types must define.
//
//  Programmer: Hank Childs
//  Creation:   March 16, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jun  7 15:04:09 PDT 2005
//    Use new interface for creating facaded filters.
//
//    Tom Fogal, Tue Jun 23 20:37:54 MDT 2009
//    Added const versions of some methods.
//
// ****************************************************************************

class PIPELINE_API avtSingleFilterFacade : public avtFacadeFilter
{
  public:
                              avtSingleFilterFacade();
    virtual                  ~avtSingleFilterFacade();

  protected:
    virtual int               GetNumberOfFacadedFilters(void) { return 1; };
    virtual avtFilter        *GetIthFacadedFilter(int);
    virtual const avtFilter  *GetIthFacadedFilter(int) const;

    virtual avtFilter        *GetFacadedFilter(void) = 0;
    virtual const avtFilter  *GetFacadedFilter(void) const = 0;
};

#endif
