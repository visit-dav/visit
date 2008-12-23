/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


/*
 *  StreamlineAnalyzer.h:
 *
 *  Written by:
 *   Allen Sanderson
 *   SCI Institute
 *   University of Utah
 *   September 2005
 *
 *  Copyright (C) 2005 SCI Group
 */

#if !defined(StreamlineAnalyzer_h)
#define StreamlineAnalyzer_h

#include <Packages/FusionPSE/Dataflow/Modules/Fields/StreamlineAnalyzerLib.h>

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>

#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/QuadBilinearLgn.h>

#include <Core/Containers/FData.h>

#include <Core/Datatypes/CurveMesh.h>
#include <Core/Datatypes/PointCloudMesh.h>
#include <Core/Datatypes/StructQuadSurfMesh.h>

#include <Core/Datatypes/GenericField.h>

#include <sstream>
using std::ostringstream;

namespace FusionPSE {

using namespace std;
using namespace SCIRun;

class StreamlineAnalyzerAlgo : public DynamicAlgoBase
{
public:

  typedef SCIRun::CurveMesh<CrvLinearLgn<Point> > CMesh;
  typedef SCIRun::CrvLinearLgn<double>            CDatBasis;
  typedef SCIRun::GenericField<CMesh, CDatBasis, vector<double> > CSField;
  typedef SCIRun::GenericField<CMesh, CDatBasis, vector<Vector> > CVField;

  typedef SCIRun::StructQuadSurfMesh<QuadBilinearLgn<Point> > SQSMesh;
  typedef SCIRun::QuadBilinearLgn<double>                     SQSDatBasis;
  typedef SCIRun::GenericField<SQSMesh, SQSDatBasis, FData2d<double,SQSMesh> > SQSSField;
  typedef SCIRun::GenericField<SQSMesh, SQSDatBasis, FData2d<Vector,SQSMesh> > SQSVField;

  typedef SCIRun::PointCloudMesh<ConstantBasis<Point> > PCMesh;
  typedef SCIRun::ConstantBasis<double>                 PCDatBasis;
  typedef SCIRun::GenericField<PCMesh, PCDatBasis, vector<double> > PCField;  

  //! virtual interface. 
  virtual void execute(FieldHandle& slsrc,
                       FieldHandle& dst,
                       FieldHandle& pccsrc,
                       FieldHandle& pccdst,
                       FieldHandle& pcssrc,
                       FieldHandle& pcsdst,
                       vector< double > &planes,
                       unsigned int color,
                       unsigned int showIslands,
                       unsigned int islandCentroids,
                       unsigned int overlaps,
                       unsigned int maxWindings,
                       unsigned int override,
                       unsigned int order,
                       vector< pair< unsigned int,
                       unsigned int > > &topology ) = 0;
  
  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *ftd,
                                            const TypeDescription *mtd,
                                            const TypeDescription *btd,
                                            const TypeDescription *dtd);
protected:
  FieldlineLib FLlib;

};


template< class IFIELD, class OFIELD, class PCFIELD >
class StreamlineAnalyzerAlgoT : public StreamlineAnalyzerAlgo
{
public:
  //! virtual interface. 
  virtual void execute(FieldHandle& src,
                       FieldHandle& dst,
                       FieldHandle& pccsrc,
                       FieldHandle& pccdst,
                       FieldHandle& pcssrc,
                       FieldHandle& pcsdst,
                       vector< double > &planes,
                       unsigned int color,
                       unsigned int showIslands,
                       unsigned int islandCentroids,
                       unsigned int overlaps,
                       unsigned int maxWindings,
                       unsigned int override,
                       unsigned int order,
                       vector< pair< unsigned int,
                       unsigned int > > &topology );

protected:
  virtual void
  loadCurve( FieldHandle &field_h,
             vector < pair< Point, double > > &nodes,
             unsigned int nplanes,
             unsigned int nbins,
             unsigned int nnodes,
             unsigned int plane,
             unsigned int bin,
             unsigned int color,
             double color_value ) = 0;

  virtual void
  loadSurface( FieldHandle &field_h,
               vector < pair< Point, double > > &nodes,
               unsigned int nplanes,
               unsigned int nbins,
               unsigned int nnodes,
               unsigned int plane,
               unsigned int bin,
               unsigned int color,
               double color_value ) = 0;
};


template< class IFIELD, class OFIELD, class PCFIELD >
class StreamlineAnalyzerAlgoTScalar :
  public StreamlineAnalyzerAlgoT< IFIELD, OFIELD, PCFIELD >
{
protected:  
  virtual void
  loadCurve( FieldHandle &field_h,
             vector < pair< Point, double > > &nodes,
             unsigned int nplanes,
             unsigned int nbins,
             unsigned int nnodes,
             unsigned int plane,
             unsigned int bin,
             unsigned int color,
             double color_value );

  virtual void
  loadSurface( FieldHandle &field_h,
               vector < pair< Point, double > > &nodes,
               unsigned int nplanes,
               unsigned int nbins,
               unsigned int nnodes,
               unsigned int plane,
               unsigned int bin,
               unsigned int color,
               double color_value );

};

template< class IFIELD, class OFIELD, class PCFIELD >
class StreamlineAnalyzerAlgoTVector :
  public StreamlineAnalyzerAlgoT< IFIELD, OFIELD, PCFIELD >
{
protected:  
  virtual void
  loadCurve( FieldHandle &field_h,
             vector < pair< Point, double > > &nodes,
             unsigned int nplanes,
             unsigned int nbins,
             unsigned int nnodes,
             unsigned int plane,
             unsigned int bin,
             unsigned int color,
             double color_value );

  virtual void
  loadSurface( FieldHandle &field_h,
               vector < pair< Point, double > > &nodes,
               unsigned int nplanes,
               unsigned int nbins,
               unsigned int nnodes,
               unsigned int plane,
               unsigned int bin,
               unsigned int color,
               double color_value );
};


