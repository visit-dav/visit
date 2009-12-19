//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2008 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
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
//    File   : KDTree.h
//    Author : Martin Cole
//    Date   : Mon Jun 18 14:55:47 2007


#if !defined(KDTree_h)
#define KDTree_h

#include <slivr/BBox.h>
#include <slivr/Plane.h>
#include <vector>

namespace SLIVR {
using std::vector;

class KDTree {
public:
  typedef size_t idx_type_t;

  struct Node {
    
  };
  struct KDNode {
    KDNode() :
      left_c_(0),
      right_c_(0),
      split_(0)
    {}

    ~KDNode()
    {
      if (left_c_) { delete left_c_; }
      if (right_c_) { delete right_c_; }
      if (split_) { delete split_; }
    }

    //! space the node covers.
    BBox                    voxel_;
    Plane                   *split_;
    
    //! left and right child nodes.
    KDNode                  *left_c_;
    KDNode                  *right_c_;
    
  };

  
  struct KDLeafNode: public KDNode {
    KDLeafNode() : 
      KDNode(),
      elems_(0)
    {}
    ~KDLeafNode()
    {}

    //! Each node has the intersecting element indices, 
    //! stored in the global array 
    vector<idx_type_t>          elems_;
  };


  KDTree(const vector<BBox> &b, const BBox &V);
  ~KDTree();  

  void view_sort(const Vector &view, vector<idx_type_t> &boxes) const;

private:
  KDNode* build(const vector<idx_type_t> &b, const BBox &V);
  bool terminate(const vector<idx_type_t> &boxes);
  Plane find_plane(const vector<idx_type_t> &boxes, const BBox &vol);
  void split_volume(const BBox &V, const Plane &p, BBox &V_l, BBox &V_r);
  void sort(const vector<idx_type_t> &boxes, 
	    const BBox &V_l, const BBox &V_r, 
	    vector<idx_type_t> &left, vector<idx_type_t> &right);

  
private:
  idx_type_t                  next_face_; //counter for the tree.
  const vector<BBox>         &all_;
  KDNode                     *root_;
};


} // namespace SLIVR

#endif //KDTree_h
