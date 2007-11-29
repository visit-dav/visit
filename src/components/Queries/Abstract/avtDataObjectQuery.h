/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                           avtDataObjectQuery.h                            //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_QUERY_H
#define AVT_DATA_OBJECT_QUERY_H

#include <query_exports.h>

#include <avtDataObjectSink.h>
#include <vectortypes.h>


class QueryAttributes;
class avtSILRestriction;
class SILRestrictionAttributes;

typedef void (*InitializeProgressCallback)(void *, int); 
typedef void (*ProgressCallback)(void *, const char *, const char *,int,int);                      

// ****************************************************************************
//  Class: avtDataObjectQuery
//
//  Purpose:
//      The base class for a query object. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 23 15:11:44 PDT 2002
//    Add ChangedInput, VerifyInput.
//  
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Removed unnecessary SetAtts.
//
//    Kathleen Bonnell, Fri Jul 11 16:33:16 PDT 2003 
//    Added units, rename GetMessage as GetResultMessage.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Tue Feb 10 15:00:32 PST 2004 
//    Added OriginalData method. 
//
//    Kathleen Bonnell, Thu Apr  1 16:04:44 PST 2004
//    Added PerformQueryInTime, SetTimeVarying, SetSILUseSet.
//
//    Kathleen Bonnell, Fri Apr  2 08:51:17 PST 2004 
//    Changed args to PerformQueryInTime. 
//
//    Kathleen Bonnell, Tue May  4 14:18:26 PDT 2004 
//    Replaced SetSILUseSet with SetILRestriction. 
//
//    Kathleen Bonnell, Thu Jun 24 07:45:47 PDT 2004 
//    Added intVector and string arguments to PerformQueryInTime. 
//
//    Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004 
//    Added Set/GetUnits. 
//
//    Kathleen Bonnell, Mon Jan  3 15:08:37 PST 2005 
//    Overloaded SetSILRestriction method with avtSILRestriction_p arg. 
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Added GetTimeCurveSpecs. 
//
//    Hank Childs, Thu Feb  8 09:57:39 PST 2007
//    Made GetNFilters be a public method.
//
//    Kathleen Bonnell, Tue Nov 20 10:27:51 PST 2007 
//    Remove unused PerformQueryInTime method. Add GetShortDescription method,
//    whereby queries-through-time can specify a short label for y-axis than
//    their query name.
//
// ****************************************************************************

class QUERY_API avtDataObjectQuery : public virtual avtDataObjectSink
{
  public:
                            avtDataObjectQuery();
    virtual                ~avtDataObjectQuery();

    virtual const char     *GetType(void) = 0;
    virtual const char     *GetDescription(void) { return NULL; };
    virtual const char     *GetShortDescription(void) { return NULL; };

    virtual bool            OriginalData(void) { return false; };
    virtual void            PerformQuery(QueryAttributes *) = 0;
    virtual std::string     GetResultMessage(void) = 0;

    static void             RegisterInitializeProgressCallback(
                                          InitializeProgressCallback, void *);

    static void             RegisterProgressCallback(ProgressCallback, void *);
    virtual int             GetNFilters();

    virtual void            SetTimeVarying(bool val) { timeVarying = val;}
    virtual void            GetTimeCurveSpecs(bool &timeForX, int &nRes);

    void                    SetSILRestriction(const SILRestrictionAttributes *);
    void                    SetSILRestriction(const avtSILRestriction_p);
    std::string            &GetUnits(void) { return units; }
    void                    SetUnits(const std::string &_units) 
                                { units = _units;}

  protected:
    static InitializeProgressCallback
                                  initializeProgressCallback;
    static void                  *initializeProgressCallbackArgs;

    static ProgressCallback       progressCallback;
    static void                  *progressCallbackArgs;

    void                          Init(const int ntimesteps = 1);

    void                          UpdateProgress(int, int);
    virtual void                  ChangedInput(void);
    virtual void                  VerifyInput(void);

    std::string                   units;
    bool                          timeVarying;
    avtSILRestriction_p           querySILR;
};


#endif