template< class IFIELD, class OFIELD, class PCFIELD >
void
StreamlineAnalyzerAlgoTScalar<IFIELD, OFIELD, PCFIELD>::
loadCurve( FieldHandle &field_h,
           vector < pair< Point, double > > &nodes,
           unsigned int nplanes,
           unsigned int nbins,
           unsigned int nnodes,
           unsigned int plane,
           unsigned int bin,
           unsigned int color,
           double color_value ) {

  StreamlineAnalyzerAlgo::CSField *ofield =
    (StreamlineAnalyzerAlgo::CSField *) field_h.get_rep();
  typename StreamlineAnalyzerAlgo::CSField::mesh_handle_type omesh =
    ofield->get_typed_mesh();
  typename StreamlineAnalyzerAlgo::CSField::mesh_type::Node::index_type n1, n2;

  n1 = omesh->add_node(nodes[0].first);
  ofield->resize_fdata();
  if( color == 0 )
    ofield->set_value( nodes[0].second, n1);
  else if( color == 1 )
    ofield->set_value( (double) color_value, n1);
  else if( color == 2 )
    ofield->set_value( (double) (0*nbins+bin), n1);
  else if( color == 3 )
    ofield->set_value( (double) plane, n1);
  else if( color == 4 )
    ofield->set_value( (double) bin, n1);
  else if( color == 5 )
    ofield->set_value( (double) 0, n1);
  else
    ofield->set_value( (double) color_value, n1);
  
  for( unsigned int i=1; i<nnodes; i++ ) {
    n2 = omesh->add_node(nodes[i].first);
    ofield->resize_fdata();

    if( color == 0 )
      ofield->set_value(nodes[i].second, n2);
    else if( color == 1 )
      ofield->set_value( (double) color_value, n2);
    else if( color == 2 )
      ofield->set_value( (double) (i*nbins+bin), n2);
    else if( color == 3 )
      ofield->set_value( (double) plane, n2);
    else if( color == 4 )
      ofield->set_value( (double) bin, n2);
    else if( color == 5 )
      ofield->set_value( (double) i, n2);
    else
      ofield->set_value( (double) color_value, n2);
    
    omesh->add_edge(n1, n2);
              
    n1 = n2;
  }
}


template< class IFIELD, class OFIELD, class PCFIELD >
void
StreamlineAnalyzerAlgoTScalar<IFIELD, OFIELD, PCFIELD>::
loadSurface( FieldHandle &field_h,
             vector < pair< Point, double > > &nodes,
             unsigned int nplanes,
             unsigned int nbins,
             unsigned int nnodes,
             unsigned int plane,
             unsigned int bin,
             unsigned int color,
             double color_value ) {
  
  StreamlineAnalyzerAlgo::SQSSField *ofield =
    (StreamlineAnalyzerAlgo::SQSSField *) field_h.get_rep();
  typename StreamlineAnalyzerAlgo::SQSSField::mesh_handle_type omesh =
    ofield->get_typed_mesh();
  typename StreamlineAnalyzerAlgo::SQSSField::mesh_type::Node::index_type n1;

  n1.mesh_ = omesh.get_rep();

  n1.j_ = nplanes * bin + plane;

  for( unsigned int i=0; i<nnodes; i++ ) {

    n1.i_ = i;

    omesh->set_point(nodes[i].first, n1);

    if( color == 0 )
      ofield->set_value( nodes[i].second, n1);
    else if( color == 1 )
      ofield->set_value( (double) color_value, n1);
    else if( color == 2 )
      ofield->set_value( (double) (i*nbins+bin), n1);
    else if( color == 3 )
      ofield->set_value( (double) plane, n1);
    else if( color == 4 )
      ofield->set_value( (double) bin, n1);
    else if( color == 5 )
      ofield->set_value( (double) i, n1);
    else
      ofield->set_value( (double) color_value, n1);
  }
}

template< class IFIELD, class OFIELD, class PCFIELD >
void
StreamlineAnalyzerAlgoTVector<IFIELD, OFIELD, PCFIELD>::
loadCurve( FieldHandle &field_h,
           vector < pair< Point, double > > &nodes,
           unsigned int nplanes,
           unsigned int nbins,
           unsigned int nnodes,
           unsigned int plane,
           unsigned int bin,
           unsigned int color,
           double color_value ) {

  if( nnodes < 2 )
    return;

  StreamlineAnalyzerAlgo::CVField *ofield =
    (StreamlineAnalyzerAlgo::CVField *) field_h.get_rep();
  typename StreamlineAnalyzerAlgo::CVField::mesh_handle_type omesh =
    ofield->get_typed_mesh();
  typename StreamlineAnalyzerAlgo::CVField::mesh_type::Node::index_type n1, n2;

  Vector tangent(1,1,1);

  unsigned int i = 0;

  n1 = omesh->add_node(nodes[i].first);
  ofield->resize_fdata();
  tangent = ((Vector) nodes[i].first - (Vector) nodes[i+1].first);
  tangent.safe_normalize();

  if( color == 0 )
    tangent *= nodes[0].second;
  else if( color == 1 )
    tangent *= color_value;
  else if( color == 2 )
    tangent *= (i*nbins+bin);
  else if( color == 3 )
    tangent *= plane;
  else if( color == 4 )
    tangent *= bin;
  else if( color == 5 )
    tangent *= i;
  else
    tangent *= color_value;
 
  ofield->set_value( tangent, n1);
  
  for( i=1; i<nnodes-1; i++ ) {
    n2 = omesh->add_node(nodes[i].first);
    ofield->resize_fdata();

    tangent = ( ((Vector) nodes[i].first - (Vector) nodes[i+1].first) + 
                ((Vector) nodes[i-1].first - (Vector) nodes[i].first) )  / 2.0;
    tangent.safe_normalize();
    if( color == 0 )
      tangent *= nodes[0].second;
    else if( color == 1 )
      tangent *= color_value;
    else if( color == 2 )
      tangent *= (i*nbins+bin);
    else if( color == 3 )
      tangent *= plane;
    else if( color == 4 )
      tangent *= bin;
    else if( color == 5 )
      tangent *= i;
    else
      tangent *= color_value;
 
    ofield->set_value( tangent, n2);
    
    omesh->add_edge(n1, n2);
              
    n1 = n2;
  }

  n2 = omesh->add_node(nodes[i].first);
  ofield->resize_fdata();

  tangent = ((Vector) nodes[i-1].first - (Vector) nodes[i].first);
  tangent.safe_normalize();
  if( color == 0 )
    tangent *= nodes[0].second;
  else if( color == 1 )
    tangent *= color_value;
  else if( color == 2 )
    tangent *= (i*nbins+bin);
  else if( color == 3 )
    tangent *= plane;
  else if( color == 4 )
    tangent *= bin;
  else if( color == 5 )
    tangent *= i;
  else
    tangent *= color_value;
 
  ofield->set_value( tangent, n2);
    
  omesh->add_edge(n1, n2);            
}


