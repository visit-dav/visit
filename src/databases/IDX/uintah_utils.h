
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

extern void parse_ups(vtkSmartPointer<vtkXMLDataParser> parser, LevelInfo& levelInfo, int dim, bool use_extracells);

#endif //UINTAH_IDX_UTILS_H
