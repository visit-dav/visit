/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <avtCellLocatorBIH.h>

#include <vtkDataSet.h>
#include <DebugStream.h>

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>

// -------------------------------------------------------------------------

class celltree
{
public:
    struct node
    {
        unsigned int index;

        union {
            struct {
                double lm;
                double rm;
            };

            struct {
                unsigned int sz;
                unsigned int st;
            };
        };

        void make_node( unsigned int left, unsigned int d, double b[2] )
        {
            index = (d & 3) | (left << 2);
            lm = b[0];
            rm = b[1];
        }

        void set_children( unsigned int left )
        {
            index = dim() | (left << 2);
        }

        bool is_node() const
        {
            return (index & 3) != 3;
        }

        unsigned int left() const
        {
            return (index >> 2);
        }

        unsigned int right() const
        {
            return (index >> 2) + 1;
        }

        unsigned int dim() const
        {
            return index & 3;
        }

        const double& lmax() const
        {
            return lm;
        }

        const double& rmin() const
        {
            return rm;
        }

        // ---

        void make_leaf( unsigned int start, unsigned int size )
        {
            index = 3;
            sz = size;
            st = start;
        }

        bool is_leaf() const
        {
            return index == 3;
        }

        unsigned int start() const
        {
            return st;
        }

        unsigned int size() const
        {
            return sz;
        }
    };

    std::vector<node>         nodes;
    std::vector<unsigned int> leaves;


    struct point_traversal
    {
        const celltree& m_ct;
        unsigned int    m_stack[32];
        unsigned int*   m_sp;
        const double*    m_pos;

        point_traversal( const celltree& ct, const double* pos ) :
            m_ct(ct), m_pos(pos)
        {
            m_stack[0] = 0;
            m_sp = m_stack + 1;
        }

        const celltree::node* next()
        {
            while( true )
            {
                if( m_sp == m_stack )
                    return 0;

                const celltree::node* n = &m_ct.nodes.front() + *(--m_sp);

                if( n->is_leaf() )
                    return n;

                const double p = m_pos[n->dim()];
                const unsigned int left = n->left();

                bool l = p <= n->lmax();
                bool r = p >= n->rmin();

                if( l && r )
                {
                    if( n->lmax()-p < p-n->rmin() )
                    {
                        *(m_sp++) = left;
                        *(m_sp++) = left+1;
                    }
                    else
                    {
                        *(m_sp++) = left+1;
                        *(m_sp++) = left;
                    }
                }
                else if( l )
                    *(m_sp++) = left;
                else if( r )
                    *(m_sp++) = left+1;
            }
        }
    };
};

// -------------------------------------------------------------------------

class celltree_builder
{
private:
    
    struct bucket
    {
        double        min;
        double        max;
        unsigned int cnt;
    
        bucket()
        {
            cnt = 0;
            min =  std::numeric_limits<double>::max();
            max = -std::numeric_limits<double>::max();
        }
    
        void add( const double _min, const double _max )
        {
            ++cnt;
        
            if( _min < min )
                min = _min;
            
            if( _max > max )
                max = _max;
        }
    };

    struct per_cell 
    {
        double        min[3];
        double        max[3];
        unsigned int ind;
    };

    struct center_order
    {
        unsigned int d;
    
        center_order( unsigned int _d ) : 
            d(_d)
        {
        }

        bool operator()( const per_cell& pc0, const per_cell& pc1 )
        {
            return (pc0.min[d] + pc0.max[d]) < (pc1.min[d] + pc1.max[d]);
        }
    };

    struct left_predicate
    {
        unsigned int       d;
        double              p;
    
        left_predicate( unsigned int _d, double _p ) : 
            d(_d), p(2.0f*_p)
        {
        }
   
        bool operator()( const per_cell& pc )
        {
            return (pc.min[d] + pc.max[d]) < p;
        }
    };


    // -------------------------------------------------------------------------

    void find_min_max( const per_cell* begin, const per_cell* end,  
                       double* min, double* max )
    {
        if( begin == end )
            return;
            
        for( unsigned int d=0; d<3; ++d )
        {
            min[d] = begin->min[d];
            max[d] = begin->max[d];
        }
        
        while( ++begin != end )
        {
            for( unsigned int d=0; d<3; ++d )
            {
                if( begin->min[d] < min[d] )    min[d] = begin->min[d];
                if( begin->max[d] > max[d] )    max[d] = begin->max[d];
            }
        }
    }

    // -------------------------------------------------------------------------
    
    void find_min_d( const per_cell* begin, const per_cell* end,  
                     unsigned int d, double& min )
    {
        min = begin->min[d];
        
        while( ++begin != end )
            if( begin->min[d] < min )    
                min = begin->min[d];
    }

    void find_max_d( const per_cell* begin, const per_cell* end,  
                     unsigned int d, double& max )
    {
        max = begin->max[d];
        
        while( ++begin != end )
            if( begin->max[d] > max )    
                max = begin->max[d];
    }

