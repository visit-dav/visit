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
//    File   : Texture.cc
//    Author : Milan Ikits
//    Date   : Thu Jul 15 10:58:23 2004

#include <slivr/gldefs.h>
#include <slivr/ShaderProgramARB.h>
#include <slivr/Texture.h>
#include <slivr/Utils.h>

namespace SLIVR {

Texture::Texture() :
  nx_(0), 
  ny_(0), 
  nz_(0),
  nc_(0), 
  vmin_(0.0), 
  vmax_(0.0), 
  gmin_(0.0), 
  gmax_(0.0)
{
  for (int i = 0; i < TEXTURE_MAX_COMPONENTS; i++) { nb_[i] = 0; }
  bricks_.resize(1);
  bricks_[0].resize(0);
}


Texture::~Texture()
{
}


void
Texture::get_sorted_bricks(vector<TextureBrick*> &bricks, 
			   const Ray& view,
			   int idx, bool is_orthographic)
{
  bricks.clear();
  vector<TextureBrick*> &brick_ = bricks_[idx];
  vector<double> dist;
  for (unsigned int i = 0; i < brick_.size(); i++)
  {
    Point minp(brick_[i]->bbox().min());
    Point maxp(brick_[i]->bbox().max());
    Vector diag(brick_[i]->bbox().diagonal());
    minp += diag / 1000.;
    maxp -= diag / 1000.;
    Point corner[8];
    corner[0] = minp;
    corner[1] = Point(minp.x(), minp.y(), maxp.z());
    corner[2] = Point(minp.x(), maxp.y(), minp.z());
    corner[3] = Point(minp.x(), maxp.y(), maxp.z());
    corner[4] = Point(maxp.x(), minp.y(), minp.z());
    corner[5] = Point(maxp.x(), minp.y(), maxp.z());
    corner[6] = Point(maxp.x(), maxp.y(), minp.z());
    corner[7] = maxp;
    double d = 0.0;
    for (unsigned int c = 0; c < 8; c++) {
      double dd;
      if (is_orthographic) {
	// orthographic: sort bricks based on distance to the view plane
	dd = Dot(corner[c], view.direction());
      } else {
	// perspective: sort bricks based on distance to the eye point
	dd = (corner[c] - view.origin()).length();
      }
      if (c == 0 || dd < d) d = dd;
    }
    bricks.push_back(brick_[i]);
    dist.push_back(-d);
  }
  Sort(dist, bricks);
}

// clear doesn't really clear everything probably bad programming-kz
void
Texture::clear()
{
  bricks_.clear();

  nx_ = 0;
  ny_ = 0;
  nz_ = 0;
  nc_ = 0;
  vmin_ = 0;
  vmax_ = 0;
  gmin_ = 0;
  gmax_ = 0;
  for (int i = 0; i < TEXTURE_MAX_COMPONENTS; i++) { nb_[i] = 0; }
  bricks_.resize(0);
}

void 
Texture::build(Nrrd* nv_nrrd, Nrrd* gm_nrrd,  
	       double vmn, double vmx,
	       double gmn, double gmx, int crd_mem)
{
  size_t axis_size[4];
  nrrdAxisInfoGet_nva(nv_nrrd, nrrdAxisInfoSize, axis_size);
  double axis_min[4];
  nrrdAxisInfoGet_nva(nv_nrrd, nrrdAxisInfoMin, axis_min);
  double axis_max[4];
  nrrdAxisInfoGet_nva(nv_nrrd, nrrdAxisInfoMax, axis_max);

  int numc = gm_nrrd ? 2 : 1;
  int numb[2];
  numb[0] = nv_nrrd->dim == 4 ? axis_size[0] : 1;
  numb[1] = gm_nrrd ? 1 : 0;

  const BBox bb(Point(0,0,0), Point(1,1,1)); 

  Transform tform;
  tform.load_identity();
  
  size_t dim = nv_nrrd->dim;
  std::vector<double> min(dim), max(dim);
  std::vector<int> size(dim);

  int offset = 0;
  if (dim > 3) offset = 1; 

  for (size_t p = 0; p < dim; p++) 
  {
    size[p] = nv_nrrd->axis[p + offset].size;
    
    if (airExists(nv_nrrd->axis[p + offset].min)) 
    {
      min[p] = nv_nrrd->axis[p + offset].min;
    }
    else
    {
      min[p] = 0.0;
    }
    
    if (airExists(nv_nrrd->axis[p + offset].max)) 
    {
      max[p] = nv_nrrd->axis[p + offset].max;
    }
    else
    {
      if (airExists(nv_nrrd->axis[p + offset].spacing)) 
      {
        if (nv_nrrd->axis[p + offset].center == 1)
        {
          max[p] = nv_nrrd->axis[p + offset].spacing * (size[p] - 1);
        }
        else
        {
          max[p] = nv_nrrd->axis[p + offset].spacing * size[p];        
        }
      }
      else
      {
        if (nv_nrrd->axis[p + offset].center == 1)
        {
          max[p] = static_cast<double>(size[p]);
        }
        else
        {
          max[p] = static_cast<double>(size[p] - 1);        
        }
      }
    }
  }  
    
  Vector origin;
  std::vector<Vector> space_dir(dim);

  if (nv_nrrd->spaceDim > 0)
  {
    const size_t sd = nv_nrrd->spaceDim;

    if (nv_nrrd->axis[offset].center == 1)
    {
      for (size_t p = 0; p < dim; p++)   
      {
        min[p] = 0.0;
        max[p] = static_cast<double>(size[p] - 1);
      } 
    }
    else
    {
      for (size_t p = 0; p < dim; p++)   
      {
        min[p] = 0.0;
        max[p] = static_cast<double>(size[p]);
      }     
    }
    
    for (size_t q = 0 ; q < sd && q < dim; q++)
    {
      if (airExists(nv_nrrd->spaceOrigin[q])) {
        origin[q] = nv_nrrd->spaceOrigin[q]; 
      } else {
        origin[q] = 0.0;
      }
      for (size_t p = 0; p < dim && p < dim; p++)
        if (airExists(nv_nrrd->axis[p + offset].spaceDirection[q])) {
          space_dir[p][q] = nv_nrrd->axis[p + offset].spaceDirection[q]; 
        } else {
          space_dir[p][q] = 0.0;
	}
    }
  
    if (dim == 1) 
    {
      space_dir[0].find_orthogonal(space_dir[1], space_dir[2]);
    }
    else if (dim == 2)
    {
      space_dir[2] = Cross(space_dir[0], space_dir[1]);
    }

    tform.load_basis(Point(origin), space_dir[0] * size[0],
		     space_dir[1] * size[1], space_dir[2] * size[2]);
  }
  else
  {
    const Point nmin(min[0], min[1], min[2]);
    const Point nmax(max[0], max[1], max[2]);
    tform.pre_scale(nmax - nmin);
    tform.pre_translate(nmin.asVector());  
  }
  
  // The Teem documentation is not specific on how deal deal with RHS and LHS
  // Hence we interpret the information as following:
  // (1) if a patient specific orientation is given, we check the parity of
  //     the space directions and the parity of the objective and convert
  //     if needed, i.e. either coord parity or space parity is LHS, then 
  //     we mirror.
  // (2) if ScannerXYZ is given, nothing is assumed about coord parity and 
  //     space parity, as it is not clear what has been defined.
  // (3) in case SpaceLeft3DHanded is given, we assume space parity is LHS and
  //     coord parity is not of importance. 
  
  bool reverseparity = false;
  bool coordparity = true;
  
  coordparity = (Dot(space_dir[2], Cross(space_dir[0], space_dir[1])) >= 0);
  
  if ((nv_nrrd->space == nrrdSpaceRightAnteriorSuperior)||
      (nv_nrrd->space == nrrdSpaceLeftPosteriorSuperior)||
      (nv_nrrd->space == nrrdSpaceRightAnteriorSuperiorTime) ||
      (nv_nrrd->space == nrrdSpaceLeftPosteriorSuperiorTime))
  {
    if (coordparity == false) reverseparity = true; 
  }

  if ((nv_nrrd->space == nrrdSpaceLeftAnteriorSuperior)||
      (nv_nrrd->space == nrrdSpaceLeftAnteriorSuperiorTime))
  {
    if (coordparity == true) reverseparity = true;
  }
  
  if ((nv_nrrd->space == nrrdSpace3DLeftHanded) ||
      (nv_nrrd->space == nrrdSpace3DLeftHandedTime))
  {
    reverseparity = true;
  }
 
  if (reverseparity)
  {
    tform.load_basis(-Point(origin), -space_dir[0], 
		     -space_dir[1], -space_dir[2]);
  }


  // The following predates direction vectors in nrrds, where the transform 
  // from a field was communicated through a string...
  char *trans_ch = 0;
  trans_ch = nrrdKeyValueGet(nv_nrrd, "sci-transform");
  // See if it's stored in the nrrd first.
  if (trans_ch)
  {
    string trans_str(trans_ch);
    double t[16];
    int old_index=0, new_index=0;
    for(int i=0; i<16; i++)
    {
      new_index = trans_str.find(" ", old_index);
      string temp = trans_str.substr(old_index, new_index-old_index);
      old_index = new_index+1;
      // convert the string to a double.
      sscanf(temp.c_str(), "%lf", &t[i]);
    }
    tform.set(t);
    free(trans_ch);
  } 
  
  
  
  vector<TextureBrick*> &brks = bricks();
  if (size[0] != nx() || size[1] != ny() || size[2] != nz() ||
      numc != nc() || numb[0] != nb(0) ||
      crd_mem != card_mem() ||
      bb.min() != bbox().min() ||
      bb.max() != bbox().max() ||
      vmn != vmin() ||
      vmx != vmax() ||
      gmn != gmin() ||
      gmx != gmax() )
  {
    build_bricks(brks, size[0], size[1], size[2], numc, numb, crd_mem);  
    set_size(size[0], size[1], size[2], numc, numb);
    set_card_mem(crd_mem);
  }
  set_bbox(bb);
  set_minmax(vmn, vmx, gmn, gmx);
  set_transform(tform);

  for (unsigned int i = 0; i < brks.size(); i++)
  {
    TextureBrick *tb = brks[i];
    tb->set_nrrds(nv_nrrd, gm_nrrd);
    tb->set_dirty(true);
  }
}


void 
Texture::build_bricks(vector<TextureBrick*> &bricks, 
		      int sz_x, int sz_y, int sz_z,
		      int numc, int* numb, int card_mem)
{
  const bool force_pow2 = !ShaderProgramARB::texture_non_power_of_two();
  const int brick_mem = card_mem * 1024 * 1024 / 2;

  const unsigned int max_texture_size = 
    ((numb[0] == 1)?
     ShaderProgramARB::max_texture_size_1() :
     ShaderProgramARB::max_texture_size_4());

  // Initial brick size
  int bsize[3];
  bsize[0] = Min(Pow2(sz_x), max_texture_size);
  bsize[1] = Min(Pow2(sz_y), max_texture_size);
  bsize[2] = Min(Pow2(sz_z), max_texture_size);

  if (force_pow2)
  {
    if (Pow2(sz_x) > (unsigned)sz_x) 
      bsize[0] = Min(Pow2(sz_x)/2, max_texture_size);
    if (Pow2(sz_y) > (unsigned)sz_y) 
      bsize[1] = Min(Pow2(sz_y)/2, max_texture_size);
    if (Pow2(sz_z) > (unsigned)sz_z) 
      bsize[2] = Min(Pow2(sz_z)/2, max_texture_size);
  }
  // Determine brick size here.

  // Slice largest axis, weighted by fastest/slowest memory access
  // axis so that our cuts leave us with contiguous blocks of data.
  // Currently set at 4x2x1 blocks.
  while (bsize[0] * bsize[1] * bsize[2] * numb[0] > brick_mem)
  {
    if (bsize[1] / bsize[2] >= 4 || bsize[2] < 4)
    {
      if (bsize[0] / bsize[1] >= 2 || bsize[1] < 4)
      {
        bsize[0] /= 2;
      }
      else
      {
        bsize[1] /= 2;
      }
    }
    else
    {
      bsize[2] /= 2;
    }
  }

  bricks.clear();

  for (int k = 0; k < sz_z; k += bsize[2])
  {
    if (k) k--;
    for (int j = 0; j < sz_y; j += bsize[1])
    {
      if (j) j--;
      for (int i = 0; i < sz_x; i += bsize[0])
      {
        if (i) i--;
        const int mx = Min(bsize[0], sz_x - i);
        const int my = Min(bsize[1], sz_y - j);
        const int mz = Min(bsize[2], sz_z - k);
        
        int mx2 = mx;
        int my2 = my;
        int mz2 = mz;
        if (force_pow2)
        {
          mx2 = Pow2(mx);
          my2 = Pow2(my);
          mz2 = Pow2(mz);
        }

        // Compute Texture Box.
        const double tx0 = i?((mx2 - mx + 0.5) / mx2): 0.0;
        const double ty0 = j?((my2 - my + 0.5) / my2): 0.0;
        const double tz0 = k?((mz2 - mz + 0.5) / mz2): 0.0;
        
        double tx1 = 1.0 - 0.5 / mx2;
        if (mx < bsize[0]) tx1 = 1.0;
        if (sz_x - i == bsize[0]) tx1 = 1.0;

        double ty1 = 1.0 - 0.5 / my2;
        if (my < bsize[1]) ty1 = 1.0;
        if (sz_y - j == bsize[1]) ty1 = 1.0;

        double tz1 = 1.0 - 0.5 / mz2;
        if (mz < bsize[2]) tz1 = 1.0;
        if (sz_z - k == bsize[2]) tz1 = 1.0;

        BBox tbox(Point(tx0, ty0, tz0), Point(tx1, ty1, tz1));

        // Compute BBox.
        double bx1 = Min((i + bsize[0] - 0.5) / (double)sz_x, 1.0);
        if (sz_x - i == bsize[0]) bx1 = 1.0;

        double by1 = Min((j + bsize[1] - 0.5) / (double)sz_y, 1.0);
        if (sz_y - j == bsize[1]) by1 = 1.0;

        double bz1 = Min((k + bsize[2] - 0.5) / (double)sz_z, 1.0);
        if (sz_z - k == bsize[2]) bz1 = 1.0;

        BBox bbox(Point(i==0?0:(i+0.5) / (double)sz_x,
                        j==0?0:(j+0.5) / (double)sz_y,
                        k==0?0:(k+0.5) / (double)sz_z),
                  Point(bx1, by1, bz1));

        TextureBrick *b = new TextureBrick(0, 0, mx2, my2, mz2, numc, numb, 
					   i-(mx2-mx), j-(my2-my), k-(mz2-mz),
					   mx2, my2, mz2, bbox, tbox);
        bricks.push_back(b);
      }
    }
  }
}


} // namespace SLIVR