template< class IFIELD, class OFIELD, class PCFIELD >
void
StreamlineAnalyzerAlgoTVector<IFIELD, OFIELD, PCFIELD>::
loadSurface( FieldHandle &field_h,
             vector < pair< Point, double > > &nodes,
             unsigned int nplanes,
             unsigned int nbins,
             unsigned int nnodes,
             unsigned int plane,
             unsigned int bin,
             unsigned int color,
             double color_value ) {
  
  if( nnodes < 2 )
    return;

  StreamlineAnalyzerAlgo::SQSVField *ofield =
    (StreamlineAnalyzerAlgo::SQSVField *) field_h.get_rep();
  typename StreamlineAnalyzerAlgo::SQSVField::mesh_handle_type omesh =
    ofield->get_typed_mesh();
  typename StreamlineAnalyzerAlgo::SQSVField::mesh_type::Node::index_type n1;

  n1.mesh_ = omesh.get_rep();

  n1.j_ = nplanes * bin + plane;

  unsigned int i = 0;

  n1.i_ = i;

  omesh->set_point(nodes[i].first, n1);

  Vector tangent = ((Vector) nodes[i].first - (Vector) nodes[i+1].first);
  tangent.safe_normalize();
  if( color == 0 )
    tangent *= nodes[i].second;
  else if( color == 1 )
    tangent *= color_value;
  else if( color == 2 )
    tangent *= (i*nbins+bin);
  else if( color == 3 )
    tangent *= plane;
  else if( color == 4 )
    tangent *= bin;
  else if( color == 5 )
    tangent *= i;
  else
    tangent *= color_value;
 
  ofield->set_value( (Vector) tangent, n1);

  for( unsigned int i=1; i<nnodes-1; i++ ) {

    n1.i_ = i;

    omesh->set_point(nodes[i].first, n1);

    tangent = ( ((Vector) nodes[i].first - (Vector) nodes[i+1].first) + 
                ((Vector) nodes[i-1].first - (Vector) nodes[i].first) )  / 2.0;
    tangent.safe_normalize();
    if( color == 0 )
      tangent *= nodes[i].second;
    else if( color == 1 )
      tangent *= color_value;
    else if( color == 2 )
      tangent *= (i*nbins+bin);
    else if( color == 3 )
      tangent *= plane;
    else if( color == 4 )
      tangent *= bin;
    else if( color == 5 )
      tangent *= i;
    else
      tangent *= color_value;
 
    ofield->set_value( (Vector) tangent, n1);
  }

  n1.i_ = i;

  omesh->set_point(nodes[i].first, n1);

  tangent = ((Vector) nodes[i-1].first - (Vector) nodes[i].first);
  tangent.safe_normalize();
  if( color == 0 )
    tangent *= nodes[i].second;
  else if( color == 1 )
    tangent *= color_value;
  else if( color == 2 )
    tangent *= (i*nbins+bin);
  else if( color == 3 )
    tangent *= plane;
  else if( color == 4 )
    tangent *= bin;
  else if( color == 5 )
    tangent *= i;
  else
    tangent *= color_value;
 
  ofield->set_value( (Vector) tangent, n1);
}