    // -------------------------------------------------------------------------

    void split( unsigned int index, double min[3], double max[3] )
    {
        unsigned int start = m_nodes[index].start();
        unsigned int size  = m_nodes[index].size();
        
        if( size < m_leafsize )
            return;

        per_cell* begin = m_pc + start;
        per_cell* end   = m_pc + start + size;
        per_cell* mid = begin;

        const int nbuckets = 6;

        const double ext[3] = { max[0]-min[0], max[1]-min[1], max[2]-min[2] };
        const double iext[3] = { ext[0]!=0 ? nbuckets/ext[0] : -1,
                                ext[1]!=0 ? nbuckets/ext[1] : -1,
                                ext[2]!=0 ? nbuckets/ext[2] : -1};
        bucket b[3][nbuckets];
            
        for( const per_cell* pc=begin; pc!=end; ++pc )
        {
            for( unsigned int d=0; d<3; ++d )
            {
                double cen = (pc->min[d] + pc->max[d])/2.0f;
                int   ind = (int)( (cen-min[d])*iext[d] );

                if( ind<0 )
                    ind = 0;

                if( ind>=nbuckets )
                    ind = nbuckets-1;

                b[d][ind].add( pc->min[d], pc->max[d] );
            }
        }
        
        double cost = std::numeric_limits<double>::max();
        double plane = 0;
        unsigned int dim = 0;

        for( unsigned int d=0; d<3; ++d )    
        {
            unsigned int sum = 0;
            
            for( unsigned int n=0; n<nbuckets-1; ++n )
            {
                double lmax = -std::numeric_limits<double>::max();
                double rmin =  std::numeric_limits<double>::max();

                for( unsigned int m=0; m<=n; ++m )
                    if( b[d][m].max > lmax )
                        lmax = b[d][m].max;
                
                for( unsigned int m=n+1; m< (unsigned int)nbuckets; ++m )
                    if( b[d][m].min < rmin )
                        rmin = b[d][m].min;
                
                sum += b[d][n].cnt;
                
                if (ext[d] != 0 && 
                    lmax != -std::numeric_limits<double>::max() && 
                    rmin !=  std::numeric_limits<double>::max())
                {
                    double lvol = (lmax-min[d])/ext[d];
                    double rvol = (max[d]-rmin)/ext[d];
                
                    double c = lvol*sum + rvol*(size-sum);
                
                    if( sum > 0 && sum < size && c < cost )
                    {
                        cost    = c;
                        dim     = d;
                        if (iext[d] > 0)
                            plane = min[d] + (n+1)/iext[d];
                    }
                }
            }
        }

        if( cost != std::numeric_limits<double>::max() )
            mid = std::partition( begin, end, left_predicate( dim, plane ) );

        // fallback
        if( mid == begin || mid == end )
        {
            dim = std::max_element( ext, ext+3 ) - ext;

            mid = begin + (end-begin)/2;
            std::nth_element( begin, mid, end, center_order( dim ) );
        }

        double lmin[3], lmax[3], rmin[3], rmax[3];

        find_min_max( begin, mid, lmin, lmax );
        find_min_max( mid,   end, rmin, rmax );

        double clip[2] = { lmax[dim], rmin[dim] };

        celltree::node child[2];
        child[0].make_leaf( begin - m_pc, mid-begin );
        child[1].make_leaf( mid   - m_pc, end-mid );
        
        m_nodes[index].make_node( m_nodes.size(), dim, clip );
        m_nodes.insert( m_nodes.end(), child, child+2 );

        split( m_nodes[index].left(), lmin, lmax );
        split( m_nodes[index].right(), rmin, rmax );
    }
     
public:

    celltree_builder()
    {
        m_buckets =  5;
        m_leafsize = 8;
    }
    
    void build( celltree& ct, vtkDataSet* ds )
    {
        const vtkIdType size = ds->GetNumberOfCells();
        assert( size <= std::numeric_limits<unsigned int>::max() );

        m_pc = new per_cell[size];

        double min[3] = { 
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max(),
            std::numeric_limits<double>::max()
        };

        double max[3] = { 
            -std::numeric_limits<double>::max(),
            -std::numeric_limits<double>::max(),
            -std::numeric_limits<double>::max(),
        };
        
        for( unsigned int i=0; i<size; ++i )
        {
            m_pc[i].ind = i;

            double bounds[6];
            ds->GetCellBounds( i, bounds );
            
            for( int d=0; d<3; ++d )
            {
                m_pc[i].min[d] = bounds[2*d+0];
                m_pc[i].max[d] = bounds[2*d+1];

                if( m_pc[i].min[d] < min[d] )
                    min[d] = m_pc[i].min[d];

                if( m_pc[i].min[d] > max[d] )
                    max[d] = m_pc[i].max[d];
            }
        }
                
        celltree::node root;
        root.make_leaf( 0, size );
        m_nodes.push_back( root );

        split( 0, min, max );
        
        ct.nodes.resize( m_nodes.size() );
        ct.nodes[0] = m_nodes[0];

        std::vector<celltree::node>::iterator ni = ct.nodes.begin();
        std::vector<celltree::node>::iterator nn = ct.nodes.begin()+1;

        for( ; ni!=ct.nodes.end(); ++ni )
        {
            if( ni->is_leaf() )
                continue;
            
            *(nn++) = m_nodes[ni->left()];
            *(nn++) = m_nodes[ni->right()];

            ni->set_children( nn-ct.nodes.begin()-2 );
        }

        // --- final 
        
        ct.leaves.resize( size );

        for( int i=0; i<size; ++i )
            ct.leaves[i] = m_pc[i].ind;
        
        delete[] m_pc;
    }

public:

