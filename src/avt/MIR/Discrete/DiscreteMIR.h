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

#ifndef DISCRETEMIR_H
#define DISCRETEMIR_H

#include <MIR.h>
#include <MIRConnectivity.h>
#include <VisItArray.h>

// ****************************************************************************
//  Class:  DiscreteMIR
//
//  Purpose:
//    Produces a discretized material reconstruction based upon:
//
//      John C. Anderson, Christoph Garth, Mark A. Duchaineau, and
//      Kenneth I. Joy, "Discrete Multi-Material Interface
//      Reconstruction for Volume Fraction Data," Computer Graphics
//      Forum (Proc. of EuroVis), vol. 27, pp. 1015--1022, May 2008.
//
//  Notes:
//
//    I haven't looked very hard, but I could probably eliminate the
//    Cell and Node classes used here in favor of code already
//    included withing VTK or VisIt.  For now, however, they work...
//
//  Programmer:  John C. Anderson
//  Creation:    October 17, 2008
//
//  Modifications:
//    Jeremy Meredith, Mon Nov 30 17:31:34 EST 2009
//    Split labels into mixed and clean versions to avoid assumptions
//    about signedness of pointers and sizes of integers and pointers.
//
// ****************************************************************************
class MIR_API DiscreteMIR : public MIR
{
  public:
    DiscreteMIR();
    virtual ~DiscreteMIR();

    // do the processing
    bool         ReconstructMesh(vtkDataSet *, avtMaterial *, int);
    virtual bool Reconstruct3DMesh(vtkDataSet *, avtMaterial *);
    virtual bool Reconstruct2DMesh(vtkDataSet *, avtMaterial *);

    // material select everything -- all variables, the mesh, and the material
    // if requested.
    virtual vtkDataSet *GetDataset(std::vector<int>, vtkDataSet *, 
                                   std::vector<avtMixedVariable *>, bool,
                                   avtMaterial * = NULL);

    // get some result flags
    virtual bool SubdivisionOccurred()   { return true; }
    virtual bool NotAllCellsSubdivided() { return false; }

  protected:

    struct ReconstructedCoord
    {
        double x,y,z;
        double weight[MAX_NODES_PER_ZONE];
        int origzone;
    };

    struct ReconstructedZone
    {
        int origzone;
        int startindex;
        int mix_index;
        int mat;
        unsigned char celltype;
        unsigned char nnodes;
    };

    int                                     origNPoints;
    std::vector<float>                      origXCoords;
    std::vector<float>                      origYCoords;
    std::vector<float>                      origZCoords;
    VisItArray<ReconstructedCoord>          coordsList;
    VisItArray<ReconstructedZone>           zonesList;
    VisItArray<vtkIdType>                   indexList;

    int                                     dimension;
    int                                     nMaterials;
    int                                     nOrigMaterials;
    bool                                    noMixedZones;
    int                                     singleMat;

    std::vector<int>                        mapMatToUsedMat;
    std::vector<int>                        mapUsedMatToMat;

    vtkPoints                              *outPts;
    vtkDataSet                             *mesh;


  protected:
    bool ReconstructCleanMesh(vtkDataSet *, avtMaterial *);
    void SetUpCoords();

  protected: // John's stuff...

    class Node;

    class Cell
    {
      public:
        Cell():
            m_i(-1),
            m_j(-1),
            m_k(0)
        {
        }

        Cell(int _m_i, int _m_j, int _m_k = 0):
            m_i(_m_i),
            m_j(_m_j),
            m_k(_m_k)
        {
        }

#if 0
        inline seagull::Point3 centroid4() const
        {
            seagull::Point3 centroid(0.0f, 0.0f, 0.0f);
            for(int n = 0; n < 4; ++n)
            {
                Gallimaufry::Node node = incidentNode4(n);
                centroid += seagull::Point3(node.m_i, node.m_j, node.m_k);
            }
            centroid /= 4;
            return centroid;
        }

        inline seagull::Point3 centroid8() const
        {
            seagull::Point3 centroid(0.0f, 0.0f, 0.0f);
            for(int n = 0; n < 8; ++n)
            {
                Gallimaufry::Node node = incidentNode8(n);
                centroid += seagull::Point3(node.m_i, node.m_j, node.m_k);
            }
            centroid /= 8;
            return centroid;
        }
#endif

        inline Cell incidentCell4(int c) const
        {
            return Cell(m_i + m_incidentCell4[c][0],
                        m_j + m_incidentCell4[c][1]);
        }
        
        inline Cell incidentCell6(int c) const
        {
            return Cell(m_i + m_incidentCell6[c][0],
                        m_j + m_incidentCell6[c][1],
                        m_k + m_incidentCell6[c][2]);
        }

        inline Cell incidentCell8(int c) const
        {
            return Cell(m_i + m_incidentCell8[c][0],
                        m_j + m_incidentCell8[c][1]);
        }