template< class IFIELD, class OFIELD, class PCFIELD >
void
StreamlineAnalyzerAlgoT<IFIELD, OFIELD, PCFIELD>::
execute(FieldHandle& ifield_h,
        FieldHandle& ofield_h,
        FieldHandle& ipccfield_h,
        FieldHandle& opccfield_h,
        FieldHandle& ipcsfield_h,
        FieldHandle& opcsfield_h,
        vector< double > &planes,
        unsigned int color,
        unsigned int showIslands,
        unsigned int islandCentroids,
        unsigned int overlaps,
        unsigned int maxWindings,
        unsigned int override,
        unsigned int order,
        vector< pair< unsigned int, unsigned int > > &topology)
{
  IFIELD *ifield = (IFIELD *) ifield_h.get_rep();
  typename IFIELD::mesh_handle_type imesh = ifield->get_typed_mesh();

  bool poincareField =
    (ifield_h->get_type_description(Field::MESH_TD_E)->get_name().find("PointCloud") !=
     string::npos );


  typename OFIELD::mesh_type *omesh = scinew typename OFIELD::mesh_type();
  OFIELD *ofield = scinew OFIELD(omesh);

  ofield_h = FieldHandle(ofield);

  bool curveField =
    (ofield->get_type_description(Field::MESH_TD_E)->get_name().find("Curve") !=
     string::npos );


  // Point Cloud Field of centroids.
  vector< vector < Vector > > baseCentroids;
  vector < unsigned int > baseCentroidsWinding;

  if( ipccfield_h.get_rep() ) {
    PCFIELD *ipcfield = (PCFIELD *) ipccfield_h.get_rep();
    typename PCFIELD::mesh_handle_type ipcmesh = ipcfield->get_typed_mesh();

    typename PCFIELD::fdata_type::iterator dataItr = ipcfield->fdata().begin();
    typename PCFIELD::mesh_type::Node::iterator inodeItr, inodeEnd;

    ipcmesh->begin( inodeItr );
    ipcmesh->end( inodeEnd );

    Point pt;

    while (inodeItr != inodeEnd) {
      ipcmesh->get_center(pt, *inodeItr);

      // Store the base centroid for each point.
      vector < Vector > baseCentroid;
      baseCentroid.push_back( (Vector) pt );
      baseCentroids.push_back( baseCentroid );
      baseCentroidsWinding.push_back( (unsigned int) *dataItr );

//       cerr << "input " << pt << endl;

      ++inodeItr;
      ++dataItr;
    }
  }

  typename PCFIELD::mesh_type *opccmesh = scinew typename PCFIELD::mesh_type();
  PCFIELD *opccfield = scinew PCFIELD(opccmesh);

  opccfield_h = FieldHandle(opccfield);

  // Point Cloud Field of Separatrices.
  vector< vector < Vector > > baseSeparatrices;
  vector < unsigned int > baseSeparatricesWinding;

  if( ipcsfield_h.get_rep() ) {
    PCFIELD *ipcfield = (PCFIELD *) ipcsfield_h.get_rep();
    typename PCFIELD::mesh_handle_type ipcmesh = ipcfield->get_typed_mesh();

    typename PCFIELD::fdata_type::iterator dataItr = ipcfield->fdata().begin();
    typename PCFIELD::mesh_type::Node::iterator inodeItr, inodeEnd;

    ipcmesh->begin( inodeItr );
    ipcmesh->end( inodeEnd );

    Point pt;

    while (inodeItr != inodeEnd) {
      ipcmesh->get_center(pt, *inodeItr);

      vector < Vector > baseSeparatrix;
      baseSeparatrix.push_back( (Vector) pt );
      baseSeparatrices.push_back( baseSeparatrix );
      baseSeparatricesWinding.push_back( (unsigned int) *dataItr );

//       cerr << "input " << pt << endl;

      ++inodeItr;
      ++dataItr;
    }
  }

  typename PCFIELD::mesh_type *opcsmesh = scinew typename PCFIELD::mesh_type();
  PCFIELD *opcsfield = scinew PCFIELD(opcsmesh);

  opcsfield_h = FieldHandle(opcsfield);


  // Input iterators
  typename IFIELD::fdata_type::iterator dataItr = ifield->fdata().begin();

  typename IFIELD::mesh_type::Node::iterator inodeItr, inodeEnd;
  typename IFIELD::mesh_type::Node::index_type inodeNext;
  vector< typename IFIELD::mesh_type::Node::index_type > inodeGlobalStart;

  imesh->begin( inodeItr );
  imesh->end( inodeEnd );
  dataItr = ifield->fdata().begin();

  if(inodeItr == inodeEnd) return;

  topology.clear();

  vector< unsigned int > windings;
  vector< unsigned int > twists;
  vector< unsigned int > skips;
  vector< unsigned int > islands;
  vector< float > avenodes;

  Point lastPt, currPt;
  double lastAng, currAng;
  bool CCWstreamline;

  if( !poincareField ) {
    // Get the direction of the streamline winding.
    imesh->get_center(lastPt, *inodeItr);
    lastAng = atan2( lastPt.y(), lastPt.x() );
    
    ++inodeItr;
    if(inodeItr == inodeEnd) return;

    imesh->get_center(currPt, *inodeItr);
    currAng = atan2( currPt.y(), currPt.x() );

    CCWstreamline = (lastAng < currAng);
  }

  unsigned int count = 0;
  unsigned int index = 0;
  
  // First find out how many windings it takes to get back to the
  // starting point at the Phi = 0 plane. 
  double plane = 0;

  vector< Point > points;

  imesh->begin( inodeItr );
  inodeNext = *inodeItr;

  dataItr = ifield->fdata().begin();
  index = 9999999;

  while (inodeItr != inodeEnd) {

    // Prep for a new set of points from a streamline or poincare points.
    if( ( poincareField && *dataItr  != index    ) ||
        (!poincareField && *inodeItr == inodeNext) ) {

//      cerr << "Starting new streamline winding "
//         << count << "  " << *inodeItr << endl;

      points.clear();

      if( poincareField ) {
        // The index for this set of poincare points.
        index = (unsigned int) *dataItr;

      } else {
        imesh->get_center(lastPt, *inodeItr);
        lastAng = atan2( lastPt.y(), lastPt.x() );      
        ++inodeItr;
        ++dataItr;

        // The index of the node from the next streamline.
        ostringstream str;
        str << "Streamline " << count+1 << " Node Index";
        if( !ifield->get_property( str.str(), inodeNext ) )
          inodeNext = *inodeEnd;
      }
    }

    // Get the current point.
    imesh->get_center(currPt, *inodeItr);

    if( poincareField ) {

      points.push_back( currPt );

    } else {
      currAng = atan2( currPt.y(), currPt.x() );

      // First look at only points that are in the correct plane.
      if( ( CCWstreamline && lastAng < plane && plane <= currAng) ||
          (!CCWstreamline && currAng < plane && plane <= lastAng) ) {
        double t;

        if( fabs(currAng-lastAng) > 1.0e-12 )
          t = (plane-lastAng) / (currAng-lastAng);
        else
          t = 0;

        Point point = FLlib.interpert( lastPt, currPt, t );

        // Save the point found before the zero plane.
        if( points.size() == 0 ) {
          --inodeItr;

          inodeGlobalStart.push_back( *inodeItr );

          ++inodeItr;
        }

        points.push_back( point );
      }

      lastPt  = currPt;
      lastAng = currAng;
    }
    
    ++inodeItr;
    ++dataItr;

    // If about to start the next streamline figure out the number of
    // windings, twist, skips, islands, etc.
    if( ( poincareField && *dataItr  != index    ) ||
        (!poincareField && *inodeItr == inodeNext) ) {

      unsigned int winding, twist, skip, type, island, windingNextBest;
      float avenode;

      if( override ) {
        Vector centroid;

        bool groupCCW;

        winding = override;

        // Do this to get the basic values of the fieldline.
        FLlib.basicChecks( points, centroid,
                           winding, twist, skip, type,
                           island, avenode, groupCCW, windingNextBest );

      } else {

        FLlib.fieldlineProperties( points, maxWindings,
                                   winding, twist, skip, type,
                                   island, avenode, windingNextBest );

        // This is for island analysis - if the initial centroid
        // locations are provided which have the winding value from
        // the island then make sure the new analysis gives the same
        // winding.

        if( baseCentroidsWinding.size() ) {

          // Iterative processing of the centroids so the desired
          // winding value is from the base centroid data. 
          unsigned int desiredWinding = baseCentroidsWinding[windings.size()];

          if( desiredWinding && winding != desiredWinding )
            winding = 0;
        }

        cerr << "End of streamline " << count
             << "  windings " << winding 
             << "  type " << type
             << "  twists " << twist
             << "  skip "   << skip
             << "  islands " << island
             << "  windingNextBest " << (windingNextBest>0 ? windingNextBest : 0)
             << endl;
      }

      windings.push_back( winding );
      twists.push_back( twist );
      skips.push_back( skip );
      islands.push_back( island );
      avenodes.push_back( avenode );

      count++;
    }
  }


  if( poincareField ) {
    imesh->begin( inodeItr );
    dataItr = ifield->fdata().begin();

    index = (unsigned int) *dataItr;
  }

  // Now bin the points.
  for( unsigned int c=0; c<windings.size(); c++ ) {

    cerr << c << " STARTING" << endl;

    unsigned int winding = windings[c];
    unsigned int twist   = twists[c];
    unsigned int skip    = skips[c];
    unsigned int island  = islands[c];
    unsigned int nnodes  = (unsigned int) avenodes[c];

    bool groupCCW;
    bool completeIslands = true;
    bool nearPI;

    if( winding == 0 ) {
      cerr << c << " SKIPPING WINDING OF 0" << endl;

      pair< unsigned int, unsigned int > topo( 0, 0 );
      topology.push_back(topo);

      index = (unsigned int) *dataItr;

      while ( inodeItr != inodeEnd &&
              (( poincareField && *dataItr  == index ) ||
               (!poincareField && *inodeItr != inodeNext)) ) {
        ++inodeItr;
        ++dataItr;
      }

      continue;
    }

    vector< vector < pair< Point, double > > > bins[planes.size()];
    
    for( unsigned int p=0; p<planes.size(); p++ ) {

      bins[p].clear();

      int bin = 0;

      if( poincareField ) {
        plane = planes[p];

        index = (unsigned int) *dataItr;

      } else {
        // Go through the planes in the same direction as the streamline.
        if( CCWstreamline )
          plane = planes[p];
        else
          plane = planes[planes.size()-1-p];
        
        // The plane goes from 0 to 2PI but it is checked against atan2
        // which returns -PI to PI so adjust accordingly.
        if( plane > M_PI )
          plane -= 2.0 * M_PI;
        
        // If the plane is near PI which is where the through zero
        // happens with atan2 adjust the angle so that the through zero
        // is at 0 - 2PI instead.
        nearPI = (fabs(plane-M_PI) < 1.0e-2);
      
//      cerr << "Plane " << p << " is " << plane << endl;
        
        // Ugly but it is necessary to start at the same place each time.
        imesh->begin( inodeItr );
        dataItr = ifield->fdata().begin();

        // Skip all of the points between the beginning and the first
        // point found before (CCW) or after (CW) the zero plane.
        while( *inodeItr != inodeGlobalStart[c] ) {
          ++inodeItr;
          ++dataItr;
        }

        imesh->get_center(lastPt, *inodeItr);
        lastAng = atan2( lastPt.y(), lastPt.x() );

        // If the plane is near PI which where through zero happens with
        // atan2 adjust the angle so that the through zero is at 0 - 2PI
        // instead.
        if( nearPI && lastAng < 0.0) lastAng += 2 * M_PI;
        
        ++inodeItr;
        ++dataItr;
      
        ostringstream str;
        str << "Streamline " << c+1 << " Node Index";

        if( !ifield->get_property( str.str(), inodeNext ) )
          inodeNext = *inodeEnd;
      }

//    cerr << "Starting new streamline binning " << c << endl;

      while ( inodeItr != inodeEnd &&
              (( poincareField && *dataItr  == index ) ||
               (!poincareField && *inodeItr != inodeNext)) ) {

        imesh->get_center(currPt, *inodeItr);

        if( poincareField ) {

          if( bins[p].size() <= (unsigned int) bin ) {
            vector< pair<Point, double> > sub_bin;
          
            sub_bin.push_back( pair<Point, double>(currPt, *dataItr) );

            bins[p].push_back( sub_bin );

          } else {
            bins[p][bin].push_back( pair<Point, double>(currPt, *dataItr) );
          }

          bin = (bin + 1) % winding;

        } else {

          currAng = atan2( currPt.y(), currPt.x() );

          // If the plane is near PI which where through zero happens with
          // atan2 adjust the angle so that the through zero is at 0 - 2PI
          // instead.
          if( nearPI && currAng < 0.0) currAng += 2 * M_PI;

          // First look at only points that are in the correct plane.
          if( ( CCWstreamline && lastAng < plane && plane <= currAng) ||
              (!CCWstreamline && currAng < plane && plane <= lastAng) ) {
            double t;

            if( fabs(currAng-lastAng) > 1.0e-12 )
              t = (plane-lastAng) / (currAng-lastAng);
            else
              t = 0;
          
            Point point = FLlib.interpert( lastPt, currPt, t );

            if( bins[p].size() <= (unsigned int) bin ) {
              vector< pair<Point, double> > sub_bin;
          
              sub_bin.push_back( pair<Point, double>(point, *dataItr) );

              bins[p].push_back( sub_bin );

            } else {
              bins[p][bin].push_back( pair<Point, double>(point, *dataItr) );
            }

            bin = (bin + 1) % winding;
          }

          lastPt  = currPt;
          lastAng = currAng;
        }

        ++inodeItr;
        ++dataItr;
      }
    }
    
    bool VALID = true;

    // Sanity check
    for( unsigned int p=0; p<planes.size(); p++ ) {
      for( unsigned int i=0; i<winding; i++ ) {

        if( nnodes > bins[p][i].size() )
          nnodes = bins[p][i].size();

        if( bins[p][i].size() < 1 ) {
          cerr << "INVALID - Plane " << p
               << " bin  " << i
               << " number of points " << bins[p][i].size()
               << endl;

          VALID = false;

          return;
        }
      }
    }

    // Get the rest of the info only from the phi = zero plane.
    unsigned int p;

    if( CCWstreamline )
      p = 0;
    else
      p = planes.size()-1;

    // Get the centroid of each group and all groups.
    Vector globalCentroid(0,0,0);
    vector< Vector > localCentroids;
    vector< Vector > localSeparatrices[2];

    localCentroids.resize(winding);
    localSeparatrices[0].resize(winding);
    localSeparatrices[1].resize(winding);

    for( unsigned int i=0; i<winding; i++ ) {
      localCentroids[i] = Vector(0,0,0);
      
      for( unsigned int j=0; j<bins[p][i].size(); j++ ) 
        localCentroids[i] += (Vector) bins[p][i][j].first;

      if( bins[p][i].size() ) {
        localCentroids[i] /= (double) bins[p][i].size();

        globalCentroid += localCentroids[i];
      }
    }

    globalCentroid /= winding;

    // Get the direction of the points within a group.
    Vector v0 = (Vector) bins[p][0][0].first - globalCentroid;
    Vector v1 = (Vector) bins[p][0][1].first - globalCentroid;

    groupCCW = (FLlib.ccw( v0, v1 ) == 1);

//    cerr << 0.0<< "  " << groupCCW << endl;

    if( island ) {
      for( unsigned int i=0; i<winding; i++ ) {
        unsigned int startIndex;
        unsigned int middleIndex;
        unsigned int stopIndex;
        unsigned int nodes;
        
        vector< Point > points;
        points.resize( bins[p][i].size() );

        for( unsigned int j=0; j<bins[p][i].size(); j++ )
          points[j] = bins[p][i][j].first;
        
        unsigned int turns =
          FLlib.islandProperties( points, globalCentroid,
                                  startIndex, middleIndex, stopIndex, nodes );
        
        cerr << "Island " << i  << "   "
             << "Turns " << turns  << "   "
             << "nodes " << nodes  << "   "
             << "Indexes "
             << startIndex  << "  "
             << middleIndex << "  "
             << stopIndex   << endl;


//      if( turns < 3 )
//      completeIslands = false;

        if( turns >= 2 ) {
//        localSeparatrices[0][i] = (Vector) bins[p][i][startIndex].first;
//        localSeparatrices[1][i] = (Vector) bins[p][i][middleIndex].first;
        }

        if( turns == 3 ) {
          unsigned int index0 = (middleIndex - startIndex ) / 2;
          unsigned int index1 = (  stopIndex - middleIndex) / 2;

          //      cerr << "Indexes mid " << nodes << " nodes "
          //           << "  " << ( startIndex + index0)%nodes 
          //           << "  " << (middleIndex - index0)%nodes
          //           << "  " << (middleIndex + index1)%nodes
          //           << "  " << (  stopIndex - index1)%nodes << endl;

          localCentroids[i] =
            ( (Vector) bins[p][i][( startIndex + index0)%nodes].first + 
              (Vector) bins[p][i][(middleIndex - index0)%nodes].first +

              (Vector) bins[p][i][(middleIndex + index1)%nodes].first + 
              (Vector) bins[p][i][(  stopIndex - index1)%nodes].first ) / 4.0;
        } else if( turns == 2 ) {
          unsigned int index0 = (middleIndex - startIndex ) / 2;

          //      cerr << "Indexes mid " << nodes << " nodes "
          //           << "  " << ( startIndex + index0)%nodes 
          //           << "  " << (middleIndex - index0)%nodes
          //           << "  " << (middleIndex + index1)%nodes
          //           << "  " << (  stopIndex - index1)%nodes << endl;

          localCentroids[i] =
            ( (Vector) bins[p][i][( startIndex + index0)%nodes].first + 
              (Vector) bins[p][i][(middleIndex - index0)%nodes].first ) / 2.0;

        } else if( turns == 1 ) {
          unsigned int index0 = (stopIndex - startIndex ) / 2;

          //      cerr << "Indexes mid " << nodes << " nodes "
          //           << "  " << ( startIndex + index0)%nodes 
          //           << "  " << (middleIndex - index0)%nodes
          //           << "  " << (middleIndex + index1)%nodes
          //           << "  " << (  stopIndex - index1)%nodes << endl;

          localCentroids[i] =
            ( (Vector) bins[p][i][(startIndex + index0)%nodes].first + 
              (Vector) bins[p][i][( stopIndex - index0)%nodes].first ) / 2.0;
        }
        
//      // Get the principal axes of the island.
//      Vector localCentroid(0,0,0);

//      for( unsigned int j=0; j<bins[p][i].size(); j++ )
//        localCentroid += (Vector) bins[p][i][j].first;

//      localCentroid /= (float) bins[p][i].size();

//      float Ixx = 0.0;
//      float Ixz = 0.0;
//      float Izz = 0.0;

//      double maxDist = 0;

//      for( unsigned int j=0; j<bins[p][i].size(); j++ ) {

//        Vector vec = (Vector) bins[p][i][j].first - localCentroid;

//        if( maxDist < vec.length() )
//          maxDist = vec.length();

//        Ixx += vec.z()*vec.z();
//        Ixz -= vec.x()*vec.z();
//        Izz += vec.x()*vec.x();
//      }

//      float alpha = atan( 2.0 * Ixz / (Ixx - Izz) ) / 2.0;

// //       cerr << "PRINCIPAL AXES " << alpha * 180.0 / M_PI << "    "
// //      << Ixx + Ixz * sin(alpha       )/cos(alpha       ) << "    "
// //      << Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) << endl;

//      if( Ixx + Ixz * sin(alpha       )/cos(alpha       ) >
//          Izz + Ixz * cos(alpha+M_PI/2)/sin(alpha+M_PI/2) )
//        localCentroid += Vector(  cos(alpha), 0, sin(alpha) ) * maxDist;
//      else
//        localCentroid += Vector( -sin(alpha), 0, cos(alpha) ) * maxDist;

//      localCentroids[i] = localCentroid;

      }
    }

    for( unsigned int p=0; p<planes.size(); p++ ) {
      if( overlaps == 1 || overlaps == 3 )
        FLlib.removeOverlap( bins[p], nnodes, winding, twist, skip, island );
      
      if( overlaps == 2 )
        FLlib.mergeOverlap( bins[p], nnodes, winding, twist, skip, island );
      else if( overlaps == 3 )
        FLlib.smoothCurve( bins[p], nnodes, winding, twist, skip, island );

      // Sanity check
      for( unsigned int i=0; i<winding; i++ ) {

        if( nnodes > bins[p][i].size() )
          nnodes = bins[p][i].size();

        if( bins[p][i].size() < 1 ) {
          cerr << "INVALID - Plane " << p
               << " bin  " << i
               << " number of points " << bins[p][i].size()
               << endl;

          VALID = false;

          return;
        }
          
//      cerr << "Surface " << c
//           << " plane " << p
//           << " bin " << i
//           << " base number of nodes " << nnodes
//           << " number of points " << bins[p][i].size()
//           << endl;
      }
    }

    cerr << "Surface " << c << " is a "
         << winding << ":" << twist << " surface ("
         << (double) winding / (double) twist << ") ";
    
    if( island > 0 ) 
      cerr << "that contains " << island << " islands"
           << (completeIslands ? " (Complete)" : "");
    
    cerr << " and has " << nnodes << " nodes"
         << endl;
        
    if( island && island != winding ) {
      cerr << "WARNING - The island count does not match the winding count" 
           << endl;
    }
    
    // Record the topology.
    pair< unsigned int, unsigned int > topo( winding, twist );
    topology.push_back(topo);

    if( !curveField ) {

      vector< unsigned int > dims;
      
      dims.resize(2);
      
      dims[0] = nnodes;
      dims[1] = (planes.size()+1) * winding;
      
      ((SQSMesh *) omesh)->set_dim( dims );

      ofield->resize_fdata();

      cerr << "Creating a surface of " << dims[0] << "  " << dims[1] << endl;
    }

    double color_value = 0;

    if( color == 1 )
      color_value = c;
    else if( color == 6 )
      color_value = winding;
    else if( color == 7 )
      color_value = twist;
    else if( color == 8 )
      color_value = (double) winding / (double) twist;


    if( island ) {

      if( completeIslands ) {

        if( baseCentroids.size() ) {

          unsigned int cc = 0;

          // Find the bounds where the islands are for this winding.
          while( cc < windings.size() ) {
            if( cc <= c && c < cc+baseCentroidsWinding[cc] )
              break;
            else
              cc += baseCentroidsWinding[cc];
          }

          cerr << "Searching winding " << baseCentroidsWinding[cc] << endl;

          for( unsigned int i=0; i<winding; i++ ) {
            
            unsigned int index;
            double mindist = 1.0e12;
            
            for( unsigned int j=cc; j<cc+winding; j++ ) {

              double dist = (localCentroids[i] - baseCentroids[j][0]).length();
              
              if( mindist > dist ) {
                mindist = dist;
                index = j;
              }
            }

            cerr << cc << "  " << i << "  "
                 << index << " index " << localCentroids[i] << endl;
            
            baseCentroids[index].push_back( localCentroids[i] );              
          }

        } else {

          for( unsigned int i=0; i<winding; i++ ) {
            // Centroids
//          cerr << i << "  " << localCentroids[i] << endl;

            typename PCFIELD::mesh_type::Node::index_type n =
              opccmesh->add_node((Point) localCentroids[i]);
            opccfield->resize_fdata();
            opccfield->set_value( (double) i, n );

            // Separatrices
            unsigned int j = (i+skip) % winding;

            unsigned int ii, jj;

            if( (localSeparatrices[0][i] - localSeparatrices[0][j]).length() <
                (localSeparatrices[0][i] - localSeparatrices[1][j]).length() )
              jj = 0;
            else
              jj = 1;

            if( (localSeparatrices[0][i] - localSeparatrices[jj][j]).length() <
                (localSeparatrices[1][i] - localSeparatrices[jj][j]).length() )
              ii = 0;
            else
              ii = 1;

            n = opcsmesh->add_node((Point) ((localSeparatrices[ii][i] +
                                             localSeparatrices[jj][j])/2.0));

            opcsfield->resize_fdata();
            opcsfield->set_value( (double) i, n);

//          n = opcsmesh->add_node((Point) localSeparatrices[0][i]);
//          opcsfield->resize_fdata();
//          opcsfield->set_value(0, n);

//          n = opcsmesh->add_node((Point) localSeparatrices[1][j]);
//          opcsfield->resize_fdata();
//          opcsfield->set_value(1, n);

//          cerr << c << "  Separatrices " << i << "  " << j << endl;
          }
        }
      }
    } else { // Surface
    }

    if( !showIslands || ( showIslands && island ) ) {

      // Add the points into the return field.
      for( unsigned int p=0; p<planes.size(); p++ ) {
        for( unsigned int i=0; i<winding; i++ ) {

          lock.lock();

          if( curveField ) {
//          cerr << "Loading curve " << p * winding + i
//               << " plane " << planes.size() << " winding " << i;

            loadCurve( ofield_h, bins[p][i],
                       planes.size(), winding,
                       bins[p][i].size(), p, i,
                       color, color_value );

//          cerr << " done";

          } else {
//          cerr << "Loading surface " << p * winding + i
//               << " plane " << planes.size() << " winding " << i;

            if( p == planes.size()-1 ) {
              unsigned int j = (i-1 + winding) % winding;

              loadSurface( ofield_h, bins[p][i],
                           planes.size()+1, winding, nnodes, p, j,
                           color, color_value );
            } else {
              loadSurface( ofield_h, bins[p][i],
                           planes.size()+1, winding, nnodes, p, i,
                           color, color_value );
            }

//          cerr << " done";
          }

          lock.unlock();

//        cerr << " unlocked " << endl;
        }
      }

      // For a surface add in the first set again so that the surface
      // is complete. However because the complete surface has
      if( !curveField ) {
        for( unsigned int i=0; i<winding; i++ ) {
          lock.lock();

          unsigned int j = (i-1 + winding) % winding;

//        cerr << "Loading surface " << planes.size() * winding + i
//             << " plane " << planes.size() << " winding " << j;

          loadSurface( ofield_h, bins[0][i],
                       planes.size()+1, winding, nnodes,
                       planes.size(), j, color, color_value );
        
//        cerr << " done" << endl;

          lock.unlock();
        }
      }
    }

    for( unsigned int p=0; p<planes.size(); p++ ) {
        for( unsigned int i=0; i<winding; i++ ) {
          bins[p][i].clear();
        }

        bins[p].clear();
    }

//     cerr << c << " DONE" << endl;
  }

//   cerr << "DONE DONE" << endl;

  if( baseCentroids.size() ) {

    if( islandCentroids ) {

      typename PCFIELD::mesh_type::Node::index_type n;

      for( unsigned int i=0; i<baseCentroids.size(); i++ ) {

        if( baseCentroids[i].size() ) {
          Vector tmpCentroid(0,0,0);
        
          for( unsigned int j=1; j<baseCentroids[i].size(); j++ )
            tmpCentroid += baseCentroids[i][j];
        
          tmpCentroid /= (double) (baseCentroids[i].size() - 1);
        
          //    cerr << tmpCentroid << endl;

          n = opccmesh->add_node((Point) tmpCentroid);
        } else {
          n = opccmesh->add_node((Point) baseCentroids[i][0]);
        }

        opccfield->resize_fdata();
//      opccfield->set_value( (double) windings[index], n );
      }
    } else { // Produce seed to show the NULL fieldline.
      typename PCFIELD::mesh_type::Node::index_type n;

      vector< Vector > newCentroid;
      vector< double > newStdDev;

      newCentroid.resize( baseCentroids.size() );
      newStdDev.resize  ( baseCentroids.size() );

      for( unsigned int i=0; i<baseCentroids.size(); i++ ) {

        if( baseCentroids[i].size() ) {

          newCentroid[i] = Vector(0,0,0);
          for( unsigned int j=1; j<baseCentroids[i].size(); j++ )
            newCentroid[i] += baseCentroids[i][j];
        
          newCentroid[i] /= (double) (baseCentroids[i].size() - 1);

          newStdDev[i] = 0.0;
          for( unsigned int j=1; j<baseCentroids[i].size(); j++ ) {
            cerr << "length " << i << "  " << j << "  "
                 << (baseCentroids[i][j]-newCentroid[i]).length2() << endl;
            newStdDev[i] += (baseCentroids[i][j]-newCentroid[i]).length2();
          }

          newStdDev[i] = sqrt( newStdDev[i] /
                               (double) (baseCentroids[i].size() - 2) );
        }
      }

      unsigned int cc = 0;

      double minSD = 1.0e12;
      unsigned int index;

      while( cc < windings.size() ) {
        cerr << "Searching winding " << cc << endl;

        for( unsigned int i=cc; i<cc+windings[cc]; i++ ) {

          cerr << "Std dev. " << i << "  " << newStdDev[i] << endl;

          if( minSD > newStdDev[i] ) {
            minSD = newStdDev[i];
            index = i;
          }
        }

        cerr << "New centroid " << newCentroid[index]
             << " index " << index << endl;

        typename PCFIELD::mesh_type::Node::index_type n =
          opccmesh->add_node((Point) newCentroid[index]);
        opccfield->resize_fdata();
        opccfield->set_value( (double) windings[index], n );

        cc += windings[cc];

        minSD = 1.0e12;
      }
    }
  }
}


} // End namespace FusionPSE

#endif // StreamlineAnalyzer_h
