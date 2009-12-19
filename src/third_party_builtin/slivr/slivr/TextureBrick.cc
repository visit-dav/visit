//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2008 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : TextureBrick.cc
//    Author : Milan Ikits
//    Date   : Wed Jul 14 16:03:05 2004

#include <math.h>

#include <slivr/TextureBrick.h>
#include <slivr/Utils.h>
#include <utility>
#include <iostream>

using namespace std;

namespace SLIVR {

TextureBrick::TextureBrick (Nrrd* n0, Nrrd* n1,
			    int nx, int ny, int nz, int nc, int* nb,
			    int ox, int oy, int oz,
			    int mx, int my, int mz,
			    const BBox& bbox, const BBox& tbox)
  : nx_(nx), ny_(ny), nz_(nz), nc_(nc), ox_(ox), oy_(oy), oz_(oz),
    mx_(mx), my_(my), mz_(mz), bbox_(bbox), tbox_(tbox), dirty_(true)
{
  for (int c=0; c<nc_; c++)
  {
    nb_[c] = nb[c];
  }

  compute_edge_rays(bbox_, edge_);
  compute_edge_rays(tbox_, tex_edge_);

  data_[0] = n0;
  data_[1] = n1;
}


TextureBrick::~TextureBrick()
{
  // Creator of the brick owns the nrrds.  
  // This object never deletes that memory.
  data_[0] = 0;
  data_[1] = 0;
}

