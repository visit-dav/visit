
/* 
  This file contains utility functions to handle Uintah metadata
*/

#ifndef UINTAH_IDX_UTILS_H
#define UINTAH_IDX_UTILS_H

#include "LevelInfo.h"
#include "visit_idx_utils.h"
#include <vtkSmartPointer.h>
#include <vtkXMLDataParser.h>
#include <sstream>

extern void ups_parse_vector(vtkXMLDataElement *el, int* vec, int dim);
extern void ups_parse_vector(vtkXMLDataElement *el, double* vec, int dim);

extern void parse_ups(vtkSmartPointer<vtkXMLDataParser> parser, LevelInfo& levelInfo, int dim, bool use_extracells, const std::string grid_type);
extern void parse_timestep(vtkSmartPointer<vtkXMLDataParser> parser, LevelInfo& levelInfo, int dim, bool use_extracells);

static bool overlap(const int& a,const int& b,const int& p,const int& q)
{return ((a)<=(q) && (b)>=(p));}

static bool intersect(bool* over, int* a_low, int* a_high,int* b_low, int* b_high, int* inter_low, int* inter_high){
   
  int overlap_size =1;
  int res1=0;
  int res2=0;
  for(int i=0;i<3;i++){
    inter_low[i]=std::max(a_low[i],b_low[i]);
    inter_high[i]=std::min(a_high[i],b_high[i]);
    int size= (inter_high[i]-inter_low[i]);

    over[i] = size >= 0;//overlap(a_low[i], a_high[i], b_low[i], b_high[i]);
    // if(size<min_dist){
    //   d_min=i;
    //   min_dist=size;
    // }
    //res1 += ((a_low[i] <= b_high[i]) && (a_high[i] >= b_low[i]));
    //res2 += ((b_low[i] <= a_high[i]) && (b_high[i] >= a_low[i]));

    overlap_size*=size;
  }

  //return (res1==3);// || (res2==3);


  // for(int i=0;i<3;i++)
  //   if(i==d_min)
  //     over[i]=true;
  //   else 
  //     over[i]=false; 

  //printf("direction overlap %d size %d\n", d_min, overlap_size);
  return overlap_size>0; 
}

static bool touch(bool* over, int* a_low, int* a_high,int* b_low, int* b_high, int* inter_low, int* inter_high){
   
  int min_dist=999999999; 
  int d_min =-999999;
  int overlap_size =1;
  int size_0 = 0;
  int size_0_d = 0;
  for(int i=0;i<3;i++){
    inter_low[i]=std::max(a_low[i],b_low[i]);
    inter_high[i]=std::min(a_high[i],b_high[i]);
    int size = (inter_high[i]-inter_low[i]);

    if(size == 0){
      size_0++;
      size_0_d = i;
    }

    if(size<min_dist){
      d_min=i;
      min_dist=size;
    }
    overlap_size*=size;
  }

  for(int i=0;i<3;i++)
    if(i==d_min)
      over[i]=true;
    else 
      over[i]=false; 

  if(size_0 == 1){ // one face overlap
    over[size_0_d] = true;
  }

  //printf("direction overlap %d size %d\n", d_min, overlap_size);
  return over[0] || over[1] || over[2]; 
}

#endif //UINTAH_IDX_UTILS_H
