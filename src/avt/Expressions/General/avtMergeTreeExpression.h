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
//                      avtLocalThresholdExpression.C                        //
// ************************************************************************* //
#ifndef AVT_MERGETREE_EXPRESSION_H
#define AVT_MERGETREE_EXPRESSION_H


#include <avtSingleInputExpressionFilter.h>
#include <vtkMergeTree.h>
#include <vtkSmartPointer.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;

// ****************************************************************************
//  Class: avtMergeTreeExpression
//
//  Purpose:
//      Creates the merge/split tree of a scalar function and the corresponding
//      per-branch labels
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************


class EXPRESSION_API avtMergeTreeExpression : public avtSingleInputExpressionFilter
{
public:

    // The internal Tree
    static vtkMergeTree* tree;

    // Invalid segmentation id
    static const vtkTypeUInt32 LNULL = (vtkTypeUInt32)-1;

             avtMergeTreeExpression(bool mt = true);
    virtual ~avtMergeTreeExpression();


    virtual const char  *GetType(void)
                            { return "avtMergeTreeExpression"; };
    virtual const char  *GetDescription(void)
                            {return "Compute the merge tree";};

    virtual void         ProcessArguments(ArgsExpr*,
                                          ExprPipelineState *);

protected:

    // Internal comparison operator
    class Greater
    {
        public:
            vtkDataArray* Function;
            // Cyrus todo:: don't inline constructors 
            
            Greater(vtkDataArray* f);

            bool operator()(vtkIdType u, vtkIdType v)
                {return this->Function->GetTuple1(u) > this->Function->GetTuple1(v);}
    };

    // Description
    // Abstract base class for different dataset types
    class NeighborhoodIterator
    {
        public:
                     NeighborhoodIterator(vtkDataSet* data);
            virtual ~NeighborhoodIterator();

            virtual void initialize(vtkIdType v) = 0;
            virtual void operator++(int i) = 0;
            virtual vtkIdType id() = 0;
            virtual bool end() = 0;
    };

    // Iterate through node neighborhood iterator
    class NodeIterator : public NeighborhoodIterator
    {
        public:
                              NodeIterator(vtkDataSet* data);
            virtual          ~NodeIterator();
            virtual void      initialize(vtkIdType v);
            virtual void      operator++(int i)
                                {nIt++;}
            virtual vtkIdType id()
                                {return *nIt;}
            virtual bool      end()
                                {return (nIt == Neighbors.end());}
        private:
            vtkDataSet* DataSet;
            std::set<vtkIdType> Neighbors;
            std::set<vtkIdType>::iterator nIt;
    };

    // Iterate through cell neighborhood iterator
    class CellIterator : public NeighborhoodIterator
    {
        public:
            
                              CellIterator(vtkDataSet* data);
            virtual          ~CellIterator();
            virtual void      initialize(vtkIdType v);
            virtual void      operator++(int i)
                                {nIt++;}
            virtual vtkIdType id()
                                {return *nIt;}
            virtual bool      end()
                                {return (nIt == Neighbors.end());}
            
        private:
            vtkDataSet* DataSet;
            std::vector<vtkIdType> Neighbors;
            std::vector<vtkIdType>::iterator nIt;
    };




    virtual int               GetVariableDimension(void)
                                { return 1; };

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *,
                                             int currentDomainsIndex);


    // Flag to indicate whether we compute merge(true) or split(false) trees
    bool computeMergeTree;

    // The threshold applied before the computation
    double threshold;


private:
    NeighborhoodIterator* neighborhood(vtkDataSet* data,
                                       vtkIdType v);
};





#endif /* AVT_MERGETREE_EXPRESSION_H */