  /* The cube is numbered in the following way 
     
Corners:

       2________6        y
      /|        |        |  
     / |       /|        |
    /  |      / |        |
   /   0_____/__4        |
  3---------7   /        |_________ x
  |  /      |  /         /
  | /       | /         /
  |/        |/         /
  1_________5         /
                     z  

Edges:

       ,____1___,        y
      /|        |        |  
     / 0       /|        |
    9  |     10 2        |
   /   |__3__/__|        |
  /_____5___/   /        |_________ x
  |  /      | 11         /
  4 8       6 /         /
  |/        |/         /
  |____7____/         /
                     z
  */

void
TextureBrick::compute_edge_rays(BBox &bbox, vector<Ray> &edges) const {
  // set up vertices
  Point corner[8];
  const Point &pmin(bbox.min());
  const Point &pmax(bbox.max());
  corner[0] = pmin;
  corner[1] = Point(pmin.x(), pmin.y(), pmax.z());
  corner[2] = Point(pmin.x(), pmax.y(), pmin.z());
  corner[3] = Point(pmin.x(), pmax.y(), pmax.z());
  corner[4] = Point(pmax.x(), pmin.y(), pmin.z());
  corner[5] = Point(pmax.x(), pmin.y(), pmax.z());
  corner[6] = Point(pmax.x(), pmax.y(), pmin.z());
  corner[7] = pmax;

  edges.resize(12);
  // set up edges
  edges[0] = Ray(corner[0], corner[2] - corner[0]);
  edges[1] = Ray(corner[2], corner[6] - corner[2]);
  edges[2] = Ray(corner[4], corner[6] - corner[4]);
  edges[3] = Ray(corner[0], corner[4] - corner[0]);
  edges[4] = Ray(corner[1], corner[3] - corner[1]);
  edges[5] = Ray(corner[3], corner[7] - corner[3]);
  edges[6] = Ray(corner[5], corner[7] - corner[5]);
  edges[7] = Ray(corner[1], corner[5] - corner[1]);
  edges[8] = Ray(corner[0], corner[1] - corner[0]);
  edges[9] = Ray(corner[2], corner[3] - corner[2]);
  edges[10] = Ray(corner[6], corner[7] - corner[6]);
  edges[11] = Ray(corner[4], corner[5] - corner[4]);
}


// compute polygon of edge plane intersections
void
TextureBrick::compute_polygon(const Ray& view, double t,
			      vector<float>& vertex, vector<float>& texcoord,
			      vector<int>& size)
{
  compute_polygons(view, t, t, 1.0, vertex, texcoord, size);
}


void
TextureBrick::compute_polygons(const Ray& view, double dt,
			       vector<float>& vertex, vector<float>& texcoord,
			       vector<int>& size)
{
  const Point &pmin(bbox_.min());
  const Point &pmax(bbox_.max());
  Point corner[8];
  corner[0] = pmin;
  corner[1] = Point(pmin.x(), pmin.y(), pmax.z());
  corner[2] = Point(pmin.x(), pmax.y(), pmin.z());
  corner[3] = Point(pmin.x(), pmax.y(), pmax.z());
  corner[4] = Point(pmax.x(), pmin.y(), pmin.z());
  corner[5] = Point(pmax.x(), pmin.y(), pmax.z());
  corner[6] = Point(pmax.x(), pmax.y(), pmin.z());
  corner[7] = pmax;

  double tmin = Dot(corner[0] - view.origin(), view.direction());;
  double tmax = tmin;
  int maxi = 0;
  for (int i=1; i<8; i++)
  {
    double t = Dot(corner[i] - view.origin(), view.direction());
    tmin = Min(t, tmin);
    if (t > tmax) { maxi = i; tmax = t; }
  }

  // Make all of the slices consistent by offsetting them to a fixed
  // position in space (the origin).  This way they are consistent
  // between bricks and don't change with camera zoom.
  double tanchor = Dot(corner[maxi], view.direction());
  double tanchor0 = floor(tanchor/dt)*dt;
  double tanchordiff = tanchor - tanchor0;
  tmax -= tanchordiff;

  compute_polygons(view, tmin, tmax, dt, vertex, texcoord, size);
}


// compute polygon list of edge plane intersections
//
// This is never called externally and could be private.
//
// The representation returned is not efficient, but it appears a
// typical rendering only contains about 1k triangles.
void
TextureBrick::compute_polygons(const Ray& view,
			       double tmin, double tmax, double dt,
			       vector<float>& vertex, vector<float>& texcoord,
			       vector<int>& size)
{

  Vector vv[6], tt[6]; // temp storage for vertices and texcoords

  int k = 0, degree = 0;
  
  // find up and right vectors
  Vector vdir = view.direction();
  view_vector_ = vdir;
  Vector up;
  Vector right;
  switch(MinIndex(fabs(vdir.x()),
		  fabs(vdir.y()),
		  fabs(vdir.z())))
    {
    case 0:
      up.x(0.0); up.y(-vdir.z()); up.z(vdir.y());
      break;
    case 1:
      up.x(-vdir.z()); up.y(0.0); up.z(vdir.x());
      break;
    case 2:
      up.x(-vdir.y()); up.y(vdir.x()); up.z(0.0);
      break;
    }
  up.normalize();
  right = Cross(vdir, up);
  for (double t = tmax; t >= tmin; t -= dt)
  {
    const vector<Ray> &edges = edge_;
    const vector<Ray> &tedges = tex_edge_;
    
    // we compute polys back to front      
    // find intersections
    degree = 0;
    for (int j=0; j<12; j++)
    {
      double u;
      const bool intersects = edges[j].planeIntersectParameter
	(-view.direction(), view.parameter(t), u);
      if (intersects && u >= 0.0 && u <= 1.0)
      {
	vv[degree] = (Vector)(edges[j].parameter(u));
	tt[degree] = (Vector)(tedges[j].parameter(u));
	degree++;
      }
    }
    
    if (degree < 3) continue;
    
    if (degree > 3)
    {
	// compute centroids
      Vector vc(0.0, 0.0, 0.0), tc(0.0, 0.0, 0.0);
      for (int j=0; j<degree; j++)
      {
	vc += vv[j]; tc += tt[j];
      }
      vc /= (double)degree; tc /= (double)degree;
      
      // sort vertices
      int idx[6];
      double pa[6];
      for (int i=0; i<degree; i++)
      {
	double vx = Dot(vv[i] - vc, right);
	double vy = Dot(vv[i] - vc, up);
	
	// compute pseudo-angle
	pa[i] = vy / (fabs(vx) + fabs(vy));
	if (vx < 0.0) pa[i] = 2.0 - pa[i];
	else if (vy < 0.0) pa[i] = 4.0 + pa[i];
	// init idx
	idx[i] = i;
      }
      Sort(pa, idx, degree);
      
      // output polygon
      for (int j=0; j<degree; j++)
      {
	vertex.push_back(vv[idx[j]].x());
	vertex.push_back(vv[idx[j]].y());
	vertex.push_back(vv[idx[j]].z());
	texcoord.push_back(tt[idx[j]].x());
	texcoord.push_back(tt[idx[j]].y());
	texcoord.push_back(tt[idx[j]].z());
      }
    }
    else if (degree == 3)
    {
      // output a single triangle
      for (int j=0; j<degree; j++)
      {
	vertex.push_back(vv[j].x());
	vertex.push_back(vv[j].y());
	vertex.push_back(vv[j].z());
	texcoord.push_back(tt[j].x());
	texcoord.push_back(tt[j].y());
	texcoord.push_back(tt[j].z());
      }
    }
    k += degree;
    size.push_back(degree);
  }
}


bool
TextureBrick::mask_polygons(vector<int> & size, 
			    vector<float> &vertex,
			    vector<float> &texcoord,
			    vector<int> &mask,
			    vector<Plane *> &planes)
{

  mask = vector<int>(size.size(), 0);

  // Iterate through all the cutting planes
  for (unsigned p = 0; p < planes.size(); p++)
  {   
    const Plane &clipplane = *planes[p];
    Vector clipnormal = clipplane.normal();
    double dot = Dot(clipnormal, view_vector_);
      
    // Some cutting planes can share a bit pattern to create convex hulls
    const int clipmask = 1 << (p+0);

    // New vertices, tex coords, bitmasks, and poly sizes created when clipping
    vector<float>  newvertex(0);
    vector<float>  newtexcoord(0);
    vector<int>    newmask(0);
    vector<int>    newsize(0);    
    int idx = 0;
    for (unsigned int s = 0; s < size.size(); ++s) 
    {
      // Arrays to store points that are unclipped and clipped
      vector<int>    unclipped_index(0);
      vector<int>    isclipped_index(0);

      // An array to store paramaters of the intersections between the
      // poly and clipping plane occurs, for vertex and texture coordinate
      // reconstruciton in step two
      vector<pair<double,pair<int,int> > > intersections(0);

      // Step 1
      // Now iterate through every point in the poly, looking at all edges
      // Start at first point, meaning the first edge tested 
      // starts at the last point and connects to the first point
      for (int i = 0; i < size[s]; ++i) {

	// p1 starts at the last point in the poly
	int p1_index = idx+i;
	Point p1(vertex[p1_index*3],vertex[p1_index*3+1],vertex[p1_index*3+2]);
	double p1_dist = clipplane.eval_point(p1);
	// last point is clipped if distance to plane is negative
	bool p1_clipped = p1_dist < 0.0;

	if (p1_clipped)
	  isclipped_index.push_back(p1_index);
	else 
	  unclipped_index.push_back(p1_index);
	
	int p2_index = idx+(i+1)%size[s];
	Point p2(vertex[p2_index*3],vertex[p2_index*3+1],vertex[p2_index*3+2]);
	double p2_dist = clipplane.eval_point(p2);
	// determine if p2 is clipped or not
	bool p2_clipped = p2_dist < 0.0;

	// If p1 is clipped and p2 isnt, or vice versa, then this edge
	// crosses the clipping plane
	if (p1_clipped != p2_clipped) { 
	  // find the intersection parameter along the edge where it crossed
	  // the clipping plane
	  double t = 42.0;
	  //	  const bool intersects = 
	  clipplane.Intersect(p1, p2-p1, t);

	  // store the edge origin index and parameter for later
	  // reconstruction of the clipped and unclipped polygons
	  intersections.push_back(make_pair(t, make_pair(p1_index,p2_index)));

	  // Now push this intersection point onto both unclipped and clipped
	  // polygon arrays since the point is shared between the two.
	  // Use a negative index to indicate the point is a newly created
	  // intersection parameter and needs to be computed for texture
	  // coordinates as well
	  isclipped_index.push_back(-intersections.size());
	  unclipped_index.push_back(-intersections.size());
	}
      }

      // Step 2, reconstruct polygon into clipped and unclippd polygons
      // Make two passes to reconstruct the data arrays, unclipped then clipped
      for (int clipped = 0; clipped < 2; ++clipped) {
	// Point to unclipped on first pass, then unclipped on second pass
	vector<int> &index = clipped ? isclipped_index : unclipped_index;

	// If this pass produced no polygons, then do nothing
	if (index.size() < 3) continue;
	
	// Number of points in this new polygon
	newsize.push_back(index.size());

	// mask[s] is the clipped state of the current polygon
	// Only turn on clipping plane mask bits if poly was not clipped
        if (1) {
          newmask.push_back(mask[s] | (clipped ? 0 : clipmask));
        } else {
          if (mask[s] && ((dot > 0) == clipped)) {
            newmask.push_back(1);
          }
          else {
            newmask.push_back(0);
          }
        }
        //	newmask.push_back(mask[s] | (clipped ? 0 : clipmask));
	  
	// Iterate through the indicies and create the new polygon
	for (unsigned int i = 0; i < index.size(); ++i) {
	  int index1 = index[i];

	  // If positive, then it was an original index
	  if (index1 >= 0) {
	    assert(index1 >= idx && index1 <= idx+size[s]);
	    // Just copy the original x,y,z vertex and texture coordinates
	    for (int j = index1*3; j < index1*3 + 3; ++j) {
	      newvertex.push_back(vertex[j]);
	      newtexcoord.push_back(texcoord[j]);
	    }
	  } else {
	    // If index is negative, then it was an intersection point and 
	    // a new vertex and texture coordinate needs to be created

	    // Get the parameter along the edge where the intersection was 
	    double t = intersections[-index1-1].first;
	    // Get the index of the edge origin
	    int index2 = intersections[-index1-1].second.second;
	    index1 = intersections[-index1-1].second.first;
	    // Get the index of the edge terminaiton

	    assert(index1 >= idx && index1 <= idx+size[s]);
	    assert(index2 >= idx && index2 <= idx+size[s]);
	    index1 *= 3; index2 *=3;
	    for (int j = 0; j < 3; ++j) {
	      // calculate new vertex coordinate along edge using t parameter
	      newvertex.push_back(vertex[index1+j]+
				  t*(vertex[index2+j]-vertex[index1+j]));
	      // calculate new texture coordinate along edge using t parameter
	      newtexcoord.push_back(texcoord[index1+j]+
				    t*(texcoord[index2+j]-texcoord[index1+j]));
	    }
	  }
	}
      }
      // continue to next polygon to test it for intersections
      idx += size[s];
    }
    // After clipping all polygons against a clipping plane,
    // set the data structures to the new state for next iteration and 
    // clippping plane
    vertex = newvertex;
    texcoord = newtexcoord;
    size = newsize;
    mask = newmask;
  }

  // Done!
  return true;
}

int
TextureBrick::sx()
{
  if (data_[0]->dim == 3)
  {
    return data_[0]->axis[0].size;
  }
  else
  {
    return data_[0]->axis[1].size;
  }
}


int
TextureBrick::sy()
{
  if (data_[0]->dim == 3)
  {
    return data_[0]->axis[1].size;
  }
  else
  {
    return data_[0]->axis[2].size;
  }
}


GLenum
TextureBrick::tex_type_aux(const Nrrd* n)
{
  // GL_BITMAP!?
  if (n->type == nrrdTypeChar)   return GL_BYTE;
  if (n->type == nrrdTypeUChar)  return GL_UNSIGNED_BYTE;
  if (n->type == nrrdTypeShort)  return GL_SHORT;
  if (n->type == nrrdTypeUShort) return GL_UNSIGNED_SHORT;
  if (n->type == nrrdTypeInt)    return GL_INT;
  if (n->type == nrrdTypeUInt)   return GL_UNSIGNED_INT;
  if (n->type == nrrdTypeFloat)  return GL_FLOAT;
  return GL_NONE;
}


size_t
TextureBrick::tex_type_size(GLenum t)
{
  if (t == GL_BYTE)           { return sizeof(GLbyte); }
  if (t == GL_UNSIGNED_BYTE)  { return sizeof(GLubyte); }
  if (t == GL_SHORT)          { return sizeof(GLshort); }
  if (t == GL_UNSIGNED_SHORT) { return sizeof(GLushort); }
  if (t == GL_INT)            { return sizeof(GLint); }
  if (t == GL_UNSIGNED_INT)   { return sizeof(GLuint); }
  if (t == GL_FLOAT)          { return sizeof(GLfloat); }
  return 0;
}


GLenum
TextureBrick::tex_type(int c)
{
  assert(c < nc_);
  return tex_type_aux(data_[c]);
}

void *
TextureBrick::tex_data(int c)
{
  assert(c >= 0 && c < TEXTURE_MAX_COMPONENTS && c < nc());  
  unsigned char *ptr = (unsigned char *)(data_[c]->data);
  const size_t offset = (oz() * sx() * sy() + oy() * sx() + ox()) * nb(c);
  return ptr + offset * tex_type_size(tex_type(c));
}

} // end namespace SLIVR
