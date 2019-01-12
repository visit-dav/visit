// ****************************************************************************
//  Method: avtLocalThresholdExpression constructor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************

// ************************************************************************* //
//                           avtMergeTreeExpression.C                        //
// ************************************************************************* //

#include <avtMergeTreeExpression.h>

#include <vector>
#include <algorithm>
#include <map>
#include <stack>

#include "vtkDataSet.h"
#include "vtkDataArray.h"
#include "vtkTypeUInt32Array.h"
#include "vtkFloatArray.h"
#include "vtkIdList.h"
#include "vtkMergeTree.h"
#include "vtkPointData.h"

#include <avtExprNode.h>
#include <ExprToken.h>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>
#include <vtkMergeTree.h>
#include <vtkCell.h>
#include <vtkCellData.h>


vtkMergeTree* avtMergeTreeExpression::tree = vtkMergeTree::New();

// ****************************************************************************
//  Class: UnionFind
//
//  Purpose:
//  Standard union-find implementation
//  This class implements a default union-find structure using an stl::map
//  to maintain an index map from label values to local indices. This may
//  not be the fastest implementation but it is convenient to maintain
//  labels in a spase index space.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
//  Modifications:
//    Kathleen Biagas, Thu Nov 10 18:14:10 PST 2016
//    Create a less-than comparator for vtkTypeUInt32, passed to std::map,
//    satisfies static symbol problems with xlc on BGQ.
//
//    Burlen Loring, Thu Nov 17 10:28:57 PST 2016
//    fix a compile error on Apple, vtkTypeUInt32Less::operator() needs
//    to be const
//
// ****************************************************************************

class UnionFind
{
public:
    //! Default constructor
    UnionFind()
        {}

    //! Default destructor
    ~UnionFind()
        {}

    //! Return the current representative of the given label
    vtkTypeUInt32 rep(vtkTypeUInt32 id);

    //! Add a label
    void addLabel(vtkTypeUInt32 label);

    //! Combine the "from" label with the "to" label
    void mergeLabel(vtkTypeUInt32 from, vtkTypeUInt32 to);

private:

    //! The current representative of the i'th label
    std::vector<vtkTypeUInt32> mLabel;

    // create a less-than comparator for vtkTypeUInt32, to be used with map
    struct vtkTypeUInt32Less
    {
        bool operator()(const vtkTypeUInt32 &lhs,
            const vtkTypeUInt32 &rhs) const
        {    return lhs < rhs; }
    };

    //! An index map to convert global label-indices into local mLabel indices
    std::map<vtkTypeUInt32,vtkTypeUInt32,vtkTypeUInt32Less> mIndexMap;
};

// ****************************************************************************
//  Method: UnionFind::rep
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkTypeUInt32
UnionFind::rep(vtkTypeUInt32 id)
{
    vtkTypeUInt32 local;

    //! Sanity check to make sure we ask only for existing labels
    assert(mIndexMap.find(id) != mIndexMap.end());

    //! Get the local index of the label in question
    local = mIndexMap.find(id)->second;

    //! Jump "upward" until you find the current representative
    std::stack<vtkTypeUInt32> s;
    while (mLabel[local] != id)
    {
        s.push(local);
        id = mLabel[local];

        assert(mIndexMap.find(id) != mIndexMap.end());
        local = mIndexMap.find(id)->second;
    }

    //! Shortcut the structure
    if (!s.empty())
    {
        s.pop();
        while (!s.empty())
        {
            mLabel[s.top()] = id;
            s.pop();
        }
    }
    
    return id;
}

// ****************************************************************************
//  Method: UnionFind::addLabel
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
UnionFind::addLabel(vtkTypeUInt32 label)
{
    mLabel.push_back(label);
    mIndexMap[label] = mLabel.size()-1;
}

// ****************************************************************************
//  Method: UnionFind::mergeLabel
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
UnionFind::mergeLabel(vtkTypeUInt32 from, vtkTypeUInt32 to)
{
    assert(mIndexMap.find(from) != mIndexMap.end());
    assert(mIndexMap.find(to) != mIndexMap.end());

    // Make sure the "newer" label survives
    assert(from < to);

    mLabel[mIndexMap.find(from)->second] = to;
}


