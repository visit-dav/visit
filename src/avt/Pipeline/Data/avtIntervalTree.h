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
//                            avtIntervalTree.h                              //
// ************************************************************************* //

#ifndef AVT_INTERVAL_TREE_H
#define AVT_INTERVAL_TREE_H

#include <pipeline_exports.h>

#include <vectortypes.h>


// ****************************************************************************
//  Class: avtIntervalTree
//
//  Purpose:
//      Keeps meta-data for fields.  The meta-data is stored as a tree and each
//      node covers the range of its children.  Each leaf node contains a range
//      that corresponds to the extents of a specific domain.  The range is of
//      spacial or data extents.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Aug 11 09:14:02 PDT 2000
//    Added conversion constructor from prototype's tree, IntervalTree.
//
//    Hank Childs, Tue Nov 19 12:26:34 PST 2002
//    Removed all knowledge of previous incarnation of 'IntervalTree'.
//
//    Mark C. Miller, Mon Oct 18 14:36:49 PDT 2004
//    Added hasBeenCalculated variable
//
//    Hank Childs, Mon Jun 27 09:01:01 PDT 2005
//    Remove unused methods for sorting.
//
//    Hank Childs, Fri Jul 28 08:09:23 PDT 2006
//    Add axially-symmetric line intersection test.
//
//    Kathleen Bonnell, Mon Aug 21 13:34:18 PDT 2006 
//    Add default bool arg to constructor -- specifies whether or not this tree
//    will required collective communication. 
//
//    Hank Childs, Thu Mar  1 16:57:34 PST 2007
//    Add a method to get the dimension (for error checking).
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtIntervalTree
{
  public:
                              avtIntervalTree(int, int, bool = true);
    virtual                  ~avtIntervalTree();

    static void               Destruct(void *);

    void                      GetExtents(double *) const;
    int                       GetDimension(void) const { return nDims; };
    void                      GetElementsList(const double *, double,
                                             std::vector<int> &) const;
    void                      GetElementsList(double [3], double[3],
                                             std::vector<int> &) const;
    void                      GetElementsListFromRange(const double *,
                                                      const double *,
                                                      std::vector<int>&) const;
    void                      GetElementsFromAxiallySymmetricLineIntersection(
                                       const double *, const double *,
                                       std::vector<int>&) const;
    void                      GetElementsListFromRay(double [3], double[3],
                                          intVector &, doubleVector &) const; 
    void                      GetElementsListFromLine(double [3], double[3],
                                          intVector &, doubleVector &) const; 

    void                      AddElement(int, double *);
    void                      Calculate(bool = false);

    int                       GetNLeaves(void) const { return nElements; };
    int                       GetLeafExtents(int, double *) const;
    void                      GetElementExtents(int, double *) const;

  protected:
    int                       nElements;
    int                       nNodes;
    int                       nDims;
    int                       vectorSize;

    double                   *nodeExtents;
    int                      *nodeIDs;

    bool                      hasBeenCalculated;
    bool                      requiresCommunication;

    void                      CollectInformation(void);
    void                      ConstructTree(void);
    void                      SetIntervals(void);
    int                       SplitSize(int);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtIntervalTree(const avtIntervalTree &) {;};
    avtIntervalTree     &operator=(const avtIntervalTree &) { return *this; };
};


#endif

  
