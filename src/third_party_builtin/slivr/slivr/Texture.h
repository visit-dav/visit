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
//    File   : Texture.h
//    Author : Milan Ikits
//    Date   : Thu Jul 15 01:00:36 2004

#ifndef SLIVR_Texture_h
#define SLIVR_Texture_h


#include <vector>
#include <slivr/Transform.h>
#include <slivr/TextureBrick.h>
#include <slivr/Utils.h>
#include <slivr/slivr_share.h>

namespace SLIVR {

using namespace std;

class Transform;

class SLIVRSHARE Texture 
{
public:
  Texture();
  virtual ~Texture();

  void build(Nrrd* val, Nrrd* grad, double vmn, double vmx,
	     double gmn, double gmx, int card_mem);

 
  inline int nx() const { return nx_; }
  inline int ny() const { return ny_; }
  inline int nz() const { return nz_; }

  inline int nc() const { return nc_; }
  inline int nb(int i) const
  {
    assert(i >= 0 && i < TEXTURE_MAX_COMPONENTS);
    return nb_[i];
  }

  inline 
  void set_size(int nx, int ny, int nz, int nc, int* nb) 
  {
    nx_ = nx; ny_ = ny; nz_ = nz; nc_ = nc;
    for(int c = 0; c < nc_; c++) {
      nb_[c] = nb[c];
    }
  }
  

  //! Interface that does not expose SLIVR::BBox.
  inline 
  void get_bounds(double &xmin, double &ymin, double &zmin,
		  double &xmax, double &ymax, double &zmax) const 
  {
    BBox b;
    get_bounds(b);
    xmin = b.min().x();
    ymin = b.min().y();
    zmin = b.min().z();

    xmax = b.max().x();
    ymax = b.max().y();
    zmax = b.max().z();
  }

  inline 
  void get_bounds(BBox &b) const 
  {
    b.extend(transform_.project(bbox_.min()));
    b.extend(transform_.project(bbox_.max()));
  }

  inline const BBox &bbox() const { return bbox_; }
  inline void set_bbox(const BBox& bbox) { bbox_ = bbox; }
  inline const Transform &transform() const { return transform_; }
  inline void set_transform(Transform tform) { transform_ = tform; }

  // TODO:  This is being called with is_orthographic in the wrong slot. fix.
  void get_sorted_bricks(vector<TextureBrick*>& bricks,
			 const Ray& view, 
			 int idx = 0, 
			 bool is_orthographic = false);

  inline vector<TextureBrick*>& bricks(int i = 0) { return bricks_[i]; }

  inline int nlevels(){ return bricks_.size(); }
  void clear();
  
  inline double vmin() const { return vmin_; }
  inline double vmax() const { return vmax_; }
  inline double gmin() const { return gmin_; }
  inline double gmax() const { return gmax_; }
  inline void set_minmax(double vmin, double vmax, double gmin, double gmax) {
    vmin_ = vmin; vmax_ = vmax; gmin_ = gmin; gmax_ = gmax;
  }
  
  inline int card_mem() const { return card_mem_; }
  inline void set_card_mem(int mem) { card_mem_ = mem; }
  
protected:
  void build_bricks(vector<TextureBrick*> &bricks, int nx, int ny, int nz,
		    int nc, int* nb, int card_mem);

  //! data carved up to texture memory sized chunks.
  vector<vector<TextureBrick*> >           bricks_;
  //! data size
  int                                      nx_;
  int                                      ny_;
  int                                      nz_; 
  //! number of components currently used.
  int                                      nc_; 
  //! bytes per texel for each component.
  int                                      nb_[TEXTURE_MAX_COMPONENTS];
  //! data tform
  Transform                                transform_; 
  double                                   vmin_;
  double                                   vmax_;
  double                                   gmin_;
  double                                   gmax_;
  //! data bbox
  BBox                                     bbox_; 
  int                                      card_mem_;
};

} // namespace SLIVR

#endif // Volume_Texture_h
