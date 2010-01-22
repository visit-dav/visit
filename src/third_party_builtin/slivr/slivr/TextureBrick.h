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
//    File   : TextureBrick.h
//    Author : Milan Ikits
//    Date   : Wed Jul 14 15:55:55 2004

#ifndef SLIVR_TextureBrick_h
#define SLIVR_TextureBrick_h

#include <slivr/Ray.h>
#include <slivr/BBox.h>
#include <slivr/Plane.h>
#include <slivr/GLinfo.h>

#include <vector>
#include <teem/nrrd.h>

#include <slivr/slivr_share.h>

namespace SLIVR {

using std::vector;

// We use no more than 2 texture units.
// GL_MAX_TEXTURE_UNITS is the actual maximum.
#define TEXTURE_MAX_COMPONENTS 2

class SLIVRSHARE TextureBrick
{
public:
  // Creator of the brick owns the nrrd memory.
  TextureBrick(Nrrd* n0, Nrrd* n1,
	       int nx, int ny, int nz, int nc, int* nb, int ox, int oy, int oz,
	       int mx, int my, int mz, const BBox& bbox, const BBox& tbox);
  virtual ~TextureBrick();

  inline const BBox &bbox() const { return bbox_; }
  //inline BBox tbox() const { return tbox_; }

  inline int nx() { return nx_; }
  inline int ny() { return ny_; }
  inline int nz() { return nz_; }
  inline int nc() { return nc_; }
  inline int nb(int c)
  {
    assert(c >= 0 && c < TEXTURE_MAX_COMPONENTS);
    return nb_[c];
  }

  inline int mx() { return mx_; }
  inline int my() { return my_; }
  inline int mz() { return mz_; }
  
  inline int ox() { return ox_; }
  inline int oy() { return oy_; }
  inline int oz() { return oz_; }

  virtual int sx();
  virtual int sy();

  // Creator of the brick owns the nrrd memory.
  void set_nrrds(Nrrd* n0, Nrrd* n1) 
  { 
    data_[0] = n0; 
    data_[1] = n1; 
  }

  virtual GLenum tex_type(int c);
  virtual void* tex_data(int c);

  inline bool dirty() const { return dirty_; }
  inline void set_dirty(bool b) { dirty_ = b; }
  
  void compute_polygons(const Ray& view, double tmin, double tmax, double dt,
                        vector<float>& vertex, vector<float>& texcoord,
                        vector<int>& size);
  void compute_polygons(const Ray& view, double dt,
                        vector<float>& vertex, vector<float>& texcoord,
                        vector<int>& size);
  void compute_polygon(const Ray& view, double t,
                       vector<float>& vertex, vector<float>& texcoord,
                       vector<int>& size);

  bool mask_polygons(vector<int> &size,
		     vector<float> &vertex,
		     vector<float> &texcoord,
		     vector<int> &masks,
		     vector<Plane *> &planes);
		     
  enum tb_td_info_e {
    FULL_TD_E,
    TB_NAME_ONLY_E,
    DATA_TD_E
  };


private:
  void compute_edge_rays(BBox &bbox, vector<Ray> &edges) const;
  size_t tex_type_size(GLenum t);
  GLenum tex_type_aux(const Nrrd* n);

  Nrrd* data_[TEXTURE_MAX_COMPONENTS];
  //! axis sizes (pow2)
  int nx_, ny_, nz_; 
  //! number of components (< TEXTURE_MAX_COMPONENTS)
  int nc_; 
  //! bytes per texel for each component.
  int nb_[TEXTURE_MAX_COMPONENTS]; 
  //! offset into volume texture
  int ox_, oy_, oz_; 
  //! data axis sizes (not necessarily pow2)
  int mx_, my_, mz_; 
  //! bounding box and texcoord box
  BBox bbox_, tbox_; 
  //! bbox edges
  vector<Ray> edge_; 
  //! tbox edges
  vector<Ray> tex_edge_; 
  bool dirty_;
  Vector view_vector_;
};

} // namespace SLIVR

#endif // Volume_TextureBrick_h