        inline Cell incidentCell26(int c) const
        {
            return Cell(m_i + m_incidentCell26[c][0],
                        m_j + m_incidentCell26[c][1],
                        m_k + m_incidentCell26[c][2]);      
        }
        
        inline Node incidentNode4(int n) const
        {
            return Node(m_i + m_incidentNode4[n][0],
                        m_j + m_incidentNode4[n][1]);
        }

        inline Node incidentNode8(int n) const
        {
            return Node(m_i + m_incidentNode8[n][0],
                        m_j + m_incidentNode8[n][1],
                        m_k + m_incidentNode8[n][2]);
        }

        bool operator==(const Cell &that) const
        {
            return (m_i == that.m_i &&
                    m_j == that.m_j &&
                    m_k == that.m_k);
        }

        bool operator!=(const Cell &that) const
        {
            return !operator==(that);
        }

        bool operator<(const Cell &that) const
        {
            if(m_i < that.m_i)
                return true;
            else if(m_i == that.m_i &&
                    m_j < that.m_j)
                return true;
            else if(m_i == that.m_i &&
                    m_j == that.m_j &&
                    m_k < that.m_k)
                return true;
            else
                return false;
        }

        size_t operator()(const Cell &that) const
        {
            return that.m_i * that.m_j * that.m_k;
        }

        int m_i, m_j, m_k;
        
      protected:

        // Clockwise enumeration of 4 face-incident cells.
        static int m_incidentCell4[4][2];
        /// Face-incident cells in 3D.
        static int m_incidentCell6[6][3];
        /// Clockwise enumeration of 8 incident cells.
        static int m_incidentCell8[8][2];
        /// Incident cells in 3D.
        static int m_incidentCell26[26][3];
        /// Incident nodes in 2D.
        static int m_incidentNode4[4][2];
        /// Incident nodes in 3D.
        static int m_incidentNode8[8][3];
    };

    class Node
    {
      public:
        Node():
            m_i(-1),
            m_j(-1),
            m_k(0)
        {
        }

        Node(int _m_i, int _m_j, int _m_k = 0):
            m_i(_m_i),
            m_j(_m_j),
            m_k(_m_k)
        {
        }
    
        inline Cell incidentCell4(int c) const
        {
            return Cell(m_i + m_incidentCell4[c][0],
                        m_j + m_incidentCell4[c][1]);
        }

        inline Cell incidentCell8(int c) const
        {
            return Cell(m_i + m_incidentCell8[c][0],
                        m_j + m_incidentCell8[c][1],
                        m_k + m_incidentCell8[c][2]);
        }

        inline Node neighborNode(int n) const
        {
            return Node(m_i + m_neighborNode[n][0],
                        m_j + m_neighborNode[n][1]);
        }
        
        bool operator==(const Node &that) const
        {
            return (m_i == that.m_i &&
                    m_j == that.m_j &&
                    m_k == that.m_k);
        }

        bool operator!=(const Node &that) const
        {
            return !operator==(that);
        }

        bool operator<(const Node &that) const
        {
            if(m_i < that.m_i)
                return true;
            else if(m_i == that.m_i &&
                    m_j < that.m_j)
                return true;
            else if(m_i == that.m_i &&
                    m_j == that.m_j &&
                    m_k < that.m_k)
                return true;
            else
                return false;
        }

        size_t operator()(const Node &that) const
        {
            return that.m_i * that.m_j * that.m_k;
        }

        int m_i, m_j, m_k;

      protected:
        static int m_incidentCell4[4][2];
        static int m_incidentCell8[8][3];
        static int m_neighborNode[4][2];
    };








    int DX, DY, DZ;
    int NX, NY, NZ;

    int dimensions[3];

    float *xspacing, *yspacing, *zspacing;

    std::vector< Cell > m_mixedCells;
    unsigned char **m_mixedlabels;
    int            *m_cleanlabels;
    float *m_neighborhood;

    size_t m_size[3];

    double m_temperature;
    
    void optimize();

    int id(const Cell &cell) const
    {
        return
            cell.m_k * (dimensions[0] * dimensions[1]) +
            cell.m_j * (dimensions[0]) +
            cell.m_i;
    }

    int id(const Node &node) const
    {
        return
            node.m_k * ((dimensions[0] + 1) * (dimensions[1] + 1)) +
            node.m_j * ((dimensions[0] + 1)) +
            node.m_i;
    }

    bool isValid(const Cell &cell) const;
    bool isValid(const Node &node) const;

    size_t getSize(int d) const
    {
        return m_size[d];
    }

    unsigned char get(size_t i, size_t j, size_t k) const;

    bool inside(size_t i, size_t j, size_t k) const
    {
        return
            i < m_size[0] &&
            j < m_size[1] &&
            k < m_size[2];
    }

};



#endif
