// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtMergeTreeExpression.h                             //
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
