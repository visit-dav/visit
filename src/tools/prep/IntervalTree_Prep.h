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
//                          IntervalTree_Prep.h                              //
// ************************************************************************* //

#ifndef INTERVAL_TREE_PREP_H
#define INTERVAL_TREE_PREP_H

#include <visitstream.h>
#include <silo.h>

#include <IntervalTree.h>


// ****************************************************************************
//  Class: IntervalTree
// 
//  Purpose:
//      A derived type of IntervalTree, this is meant exclusively for
//      meshtvprep.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

class IntervalTree_Prep : public IntervalTree
{
  public:
                   IntervalTree_Prep();
    virtual       ~IntervalTree_Prep();

    void           AddVar(int, float *);

    void           SetFieldName(char *);
    void           SetNDomains(int);
    void           SetNDims(int);

    int            GetNDims(void)  { return nDims; };
    void           GetRootExtents(float *);

    void           ReadVar(DBmeshvar *, int);
    void           ReadVar(DBpointmesh *, int);
    void           ReadVar(DBquadmesh *, int);
    void           ReadVar(DBquadvar *, int);
    void           ReadVar(DBucdmesh *, int);
    void           ReadVar(DBucdvar *, int);

    void           WrapUp(void);
    virtual void   Write(DBfile *);
      
  protected:
    bool           setNDims;
    bool           setNDomains;
    int            vectorSize;

    // Protected Methods
    void           CollectInformation(void);
    void           ConstructNodeExtents(void);
    void           ConstructTree(void);
    bool           Less(float *, float *, int);
    void           ReadVar(float **, int, int *, int);
    void           SetIntervals(void);
    void           Sort(float *, int *, int, int);
    int            SplitSize(int);

    // Constants
    static char * const    SILO_LOCATION;
};


#endif