// ****************************************************************************
//  Method: avtMergeTreeExpression::Greater constructor 
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::Greater::Greater(vtkDataArray* f)
: Function(f) 
{
    //empty
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::NeighborhoodIterator constructor 
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::NeighborhoodIterator::NeighborhoodIterator(vtkDataSet* data)
{
    //empty
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::NeighborhoodIterator destructor 
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::NeighborhoodIterator::~NeighborhoodIterator()
{
    //empty
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::NodeIterator constructor 
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::NodeIterator::NodeIterator(vtkDataSet* data)
: NeighborhoodIterator(data), DataSet(data)
{
    //empty
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::NodeIterator destructor 
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::NodeIterator::~NodeIterator()
{
    //empty
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::NodeIterator::initialize
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
avtMergeTreeExpression::NodeIterator::initialize(vtkIdType v)
{
    static vtkSmartPointer<vtkIdList> cells = vtkSmartPointer<vtkIdList>::New();
    static vtkSmartPointer<vtkIdList> verts = vtkSmartPointer<vtkIdList>::New();


    Neighbors.clear();

    DataSet->GetPointCells(v,cells);

    for (vtkIdType i=0;i<cells->GetNumberOfIds();i++)
    {

        DataSet->GetCellPoints(cells->GetId(i),verts);
        for (vtkIdType j=0;j<verts->GetNumberOfIds();j++)
        {
            if (verts->GetId(j) != v)
                Neighbors.insert(verts->GetId(j));
    }
    }

    nIt = Neighbors.begin();
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::CellIterator constructor 
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::CellIterator::CellIterator(vtkDataSet* data)
: NeighborhoodIterator(data), DataSet(data) 
{
    //empty
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::CellIterator destructor 
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::CellIterator::~CellIterator()
{
    //empty
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::CellIterator initialize 
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
avtMergeTreeExpression::CellIterator::initialize(vtkIdType v)
{
    vtkCell* cell = DataSet->GetCell(v);
    static vtkSmartPointer<vtkIdList> neighs = vtkSmartPointer<vtkIdList>::New();

    //fprintf(stderr,"Cell dimension %d  #faces %d  #edges %d\n",cell->GetCellDimension(),cell->GetNumberOfFaces(),cell->GetNumberOfEdges());
    Neighbors.clear();

    // If we are dealing with a 2D mesh
    if (cell->GetCellDimension() == 2)
    {
        for (vtkIdType i=0; i < cell->GetNumberOfEdges(); i++)
        {
            DataSet->GetCellNeighbors(v,
                                      cell->GetEdge(i)->GetPointIds(),
                                      neighs);

            //assert(neighs->GetNumberOfIds() < 2);

            for (vtkIdType j=0;j<neighs->GetNumberOfIds();j++)
                Neighbors.push_back(neighs->GetId(j));
        }
    }
    else
    {
        assert (cell->GetCellDimension() == 3);

        for (vtkIdType i=0;i<cell->GetNumberOfFaces();i++)
        {
            DataSet->GetCellNeighbors(v,cell->GetFace(i)->GetPointIds(),neighs);

            //assert(neighs->GetNumberOfIds() < 2);

            for (vtkIdType j=0;j<neighs->GetNumberOfIds();j++)
                Neighbors.push_back(neighs->GetId(j));
        }
    }

    nIt = Neighbors.begin();
}

// ****************************************************************************
//  Method: avtMergeTreeExpression constructor
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::avtMergeTreeExpression(bool mt)
{
    //tree = vtkSegmentedMergeTree::New();
    computeMergeTree = mt;
}


// ****************************************************************************
//  Method: avtMergeTreeExpression destructor
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
avtMergeTreeExpression::~avtMergeTreeExpression()
{
    // empty For now ... Later we need to delete the tree
}

// ****************************************************************************
//  Method: avtMergeTreeExpression::DeriveVariable 
//
//  Purpose:
//      Called to calc the expression results.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkDataArray *
avtMergeTreeExpression::DeriveVariable(vtkDataSet *in_ds,
                                       int currentDomainsIndex)
{
    vtkIdType i;

    // Create an array for the vertex labels
    vtkTypeUInt32Array* labels = vtkTypeUInt32Array::New();
    labels->SetNumberOfComponents(1);
    labels->SetName("Labels");


    debug5 << "Number of points " << in_ds->GetNumberOfPoints() << std::endl
           <<  "Threshold " << this->threshold;
    // fprintf(stderr,"Number of points %lld\nThreshold %f\n",in_ds->GetNumberOfPoints(),this->threshold);

    // First we assume that the incoming function is node-centered
    avtCentering centering = AVT_NODECENT;
    vtkDataArray* function = in_ds->GetPointData()->GetArray(activeVariable);

    if (function == NULL)
    { // IF we could not find it look for a cell-centered function
        function = in_ds->GetCellData()->GetArray(activeVariable);
        centering = AVT_ZONECENT;
    }

    if (centering == AVT_NODECENT)
    {
        // For now we will create a full sized array
        labels->SetNumberOfTuples(in_ds->GetNumberOfPoints());
    }
    else
    {
        labels->SetNumberOfTuples(in_ds->GetNumberOfCells());
    }

    std::vector<vtkIdType> order;
    std::vector<vtkIdType>::const_iterator oIt;

    debug5 << "number of labels entries " << labels->GetNumberOfTuples()
           << " number of funtion entries" << function->GetNumberOfTuples();

    // fprintf(stderr,"%d %d \n",labels->GetNumberOfTuples(),function->GetNumberOfTuples());

    // Initialize the comparison function with the appropriate array
    Greater cmp(function);

    // compute either merge tree or split tree
    if (computeMergeTree)
    {

        // Initialize the array (there really should be a faster way) and screen the vertices
        for (i=0;i<labels->GetNumberOfTuples();i++)
        {
          labels->SetTuple1(i,LNULL);

          if (function->GetTuple1(i) >= threshold)
              order.push_back(i);
        }
        // For merge trees we order in descending order
        std::sort(order.begin(),order.end(),cmp);
    }
    else
    {
        // Initialize the array (there really should be a faster way) and screen the vertices
        for (i=0;i<labels->GetNumberOfTuples();i++)
        {
            labels->SetTuple1(i,LNULL);

            if (function->GetTuple1(i) <= threshold)
                order.push_back(i);
        }

        // For split trees we sort in ascending order
        std::sort(order.rbegin(),order.rend(),cmp);
    }

    debug5 << "down with sorting";
    //fprintf(stderr,"Done with sorting\n");

    // Note that this assignment is by logical order. The "maximum" will be the most extreme leaf
    // While the minimum will be the function value of the root of the tree
    tree->SetMaximum(function->GetTuple1(order[0]));
    tree->SetMinimum(function->GetTuple1(order.back()));


    UnionFind uf;
    vtkTypeUInt32 neigh_label;
    vtkTypeUInt32 new_label,tmp;
    vtkIdList* cells;
    NeighborhoodIterator* it;

    // Create the appropriate neighborhood iterator
    if (centering == AVT_NODECENT)
        it = new NodeIterator(in_ds);
    else
        it = new CellIterator(in_ds);


    // uint32_t progress = 0;
    // uint32_t next = 1;
    //fprintf(stderr,"Processing  %03d%%\r",0);

    // For all vertices in descending order (assuming a merge tree)
    for (oIt=order.begin();oIt!=order.end();oIt++)
    {

        // if (100*progress/order.size() >= next)
        // {
        //     //fprintf(stderr,"Processing  %03ld%%\r",100*progress/order.size());
        //     next++;
        // }
        // progress++;

        //fprintf(stderr,"Processing vertex/cell %d\n",*oIt);

        // For all neighbors (careful might touch the same neighbor
        // multiple times)
        for (it->initialize(*oIt);!it->end();(*it)++)
        {
              //fprintf(stderr,"\tNeighbor %lld  label: %d\n",it->id(),labels->GetTuple1(it->id()));
            if (labels->GetTuple1(it->id()) != LNULL)
            { 
                // If the neighbor has already been labeled it is considered higher
                // Find its current active label
                neigh_label = uf.rep(labels->GetTuple1(it->id()));

                if (labels->GetTuple1(*oIt) == LNULL)
                {
                    // If this is the first label we see
                    labels->SetTuple1(*oIt,neigh_label); // We pass on this label
                }
                else if (neigh_label != labels->GetTuple1(*oIt))
                {
                    // If we see a second label *oIt is a saddle
                    // If the node corresponding to our current label is not *oIt itself
                    // then we have not yet created a critical point for *oIt
                    if (tree->GetId(labels->GetTuple1(*oIt)) != *oIt)
                    {
                        // Add a new node into the tree and use its id (wrt. the tree) as label
                        new_label = tree->AddNode(*oIt);

                        //if (new_label == 117)
                        //  fprintf(stderr,"Break\n");

                        // Now set the pointer for the node corresponding to the current label
                        tree->AddEdge(labels->GetTuple1(*oIt),new_label);

                        // And pass on the representative
                        tree->SetRep(new_label,tree->GetRep(labels->GetTuple1(*oIt)));

                        // Create a corresponding UF label
                        uf.addLabel(new_label);

                        // And merge the two labels making sure the later one survives
                        uf.mergeLabel(labels->GetTuple1(*oIt),new_label);

                        // And update our own label
                        labels->SetTuple1(*oIt,new_label);
                    }

                    // The above if statement took care of the first arc that reached *oIt.
                    // Now we take care of the second arc with neigh_label

                    // Set the appropriate down pointer
                    tree->AddEdge(neigh_label,labels->GetTuple1(*oIt));

                    // First, update the representative if necessary. Since we create the
                    // labels (and nodes) in order of the sort, the rep id can be used to
                    // determine which node is higher.
                    if (tree->GetRep(neigh_label) < tree->GetRep(labels->GetTuple1(*oIt)))
                    {
                        tree->SetRep(labels->GetTuple1(*oIt),tree->GetRep(neigh_label));
                    }

                    // Now we merge the labels
                    uf.mergeLabel(neigh_label,labels->GetTuple1(*oIt));

                } // end-if we see a second/third/... label
            } // end-if we found a labeled neighbor
        } // end-for all neighbors

        if (labels->GetTuple1(*oIt) == LNULL)
        { 
            // If we have not found a higher neighbor

            // Add a new node into the tree and use its id as label
            new_label = tree->AddNode(*oIt);

            // Add the label to the UF
            uf.addLabel(new_label);

            // Set the new label
            labels->SetTuple1(*oIt,new_label);
        }

        // Finally, we have the actual label for *oIt and we add it to the respective branch
        // This is only required if we want to keep an explicit representation of the branches
        // For now all use cases rely on the implicit labels so this is disabled.
        // tree->AddVertexToBranch(labels->GetTuple1(*oIt),*oIt);

    } // end-for all vertices in sorted order

    debug5 << "done computing labels" << std::endl;
    delete it;
    return labels;
}

// ****************************************************************************
//  Method: avtLocalThresholdExpression::ProcessArguments
//
//  Purpose:
//      Processes args passed to the expression.
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
avtMergeTreeExpression::ProcessArguments(ArgsExpr* args,
                                         ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException,
                   outputVariableName,
                   "avtMergeTreeExpression: No arguments given.");
    }

    // Tell the first argument to create its filters.
    ArgExpr *firstarg = (*arguments)[0];
    avtExprNode *firstTree = dynamic_cast<avtExprNode*>(firstarg->GetExpr());
    firstTree->CreateFilters(state);

    // Check if there's a second argument.
    if (nargs == 1)
    {
        if (computeMergeTree)
            this->threshold = -10e34;
        else
            this->threshold = 10e34;
        return;
    }

    // Pull off the second argument and check whether we can convert that into a number
    ArgExpr *secondarg = (*arguments)[1];
    avtExprNode *secondTree = dynamic_cast<avtExprNode*>(secondarg->GetExpr());
    std::string type = secondTree->GetTypeName();
    if (!GetNumericVal(secondTree, this->threshold))
    {
        debug5 << "avtMergeTreeExpression: Second argument is not a number: \"" << type << "\"" << endl;
        EXCEPTION2(ExpressionException,
                    outputVariableName,
                    "avtMergeTreeExpression: Second argument is not a number.");
    }


    debug1 << "Parsed arguments in MergeTree: varname = " << outputVariableName  
           << " threshold = " << this->threshold << std::endl;
}





