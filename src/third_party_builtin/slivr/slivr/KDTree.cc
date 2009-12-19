/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.

   
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


#include <slivr/KDTree.h>
#include <iostream>

namespace SLIVR {
using namespace std;

KDTree::KDTree(const vector<BBox> &b, const BBox &V) :
  next_face_(0),
  all_(b),
  root_(0)
{
  vector<idx_type_t> all(all_.size());
  for (int i = 0; i < all_.size(); ++i) {
    all[i] = i;
  }
  root_ = build(all, V);
}

KDTree::~KDTree() 
{
}

void
print_bbox(string s, const BBox &b) {
  cerr << s << b.min() << " to " << b.max() << endl;
}

KDTree::KDNode* 
KDTree::build(const vector<idx_type_t> &boxes, const BBox &V) 
{
  if (terminate(boxes)) {
    KDLeafNode *n = new KDLeafNode();
    n->voxel_ = V;
    n->elems_ = boxes;
    cerr << "LEAF" << endl;
    return n;
  }
  Plane p = find_plane(boxes, V);
  BBox V_l;
  BBox V_r; 
  V_l.set_valid(false);
  V_r.set_valid(false);
  split_volume(V, p, V_l, V_r);
  print_bbox("left", V_l);
  print_bbox("right", V_r);

  vector<idx_type_t> boxes_l;
  vector<idx_type_t> boxes_r;
  sort(boxes, V_l, V_r, boxes_l, boxes_r);
  cerr << "post split/sort: L" << boxes_l.size() 
       << " R " << boxes_r.size() << " total: " << boxes.size() << endl;
  KDNode *n = new KDNode();
  n->voxel_ = V;
  n->split_ = new Plane(p);
  n->left_c_  = build(boxes_l, V_l);
  n->right_c_ = build(boxes_r, V_r);
  cerr << "NODE" << endl;
  return n;
}

bool
KDTree::terminate(const vector<idx_type_t> &boxes)
{
  return boxes.size() < 2;
}


Plane
KDTree::find_plane(const vector<idx_type_t> &boxes, const BBox &vol)
{
  //median of 3 random
  size_t n = boxes.size();
  
//   for (int i = 0; i < 10; i++) {
//     cerr << "rand:" << drand48() << ", " << drand48() << ", " << drand48() << endl;
//   }
  
  unsigned int idx0 = 0;
  unsigned int idx1 = (n > 1) ? 1 : 0;
  unsigned int idx2 = (n > 2) ? 2 : (n > 1) ? 1 : 0;

  if (boxes.size() > 3) {
    idx0 = (unsigned int)(drand48() * n);
    idx1 = (unsigned int)(drand48() * n);
    idx2 = (unsigned int)(drand48() * n);
  }
  cerr << "sz: " << n << " chose indices:" 
       << idx0 << ", " << idx1 << ", " << idx2 << endl;

  // Pick one of the 6 planes that make up the bounding box faces. 
  // For now just round robin the faces.
  Plane split;
  switch(next_face_ % 6) {
  case 0:
    {
      //min x,y
      Vector d = vol.diagonal();
      double vmid = ((d * .5) + vol.min()).z();
      // which min z is closest to the z value of the vol mid.
      double b0 = all_[boxes[idx0]].min().z();
      double b1 = all_[boxes[idx1]].min().z();
      double b2 = all_[boxes[idx2]].min().z();

      double min = fabs(vmid - b0);
      unsigned int closest = idx0;
      double b1dist = fabs(vmid - b1);
      if (b1dist < min) {
	min = b1dist;
	closest = idx1;
      }

      double b2dist = fabs(vmid - b2);
      if (b2dist < min) {
	closest = idx2;
      }
      cerr << "closest: " << closest << endl;
      // create the plane.
      split = Plane(all_[boxes[closest]].min(), Vector(0.0, 0.0, 1.0));
    }
  break;
  case 1:
    {
      //min x,z
      Vector d = vol.diagonal();
      double vmid = ((d * .5) + vol.min()).y();
      // which min y is closest to the y value of the vol mid.
      double b0 = all_[boxes[idx0]].min().y();
      double b1 = all_[boxes[idx1]].min().y();
      double b2 = all_[boxes[idx2]].min().y();

      double min = fabs(vmid - b0);
      unsigned int closest = idx0;
      double b1dist = fabs(vmid - b1);
      if (b1dist < min) {
	min = b1dist;
	closest = idx1;
      }

      double b2dist = fabs(vmid - b2);
      if (b2dist < min) {
	closest = idx2;
      }
      // create the plane.
      split = Plane(all_[boxes[closest]].min(), Vector(0.0, 1.0, 0.0));
    }
  break;
  case 2:
    {
      //min y,z
      Vector d = vol.diagonal();
      double vmid = ((d * .5) + vol.min()).x();
      // which min x is closest to the x value of the vol mid.
      double b0 = all_[boxes[idx0]].min().x();
      double b1 = all_[boxes[idx1]].min().x();
      double b2 = all_[boxes[idx2]].min().x();

      double min = fabs(vmid - b0);
      unsigned int closest = idx0;
      double b1dist = fabs(vmid - b1);
      if (b1dist < min) {
	min = b1dist;
	closest = idx1;
      }

      double b2dist = fabs(vmid - b2);
      if (b2dist < min) {
	closest = idx2;
      }
      // create the plane.
      split = Plane(all_[boxes[closest]].min(), Vector(1.0, 0.0, 0.0));
    }
  break;
  case 3:
    {
      //max x,y
      Vector d = vol.diagonal();
      double vmid = ((d * .5) + vol.max()).z();
      // which max z is closest to the z value of the vol mid.
      double b0 = all_[boxes[idx0]].max().z();
      double b1 = all_[boxes[idx1]].max().z();
      double b2 = all_[boxes[idx2]].max().z();

      double min = fabs(vmid - b0);
      unsigned int closest = idx0;
      double b1dist = fabs(vmid - b1);
      if (b1dist < min) {
	min = b1dist;
	closest = idx1;
      }

      double b2dist = fabs(vmid - b2);
      if (b2dist < min) {
	closest = idx2;
      }
      // create the plane.
      split = Plane(all_[boxes[closest]].max(), Vector(0.0, 0.0, -1.0));
    }
  break;
  case 4:
    {
      //max x,z
      Vector d = vol.diagonal();
      double vmid = ((d * .5) + vol.max()).y();
      // which max y is closest to the y value of the vol mid.
      double b0 = all_[boxes[idx0]].max().y();
      double b1 = all_[boxes[idx1]].max().y();
      double b2 = all_[boxes[idx2]].max().y();

      double min = fabs(vmid - b0);
      unsigned int closest = idx0;
      double b1dist = fabs(vmid - b1);
      if (b1dist < min) {
	min = b1dist;
	closest = idx1;
      }

      double b2dist = fabs(vmid - b2);
      if (b2dist < min) {
	closest = idx2;
      }
      // create the plane.
      split = Plane(all_[boxes[closest]].max(), Vector(0.0, -1.0, 0.0));
    }
  break;
  case 5:
    {
      //max y,z
      Vector d = vol.diagonal();
      double vmid = ((d * .5) + vol.max()).x();
      // which max x is closest to the x value of the vol mid.
      double b0 = all_[boxes[idx0]].max().x();
      double b1 = all_[boxes[idx1]].max().x();
      double b2 = all_[boxes[idx2]].max().x();

      double min = fabs(vmid - b0);
      unsigned int closest = idx0;
      double b1dist = fabs(vmid - b1);
      if (b1dist < min) {
	min = b1dist;
	closest = idx1;
      }

      double b2dist = fabs(vmid - b2);
      if (b2dist < min) {
	closest = idx2;
      }
      // create the plane.
      split = Plane(all_[boxes[closest]].max(), Vector(-1.0, 0.0, 0.0));
    }
  break;

  };
  ++next_face_;
  return split;
}

void 
KDTree::split_volume(const BBox &V, const Plane &p, BBox &V_l, BBox &V_r)
{
  Point hit;
  Point min_l;
  Point max_l;
  Point min_r;
  Point max_r;

  if (p.Intersect(V.min(), Vector(1.0, 0.0, 0.0), hit)) {
    // y,z plane.
    cerr << "y,z plane. " << hit << endl;
    min_l = V.min();
    max_l = V.max();
    max_l.x(hit.x());

    min_r = hit;
    max_r = V.max();
  } else if (p.Intersect(V.min(), Vector(0.0, 1.0, 0.0), hit)) {
    // x,z plane.
    cerr << "x,z plane." << endl;
    min_l = V.min();
    max_l = V.max();
    max_l.y(hit.y());

    min_r = hit;
    max_r = V.max();
  } else {
    p.Intersect(V.min(), Vector(0.0, 0.0, 1.0), hit);
    // x,y plane.
    cerr << "x,y plane." << endl;
    min_l = V.min();
    max_l = V.max();
    max_l.z(hit.z());

    min_r = hit;
    max_r = V.max();
  }
  V_l.extend(min_l);
  V_l.extend(max_l);
  V_r.extend(min_r);
  V_r.extend(max_r);
}


void
KDTree::sort(const vector<idx_type_t> &boxes, 
	     const BBox &V_l, const BBox &V_r, 
	     vector<idx_type_t> &left, vector<idx_type_t> &right)
{
  vector<idx_type_t>::const_iterator iter = boxes.begin();
  while (iter != boxes.end()) {
    idx_type_t idx = *iter++;
    const BBox &b = all_[idx];
    if (b.overlaps_inside(V_l)) {
      left.push_back(idx);
    } 
    if (b.overlaps_inside(V_r)) {
      right.push_back(idx);
    }
  }
}

// void
// KDTree::view_sort_recurse(const Vector &view, vector<idx_type_t> &boxes,
// 			  KDNode *cur) 
// {
//     KDLeafNode *l = dynamic_cast<KDLeafNode*>(cur);
//     if (l) {
//       cerr << "leaf: " << l << endl;
//       return;
//     } else {
      
//     }
// }

void
KDTree::view_sort(const Vector &view, vector<idx_type_t> &boxes) const
{
//   KDNode *cur = root_;
//   while (!done) {
//     KDLeafNode *l = dynamic_cast<KDLeafNode*>(cur);
//     if (l) {
//       cerr << "leaf: " << l << endl;
//     } else {
      
//     }

    


//   }
}


} // namespace SLIVR