    unsigned int                  m_buckets;
    unsigned int                  m_leafsize;
    per_cell*                     m_pc;
    std::vector<celltree::node>   m_nodes;
};

// ---------------------------------------------------------------------------

avtCellLocatorBIH::avtCellLocatorBIH( vtkDataSet* ds ) :
    avtCellLocator( ds )
{
    MaxCellsPerLeaf = 8;
    NumberOfBuckets = 5;

    CellArray = NULL;
    Locations = NULL;

    Tree = NULL;

    Build();
}

// ---------------------------------------------------------------------------

avtCellLocatorBIH::~avtCellLocatorBIH()
{
    Free();
}

// ---------------------------------------------------------------------------

// void avtCellLocatorBIH::SetDataSet( vtkDataSet* ds )
// {
//     vtkVisItAbstractCellLocator::SetDataSet( ds );
//
//     vtkUnstructuredGrid* ug = vtkUnstructuredGrid::SafeDownCast( this->DataSet );
//
//     if( ug )
//     {
//         this->CellArray = ug->GetCells()->GetPointer();
//         this->Locations = ug->GetCellLocationsArray()->GetPointer(0);
//     }
//     else
//     {
//         this->CellArray = NULL;
//         this->Locations = NULL;
//     }
// }

// ---------------------------------------------------------------------------

void avtCellLocatorBIH::Build()
{
    Free();

    vtkIdType numCells;

    if( !dataSet || (numCells = dataSet->GetNumberOfCells()) < 1 )
    {
        debug5 << "avtCellLocatorBIH::BuildLocator(): no cells\n";
        return;
    }

    this->Tree = new celltree;

    celltree_builder builder;

    builder.m_leafsize = MaxCellsPerLeaf;
    builder.m_buckets  = NumberOfBuckets;
    builder.build( *(Tree), dataSet );
}

// ---------------------------------------------------------------------------

void avtCellLocatorBIH::Free()
{
    if( Tree )
    {
        delete Tree;
        Tree = NULL;
    }
}

// ---------------------------------------------------------------------------

void avtCellLocatorBIH::FindCellRecursive( const double pos[3], 
                                           avtInterpolationWeights* weights,
                                           unsigned int node,
                                           vtkIdType& cell,
                                           bool ignoreGhostCells ) const
{
    const celltree::node& n = Tree->nodes[node];

    if( n.is_leaf() )
    {
        // if this is a leaf, test all cells it contains
        const unsigned int* begin = &(Tree->leaves[n.start()]);
        const unsigned int* end   = begin + n.size();

        for( ; begin!=end; ++begin )
        {
            if( TestCell( *begin, pos, weights, ignoreGhostCells ) )
            {
                cell = *begin;
                return;
            }
        }
    }
    else
    {
        // else descend the tree
        const double p = pos[n.dim()];
        const unsigned int left = n.left();

        bool l = p <= n.lmax();
        bool r = p >= n.rmin();

        if( l && r )
        {
            if( n.lmax()-p < p-n.rmin() )
            {
                // go left first
                FindCellRecursive( pos, weights, left, cell, ignoreGhostCells );

                if( cell < 0 )
                    FindCellRecursive( pos, weights, left+1, cell, ignoreGhostCells );
            }
            else
            {
                // go right first
                FindCellRecursive( pos, weights, left+1, cell, ignoreGhostCells );

                if( cell < 0 )
                    FindCellRecursive( pos, weights, left, cell, ignoreGhostCells );
            }
       }
       else if( l )
           FindCellRecursive( pos, weights, left, cell, ignoreGhostCells );
       else if( r )
           FindCellRecursive( pos, weights, left+1, cell, ignoreGhostCells );
    }
}

// ---------------------------------------------------------------------------

vtkIdType avtCellLocatorBIH::FindCell( const double pos[3],
                                       avtInterpolationWeights* weights,
                                       bool ignoreGhostCells ) const
{
    if( Tree == 0 )
        return -1;

    vtkIdType cell = -1;
    FindCellRecursive( pos, weights, 0, cell, ignoreGhostCells );

    return cell;
}

// ---------------------------------------------------------------------------

