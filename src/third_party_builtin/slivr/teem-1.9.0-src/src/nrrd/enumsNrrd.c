/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "nrrd.h"

/*
** Rules of thumb for editing these things.  The airEnum definitions are
** unfortunately EXTREMELY sensitive to small typo errors, and there is
** no good way to detect the errors.  So:
**
** 1) Be awake and undistracted.  Turn down the music.
** 2) When editing the char arrays, make sure that you put commas
**    where you mean them to be.  C's automatic string concatenation 
**    is not your friend here.  In fact, EXPLOIT the fact that you can have
**    a comma after the last element of a list (of strings)- it decreases
**    the chances that adding a new element at the end will be thwarted by 
**    the lack of a comma at the end of the previous (and previously last)
**    string.
** 3) When editing the *StrEqv and *ValEqv arrays, make absolutely
**    sure that both are changed in parallel.  Use only one enum value
**    per line; putting all equivalents on that line, and make sure that
**    there is one line in both *StrEqv and *ValEqv for all the possible
**    enum values, and that there are as many elements in each line.
** 4) Make sure that additions here are reflected in nrrdEnums.h and
**    vice versa.
*/

/* ------------------------ nrrdFormat ------------------------- */

char
_nrrdFormatTypeStr[NRRD_FORMAT_TYPE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_format)",
  "nrrd",
  "pnm",
  "png",
  "vtk",
  "text",
  "eps",
};

char
_nrrdFormatTypeDesc[NRRD_FORMAT_TYPE_MAX+1][AIR_STRLEN_MED] = {
  "unknown_format",
  "native format for nearly raw raster data",
  "Portable aNy Map: includes PGM for grayscale and PPM for color",
  "Portable Network Graphics: lossless compression of 8- and 16-bit data",
  "Visualization ToolKit STRUCTURED_POINTS data",
  "white-space-delimited plain text encoding of 2-D float array",
  "Encapsulated PostScript images",
};

char
_nrrdFormatTypeStrEqv[][AIR_STRLEN_SMALL] = {
  "nrrd",
  "pnm",
  "png",
  "vtk",
  "table", "text", "txt",
  "eps",
  ""
};

int
_nrrdFormatTypeValEqv[] = {
  nrrdFormatTypeNRRD,
  nrrdFormatTypePNM,
  nrrdFormatTypePNG,
  nrrdFormatTypeVTK,
  nrrdFormatTypeText, nrrdFormatTypeText, nrrdFormatTypeText,
  nrrdFormatTypeEPS,
};

airEnum
_nrrdFormatType = {
  "format",
  NRRD_FORMAT_TYPE_MAX,
  _nrrdFormatTypeStr,  NULL,
  _nrrdFormatTypeDesc,
  _nrrdFormatTypeStrEqv, _nrrdFormatTypeValEqv,
  AIR_FALSE
};
airEnum *
nrrdFormatType = &_nrrdFormatType;

/* ------------------------ nrrdType ------------------------- */

char 
_nrrdTypeStr[NRRD_TYPE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_type)",
  "signed char",
  "unsigned char",
  "short",
  "unsigned short",
  "int",
  "unsigned int",
  "long long int",
  "unsigned long long int",
  "float",
  "double",
  "block",
};

char 
_nrrdTypeDesc[NRRD_TYPE_MAX+1][AIR_STRLEN_MED] = {
  "unknown type",
  "signed 1-byte integer",
  "unsigned 1-byte integer",
  "signed 2-byte integer",
  "unsigned 2-byte integer",
  "signed 4-byte integer",
  "unsigned 4-byte integer",
  "signed 8-byte integer",
  "unsigned 8-byte integer",
  "4-byte floating point",
  "8-byte floating point",
  "size user-defined at run-time",
};

#define ntCH nrrdTypeChar
#define ntUC nrrdTypeUChar
#define ntSH nrrdTypeShort
#define ntUS nrrdTypeUShort
#define ntIN nrrdTypeInt
#define ntUI nrrdTypeUInt
#define ntLL nrrdTypeLLong
#define ntUL nrrdTypeULLong
#define ntFL nrrdTypeFloat
#define ntDB nrrdTypeDouble
#define ntBL nrrdTypeBlock

char
_nrrdTypeStrEqv[][AIR_STRLEN_SMALL] = {
  "signed char", /* but NOT just "char" */ "int8", "int8_t",
  "uchar", "unsigned char", "uint8", "uint8_t", 
  "short", "short int", "signed short", "signed short int", "int16", "int16_t",
  "ushort", "unsigned short", "unsigned short int", "uint16", "uint16_t", 
  "int", "signed int", "int32", "int32_t", 
  "uint", "unsigned int", "uint32", "uint32_t",
  "longlong", "long long", "long long int", "signed long long",
               "signed long long int", "int64", "int64_t", 
  "ulonglong", "unsigned long long", "unsigned long long int",
               "uint64", "uint64_t", 
  "float",
  "double",
  "block",
  ""
};

int
_nrrdTypeValEqv[] = {
  ntCH, ntCH, ntCH,
  ntUC, ntUC, ntUC, ntUC,
  ntSH, ntSH, ntSH, ntSH, ntSH, ntSH,
  ntUS, ntUS, ntUS, ntUS, ntUS,
  ntIN, ntIN, ntIN, ntIN,
  ntUI, ntUI, ntUI, ntUI, 
  ntLL, ntLL, ntLL, ntLL, ntLL, ntLL, ntLL, 
  ntUL, ntUL, ntUL, ntUL, ntUL, 
  ntFL,
  ntDB,
  ntBL,
};

airEnum
_nrrdType = {
  "type",
  NRRD_TYPE_MAX,
  _nrrdTypeStr, NULL,
  _nrrdTypeDesc,
  _nrrdTypeStrEqv, _nrrdTypeValEqv,
  AIR_FALSE
};
airEnum *
nrrdType = &_nrrdType;

/* ------------------------ nrrdEncodingType ------------------------- */

char
_nrrdEncodingTypeStr[NRRD_ENCODING_TYPE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_encoding)",
  "raw",
  "ascii",
  "hex",
  "gz",
  "bz2",
};

char
_nrrdEncodingTypeDesc[NRRD_ENCODING_TYPE_MAX+1][AIR_STRLEN_MED] = {
  "unknown encoding",
  "file is byte-for-byte same as memory representation",
  "values written out in ASCII",
  "case-insenstive hexadecimal encoding (2 chars / byte)",
  "gzip compression of binary encoding",
  "bzip2 compression of binary encoding",
};

char
_nrrdEncodingTypeStrEqv[][AIR_STRLEN_SMALL] = {
  "(unknown_encoding)",
  "raw",
  "txt", "text", "ascii",
  "hex",
  "gz", "gzip",
  "bz2", "bzip2",
  ""
};

int
_nrrdEncodingTypeValEqv[] = {
  nrrdEncodingTypeUnknown,
  nrrdEncodingTypeRaw,
  nrrdEncodingTypeAscii, nrrdEncodingTypeAscii, nrrdEncodingTypeAscii,
  nrrdEncodingTypeHex,
  nrrdEncodingTypeGzip, nrrdEncodingTypeGzip,
  nrrdEncodingTypeBzip2, nrrdEncodingTypeBzip2,
};

airEnum
_nrrdEncodingType = {
  "encoding",
  NRRD_ENCODING_TYPE_MAX,
  _nrrdEncodingTypeStr, NULL,
  _nrrdEncodingTypeDesc,
  _nrrdEncodingTypeStrEqv, _nrrdEncodingTypeValEqv,
  AIR_FALSE
};
airEnum *
nrrdEncodingType = &_nrrdEncodingType;

/* ------------------------ nrrdCenter ------------------------- */

char
_nrrdCenterStr[NRRD_CENTER_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_center)",
  "node",
  "cell",
};

char
_nrrdCenterDesc[NRRD_CENTER_MAX+1][AIR_STRLEN_MED] = {
  "unknown centering",
  "samples are at boundaries between elements along axis",
  "samples are at centers of elements along axis",
};

airEnum
_nrrdCenter_enum = {
  "centering",
  NRRD_CENTER_MAX,
  _nrrdCenterStr, NULL,
  _nrrdCenterDesc,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
nrrdCenter = &_nrrdCenter_enum;

/* ------------------------ nrrdKind ------------------------- */

/*
  nrrdKindUnknown,
  nrrdKindDomain,            *  1: any image domain *
  nrrdKindSpace,             *  2: a spatial domain *
  nrrdKindTime,              *  3: a temporal domain *
  * -------------------------- end domain kinds *
  * -------------------------- begin range kinds *
  nrrdKindList,              *  4: any list of values, non-resample-able *
  nrrdKindPoint,             *  5: coords of a point *
  nrrdKindVector,            *  6: coeffs of (contravariant) vector *
  nrrdKindCovariantVector,   *  7: coeffs of covariant vector (eg gradient) *
  nrrdKindNormal,            *  8: coeffs of unit-length covariant vector *
  * -------------------------- end arbitrary size kinds *
  * -------------------------- begin size-specific kinds *
  nrrdKindStub,              *  9: axis with one sample (a placeholder) *
  nrrdKindScalar,            * 10: effectively, same as a stub *
  nrrdKindComplex,           * 11: real and imaginary components *
  nrrdKind2Vector,           * 12: 2 component vector *
  nrrdKind3Color,            * 13: ANY 3-component color value *
  nrrdKindRGBColor,          * 14: RGB, no colorimetry *
  nrrdKindHSVColor,          * 15: HSV, no colorimetry *
  nrrdKindXYZColor,          * 16: perceptual primary colors *
  nrrdKind4Color,            * 17: ANY 4-component color value *
  nrrdKindRGBAColor,         * 18: RGBA, no colorimetry *
  nrrdKind3Vector,           * 19: 3-component vector *
  nrrdKind3Gradient,         * 20: 3-component covariant vector *
  nrrdKind3Normal,           * 21: 3-component covector, assumed normalized *
  nrrdKind4Vector,           * 22: 4-component vector *
  nrrdKindQuaternion,        * 23: (w,x,y,z), not necessarily normalized *
  nrrdKind2DSymMatrix,       * 24: Mxx Mxy Myy *
  nrrdKind2DMaskedSymMatrix, * 25: mask Mxx Mxy Myy *
  nrrdKind2DMatrix,          * 26: Mxx Mxy Myx Myy *
  nrrdKind2DMaskedMatrix,    * 27: mask Mxx Mxy Myx Myy *
  nrrdKind3DSymMatrix,       * 28: Mxx Mxy Mxz Myy Myz Mzz *
  nrrdKind3DMaskedSymMatrix, * 29: mask Mxx Mxy Mxz Myy Myz Mzz *
  nrrdKind3DMatrix,          * 30: Mxx Mxy Mxz Myx Myy Myz Mzx Mzy Mzz *
  nrrdKind3DMaskedMatrix,    * 31: mask Mxx Mxy Mxz Myx Myy Myz Mzx Mzy Mzz *
*/

char
_nrrdKindStr[NRRD_KIND_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_kind)",
  "domain",
  "space",
  "time",
  "list",
  "point",
  "vector",
  "covariant-vector",
  "normal",
  "stub",
  "scalar",
  "complex",
  "2-vector",
  "3-color",
  "RGB-color",
  "HSV-color",
  "XYZ-color",
  "4-color",
  "RGBA-color",
  "3-vector",
  "3-gradient",
  "3-normal",
  "4-vector",
  "quaternion",
  "2D-symmetric-matrix",
  "2D-masked-symmetric-matrix",
  "2D-matrix",
  "2D-masked-matrix",
  "3D-symmetric-matrix",
  "3D-masked-symmetric-matrix",
  "3D-matrix",
  "3D-masked-matrix",
};

char
_nrrdKindDesc[NRRD_KIND_MAX+1][AIR_STRLEN_MED] = {
  "unknown kind",
  "a domain variable of the function which the nrrd samples",
  "a spatial domain, like the axes of a measured volume image",
  "a temporal domain, as from time-varying measurements",
  "some list of attributes; it makes no sense to resample along these",
  "coordinates of a point",
  "coefficients of a (contravariant) vector",
  "coefficients of a covariant vector, such as a gradient",
  "coefficients of a normalized covariant vector",
  "a place-holder axis with a single sample",
  "axis used to indicate that the nrrd contains a scalar value",
  "real and imaginary parts of a value",
  "a 2-component vector",
  "any 3-component color value",
  "red-green-blue color",
  "hue-saturation-value single hexcone color",
  "perceptual primaries color",
  "any 4-component color value",
  "red-green-blue-alpha color",
  "a 3-element (contravariant) vector",
  "a 3-element gradient (covariant) vector",
  "a 3-element (covariant) vector which is assumed normalized",
  "a 4-element (contravariant) vector",
  "quaternion: x y z w",
  "3 elements of 2D symmetric matrix: Mxx Mxy Myy",
  "mask plus 3 elements of 2D symmetric matrix: mask Mxx Mxy Myy",
  "4 elements of general 2D matrix: Mxx Mxy Myx Myy",
  "mask plus 4 elements of general 2D matrix: mask Mxx Mxy Myx Myy",
  "6 elements of 3D symmetric matrix: Mxx Mxy Mxz Myy Myz Mzz",
  "mask plus 6 elements of 3D symmetric matrix: mask Mxx Mxy Mxz Myy Myz Mzz",
  "9 elements of general 3D matrix: Mxx Mxy Mxz Myx Myy Myz Mzx Mzy Mzz",
  "mask plus 9 elements of general 3D matrix: mask Mxx Mxy Mxz Myx Myy Myz Mzx Mzy Mzz",
};

char
_nrrdKindStr_Eqv[][AIR_STRLEN_SMALL] = {
  "domain",
  "space",
  "time",
  "list",
  "point",
  "vector", "contravariant-vector",
  "covariant-vector",
  "normal",
  "stub",
  "scalar",
  "complex",
  "2-vector",
  "3-color",
  "RGB-color", "RGBcolor", "RGB",
  "HSV-color", "HSVcolor", "HSV",
  "XYZ-color",
  "4-color",
  "RGBA-color", "RGBAcolor", "RGBA",
  "3-vector",
  "3-gradient",
  "3-normal",
  "4-vector",
  "quaternion",
  "2D-symmetric-matrix", "2D-sym-matrix", 
        "2D-symmetric-tensor", "2D-sym-tensor",
  "2D-masked-symmetric-matrix", "2D-masked-sym-matrix",
        "2D-masked-symmetric-tensor", "2D-masked-sym-tensor",
  "2D-matrix",
        "2D-tensor",
  "2D-masked-matrix",
        "2D-masked-tensor",
  "3D-symmetric-matrix", "3D-sym-matrix",
        "3D-symmetric-tensor", "3D-sym-tensor",
  "3D-masked-symmetric-matrix", "3D-masked-sym-matrix",
        "3D-masked-symmetric-tensor", "3D-masked-sym-tensor",
  "3D-matrix",
        "3D-tensor",
  "3D-masked-matrix",
        "3D-masked-tensor",
  ""
};

int
_nrrdKindVal_Eqv[] = {
  nrrdKindDomain,
  nrrdKindSpace,
  nrrdKindTime,
  nrrdKindList,
  nrrdKindPoint,
  nrrdKindVector, nrrdKindVector, 
  nrrdKindCovariantVector,
  nrrdKindNormal,
  nrrdKindStub,
  nrrdKindScalar,
  nrrdKindComplex,
  nrrdKind2Vector,
  nrrdKind3Color,
  nrrdKindRGBColor, nrrdKindRGBColor, nrrdKindRGBColor,
  nrrdKindHSVColor, nrrdKindHSVColor, nrrdKindHSVColor,
  nrrdKindXYZColor,
  nrrdKind4Color,
  nrrdKindRGBAColor, nrrdKindRGBAColor, nrrdKindRGBAColor,
  nrrdKind3Vector,
  nrrdKind3Gradient,
  nrrdKind3Normal,
  nrrdKind4Vector,
  nrrdKindQuaternion,
  nrrdKind2DSymMatrix, nrrdKind2DSymMatrix,
        nrrdKind2DSymMatrix, nrrdKind2DSymMatrix,
  nrrdKind2DMaskedSymMatrix, nrrdKind2DMaskedSymMatrix,
        nrrdKind2DMaskedSymMatrix, nrrdKind2DMaskedSymMatrix,
  nrrdKind2DMatrix,
        nrrdKind2DMatrix,
  nrrdKind2DMaskedMatrix,
        nrrdKind2DMaskedMatrix,
  nrrdKind3DSymMatrix, nrrdKind3DSymMatrix,
        nrrdKind3DSymMatrix, nrrdKind3DSymMatrix,
  nrrdKind3DMaskedSymMatrix, nrrdKind3DMaskedSymMatrix,
        nrrdKind3DMaskedSymMatrix, nrrdKind3DMaskedSymMatrix,
  nrrdKind3DMatrix,
        nrrdKind3DMatrix,
  nrrdKind3DMaskedMatrix,
        nrrdKind3DMaskedMatrix,
};

airEnum
_nrrdKind_enum = {
  "kind",
  NRRD_KIND_MAX,
  _nrrdKindStr, NULL,
  _nrrdKindDesc,
  _nrrdKindStr_Eqv, _nrrdKindVal_Eqv,
  AIR_FALSE
};
airEnum *
nrrdKind = &_nrrdKind_enum;

/* ------------------------ nrrdField ------------------------- */

char
_nrrdFieldStr[NRRD_FIELD_MAX+1][AIR_STRLEN_SMALL] = {
  "Ernesto \"Che\" Guevara",
  "#",
  "content",
  "number",
  "type",
  "block size",
  "dimension",
  "space",
  "space dimension",
  "sizes",
  "spacings",
  "thicknesses",
  "axis mins",
  "axis maxs",
  "space directions",
  "centerings",
  "kinds",
  "labels",
  "units",
  "min",
  "max",
  "old min",
  "old max",
  "endian",
  "encoding",
  "line skip",
  "byte skip",
  "key/value",
  "sample units",
  "space units",
  "space origin",
  "measurement frame",
  "data file",
};

char
_nrrdFieldDesc[NRRD_FIELD_MAX+1][AIR_STRLEN_MED] = {
  "unknown field identifier",
  "comment",
  "short description of whole array and/or its provenance",
  "total number of samples in array",
  "type of sample value",
  "number of bytes in one block (for block-type)",
  "number of axes in array",
  "identifier for space in which array grid lies",
  "dimension of space in which array grid lies",
  "list of number of samples along each axis, aka \"dimensions\" of the array",
  "list of sample spacings along each axis",
  "list of sample thicknesses along each axis",
  "list of minimum positions associated with each axis",
  "list of maximum positions associated with each axis",
  "list of direction inter-sample vectors for each axis",
  "list of sample centerings for each axis",
  "list of kinds for each axis",
  "list of short descriptions for each axis",
  "list of units in which each axes' spacing and thickness is measured",
  "supposed minimum array value",
  "supposed maximum array value",
  "minimum array value prior to quantization",
  "maximum array value prior to quantization",
  "endiannes of data as written in file",
  "encoding of data written in file",
  "number of lines to skip prior to byte skip and reading data",
  "number of bytes to skip after line skip and prior to reading data",
  "string-based key/value pairs",
  "units of measurement of (scalar) values inside array itself",
  "list of units for measuring origin and direct vectors' coefficients",
  "location in space of center of first (lowest memory address) sample",
  "maps coords of (non-scalar) values to coords of surrounding space",
  "with detached headers, where is data to be found",
};

char
_nrrdFieldStrEqv[][AIR_STRLEN_SMALL]  = {
  "#",
  "content",
  "number",
  "type",
  "block size", "blocksize",
  "dimension",
  "space",
  "space dimension", "spacedimension",
  "sizes",
  "spacings",
  "thicknesses",
  "axis mins", "axismins",
  "axis maxs", "axismaxs",
  "space directions", "spacedirections",
  "centers", "centerings",
  "kinds",
  "labels",
  "units",
  "min",
  "max",
  "old min", "oldmin",
  "old max", "oldmax",
  "endian",
  "encoding",
  "line skip", "lineskip",
  "byte skip", "byteskip",
  /* nothing for keyvalue */
  "sample units", "sampleunits",
  "space units", "spaceunits",
  "space origin", "spaceorigin",
  "measurement frame", "measurementframe", 
  "data file", "datafile",
  ""
};

int
_nrrdFieldValEqv[] = {
  nrrdField_comment,
  nrrdField_content,
  nrrdField_number,
  nrrdField_type,
  nrrdField_block_size, nrrdField_block_size,
  nrrdField_dimension,
  nrrdField_space,
  nrrdField_space_dimension, nrrdField_space_dimension,
  nrrdField_sizes,
  nrrdField_spacings,
  nrrdField_thicknesses,
  nrrdField_axis_mins, nrrdField_axis_mins,
  nrrdField_axis_maxs, nrrdField_axis_maxs,
  nrrdField_space_directions, nrrdField_space_directions,
  nrrdField_centers, nrrdField_centers,
  nrrdField_kinds,
  nrrdField_labels,
  nrrdField_units,
  nrrdField_min,
  nrrdField_max,
  nrrdField_old_min, nrrdField_old_min,
  nrrdField_old_max, nrrdField_old_max,
  nrrdField_endian,
  nrrdField_encoding,
  nrrdField_line_skip, nrrdField_line_skip,
  nrrdField_byte_skip, nrrdField_byte_skip,
  /* nothing for keyvalue */
  nrrdField_sample_units, nrrdField_sample_units,
  nrrdField_space_units, nrrdField_space_units,
  nrrdField_space_origin, nrrdField_space_origin,
  nrrdField_measurement_frame, nrrdField_measurement_frame,
  nrrdField_data_file, nrrdField_data_file,
};

airEnum
_nrrdField = {
  "nrrd_field",
  NRRD_FIELD_MAX,
  _nrrdFieldStr, NULL,
  _nrrdFieldDesc,
  _nrrdFieldStrEqv, _nrrdFieldValEqv, 
  AIR_FALSE  /* field identifiers not case sensitive */
};
airEnum *
nrrdField = &_nrrdField;

/* ------------------------ nrrdSpace ------------------------- */

/*
  nrrdSpaceUnknown,
  nrrdSpaceRightAnteriorSuperior,     *  1: NIFTI-1 (right-handed) *
  nrrdSpaceLeftAnteriorSuperior,      *  2: standard Analyze (left-handed) *
  nrrdSpaceLeftPosteriorSuperior,     *  3: DICOM 3.0 (right-handed) *
  nrrdSpaceRightAnteriorSuperiorTime, *  4: *
  nrrdSpaceLeftAnteriorSuperiorTime,  *  5: *
  nrrdSpaceLeftPosteriorSuperiorTime, *  6: *
  nrrdSpaceScannerXYZ,                *  7: ACR/NEMA 2.0 (pre-DICOM 3.0) *
  nrrdSpaceScannerXYZTime,            *  8: *
  nrrdSpace3DRightHanded,             *  9: *
  nrrdSpace3DLeftHanded,              * 10: *
  nrrdSpace3DRightHandedTime,         * 11: *
  nrrdSpace3DLeftHandedTime,          * 12: *
  nrrdSpaceLast
*/

char
_nrrdSpaceStr[NRRD_SPACE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_space)",
  "right-anterior-superior",
  "left-anterior-superior",
  "left-posterior-superior",
  "right-anterior-superior-time",
  "left-anterior-superior-time",
  "left-posterior-superior-time",
  "scanner-xyz",
  "scanner-xyz-time",
  "3D-right-handed",
  "3D-left-handed",
  "3D-right-handed-time",
  "3D-left-handed-time",
};

char
_nrrdSpaceDesc[NRRD_SPACE_MAX+1][AIR_STRLEN_MED] = {
  "unknown space",
  "right-anterior-superior (used in NIFTI-1 and SPL's 3D Slicer)",
  "left-anterior-superior (used in Analyze 7.5)",
  "left-posterior-superior (used in DICOM 3)",
  "right-anterior-superior-time",
  "left-anterior-superior-time",
  "left-posterior-superior-time",
  "scanner-xyz (used in ACR/NEMA 2.0)",
  "scanner-xyz-time",
  "3D-right-handed",
  "3D-left-handed",
  "3D-right-handed-time",
  "3D-left-handed-time",
};

char
_nrrdSpaceStrEqv[][AIR_STRLEN_SMALL] = {
  "(unknown_space)",
  "right-anterior-superior", "right anterior superior",
      "rightanteriorsuperior", "RAS",
  "left-anterior-superior", "left anterior superior",
      "leftanteriorsuperior", "LAS",
  "left-posterior-superior", "left posterior superior",
      "leftposteriorsuperior", "LPS",
  "right-anterior-superior-time", "right anterior superior time",
      "rightanteriorsuperiortime", "RAST",
  "left-anterior-superior-time", "left anterior superior time",
      "leftanteriorsuperiortime", "LAST",
  "left-posterior-superior-time", "left posterior superior time",
      "leftposteriorsuperiortime", "LPST",
  "scanner-xyz",
  "scanner-xyz-time", "scanner-xyzt", 
  "3D-right-handed", "3D right handed", "3Drighthanded"
  "3D-left-handed", "3D left handed", "3Dlefthanded",
  "3D-right-handed-time", "3D right handed time",
      "3Drighthandedtime",
  "3D-left-handed-time", "3D left handed time",
      "3Dlefthandedtime",
  ""
};

int
_nrrdSpaceValEqv[] = {
  nrrdSpaceUnknown,
  nrrdSpaceRightAnteriorSuperior, nrrdSpaceRightAnteriorSuperior,
     nrrdSpaceRightAnteriorSuperior, nrrdSpaceRightAnteriorSuperior,
  nrrdSpaceLeftAnteriorSuperior, nrrdSpaceLeftAnteriorSuperior,
     nrrdSpaceLeftAnteriorSuperior, nrrdSpaceLeftAnteriorSuperior,
  nrrdSpaceLeftPosteriorSuperior, nrrdSpaceLeftPosteriorSuperior,
     nrrdSpaceLeftPosteriorSuperior, nrrdSpaceLeftPosteriorSuperior,
  nrrdSpaceRightAnteriorSuperiorTime, nrrdSpaceRightAnteriorSuperiorTime,
     nrrdSpaceRightAnteriorSuperiorTime, nrrdSpaceRightAnteriorSuperiorTime,
  nrrdSpaceLeftAnteriorSuperiorTime, nrrdSpaceLeftAnteriorSuperiorTime,
     nrrdSpaceLeftAnteriorSuperiorTime, nrrdSpaceLeftAnteriorSuperiorTime,
  nrrdSpaceLeftPosteriorSuperiorTime, nrrdSpaceLeftPosteriorSuperiorTime,
     nrrdSpaceLeftPosteriorSuperiorTime, nrrdSpaceLeftPosteriorSuperiorTime,
  nrrdSpaceScannerXYZ,
  nrrdSpaceScannerXYZTime, nrrdSpaceScannerXYZTime, 
  nrrdSpace3DRightHanded, nrrdSpace3DRightHanded, nrrdSpace3DRightHanded,
  nrrdSpace3DLeftHanded, nrrdSpace3DLeftHanded, nrrdSpace3DLeftHanded,
  nrrdSpace3DRightHandedTime, nrrdSpace3DRightHandedTime, 
     nrrdSpace3DRightHandedTime,
  nrrdSpace3DLeftHandedTime, nrrdSpace3DLeftHandedTime,
     nrrdSpace3DLeftHandedTime
};

airEnum
_nrrdSpace = {
  "space",
  NRRD_SPACE_MAX,
  _nrrdSpaceStr, NULL,
  _nrrdSpaceDesc,
  _nrrdSpaceStrEqv, _nrrdSpaceValEqv,
  AIR_FALSE
};
airEnum *
nrrdSpace = &_nrrdSpace;

/* ---- BEGIN non-NrrdIO */

/* ------------------------ nrrdBoundary ------------------------- */

char
_nrrdBoundaryStr[NRRD_BOUNDARY_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_boundary)",
  "pad",
  "bleed",
  "wrap",
  "weight",
};

char
_nrrdBoundaryDesc[NRRD_BOUNDARY_MAX+1][AIR_STRLEN_MED] = {
  "unknown boundary behavior",
  "pad with some specified value",
  "copy values from edge outward as needed",
  "wrap around to other end of axis",
  "re-weight (by normalization) samples within axis range",
};

airEnum
_nrrdBoundary = {
  "boundary behavior",
  NRRD_BOUNDARY_MAX,
  _nrrdBoundaryStr, NULL,
  _nrrdBoundaryDesc,
  NULL, NULL,
  AIR_FALSE
};
airEnum *
nrrdBoundary = &_nrrdBoundary;

/* ------------------------ nrrdMeasure ------------------------- */

char
_nrrdMeasureStr[NRRD_MEASURE_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_measure)",
  "min",
  "max",
  "mean",
  "median",
  "mode",
  "product",
  "sum",
  "L1",
  "L2",
  "Linf",
  "variance",
  "SD",
  "skew",
  "line-slope",
  "line-intercept",
  "line-error",
  "histo-min",
  "histo-max",
  "histo-mean",
  "histo-median",
  "histo-mode",
  "histo-product",
  "histo-sum",
  "histo-L2",
  "histo-variance",
  "histo-SD",
};

char
_nrrdMeasureDesc[NRRD_MEASURE_MAX+1][AIR_STRLEN_MED] = {
  "unknown measure",
  "minimum of values",
  "maximum of values",
  "mean of values",
  "median of values",
  "mode of values",
  "product of values",
  "sum of values",
  "L1 norm of values",
  "L2 norm of values",
  "Linf norm of values",
  "variance of values",
  "standard deviation of values",
  "skew of values",
  "slope of line of best fit",
  "y-intercept of line of best fit",
  "error of line fitting",
  "minimum of histogrammed values",
  "maximum of histogrammed values",
  "mean of histogrammed values",
  "median of histogrammed values",
  "mode of histogrammed values",
  "product of histogrammed values",
  "sum of histogrammed values",
  "L2 norm of histogrammed values",
  "variance of histogrammed values",
  "standard deviation of histogrammed values",
};

char
_nrrdMeasureStrEqv[][AIR_STRLEN_SMALL] = {
  "(unknown_measure)",
  "min",
  "max",
  "mean",
  "median",
  "mode",
  "product", "prod",
  "sum",
  "L1",
  "L2",
  "Linf",
  "variance", "var",
  "SD", "stdv",
  "skew", "skewness",
  "slope", "line-slope",
  "intc", "intercept", "line-intc", "line-intercept",
  "error", "line-error",
  "histo-min",
  "histo-max",
  "histo-mean",
  "histo-median",
  "histo-mode",
  "histo-product",
  "histo-sum",
  "histo-l2",
  "histo-variance", "histo-var",
  "histo-sd",
  ""
};

int
_nrrdMeasureValEqv[] = {
  nrrdMeasureUnknown,
  nrrdMeasureMin,
  nrrdMeasureMax,
  nrrdMeasureMean,
  nrrdMeasureMedian,
  nrrdMeasureMode,
  nrrdMeasureProduct, nrrdMeasureProduct,
  nrrdMeasureSum,
  nrrdMeasureL1,
  nrrdMeasureL2,
  nrrdMeasureLinf,
  nrrdMeasureVariance, nrrdMeasureVariance,
  nrrdMeasureSD, nrrdMeasureSD,
  nrrdMeasureSkew, nrrdMeasureSkew,
  nrrdMeasureLineSlope, nrrdMeasureLineSlope,
  nrrdMeasureLineIntercept, nrrdMeasureLineIntercept,
     nrrdMeasureLineIntercept, nrrdMeasureLineIntercept,
  nrrdMeasureLineError, nrrdMeasureLineError,
  nrrdMeasureHistoMin,
  nrrdMeasureHistoMax,
  nrrdMeasureHistoMean,
  nrrdMeasureHistoMedian,
  nrrdMeasureHistoMode,
  nrrdMeasureHistoProduct,
  nrrdMeasureHistoSum,
  nrrdMeasureHistoL2,
  nrrdMeasureHistoVariance, nrrdMeasureHistoVariance,
  nrrdMeasureHistoSD,
};

airEnum
_nrrdMeasure = {
  "measure",
  NRRD_MEASURE_MAX,
  _nrrdMeasureStr, NULL,
  _nrrdMeasureDesc,
  _nrrdMeasureStrEqv, _nrrdMeasureValEqv, 
  AIR_FALSE
};
airEnum *
nrrdMeasure = &_nrrdMeasure;

/* ------------------------ nrrdUnaryOp ---------------------- */

#define nuNeg nrrdUnaryOpNegative
#define nuRcp nrrdUnaryOpReciprocal
#define nuSin nrrdUnaryOpSin
#define nuCos nrrdUnaryOpCos
#define nuTan nrrdUnaryOpTan
#define nuAsn nrrdUnaryOpAsin
#define nuAcs nrrdUnaryOpAcos
#define nuAtn nrrdUnaryOpAtan
#define nuExp nrrdUnaryOpExp
#define nuLge nrrdUnaryOpLog
#define nuLg2 nrrdUnaryOpLog2
#define nuLgt nrrdUnaryOpLog10
#define nuL1p nrrdUnaryOpLog1p
#define nuSqt nrrdUnaryOpSqrt
#define nuCbt nrrdUnaryOpCbrt
#define nuErf nrrdUnaryOpErf
#define nuCil nrrdUnaryOpCeil
#define nuFlr nrrdUnaryOpFloor
#define nuRup nrrdUnaryOpRoundUp
#define nuRdn nrrdUnaryOpRoundDown
#define nuAbs nrrdUnaryOpAbs
#define nuSgn nrrdUnaryOpSgn
#define nuExs nrrdUnaryOpExists
#define nuRnd nrrdUnaryOpRand
#define nuNrn nrrdUnaryOpNormalRand
#define nuZer nrrdUnaryOpZero
#define nuOne nrrdUnaryOpOne

char 
_nrrdUnaryOpStr[NRRD_UNARY_OP_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_unary_op)",
  "-",
  "r",
  "sin",
  "cos",
  "tan",
  "asin",
  "acos",
  "atan",
  "exp",
  "log",
  "log2",
  "log10",
  "log1p",
  "sqrt",
  "cbrt",
  "erf",
  "ceil",
  "floor",
  "roundup",
  "rounddown",
  "abs",
  "sgn",
  "exists",
  "rand",
  "normrand",
  "zero",
  "one"
};

char 
_nrrdUnaryOpDesc[NRRD_UNARY_OP_MAX+1][AIR_STRLEN_MED] = {
  "unknown unary op",
  "negative; additive inverse",
  "reciprocal; multiplicative inverse",
  "sin",
  "cos",
  "tan",
  "arcsin",
  "arccos",
  "arctan",
  "e raised to something",
  "natural (base e) logarithm",
  "base 2 logarithm",
  "base 10 logarithm",
  "ln(1+x), accurate for small x",
  "square root",
  "cube root",
  "error function (integral of gaussian)",
  "smallest integer greater than or equal",
  "largest integer less than or equal",
  "round to closest integer (0.5 rounded to 1)",
  "round to closest integer (0.5 rounded to 0)",
  "absolute value",
  "sign of value (-1, 0, or 1)",
  "value is not infinity or NaN",
  "uniformly distributed random value between 0 and 1",
  "normally distributed random value, mean 0, stdv 1",
  "always zero",
  "always one"
};

char
_nrrdUnaryOpStrEqv[][AIR_STRLEN_SMALL] = {
  "-", "neg", "negative", "minus",
  "r", "recip",
  "sin",
  "cos",
  "tan",
  "asin", "arcsin",
  "acos", "arccos",
  "atan", "arctan",
  "exp",
  "ln", "log",
  "log2",
  "log10",
  "ln1p", "log1p",
  "sqrt",
  "cbrt",
  "erf",
  "ceil",
  "floor",
  "roundup", "rup",
  "rounddown", "rdown", "rdn",
  "abs", "fabs",
  "sgn", "sign",
  "exists",
  "rand",
  "normalrand", "normrand", "nrand",
  "zero", "0",
  "one", "1",
  ""
};

int
_nrrdUnaryOpValEqv[] = {
  nuNeg, nuNeg, nuNeg, nuNeg,
  nuRcp, nuRcp,
  nuSin,
  nuCos,
  nuTan,
  nuAsn, nuAsn,
  nuAcs, nuAcs,
  nuAtn, nuAtn,
  nuExp,
  nuLge, nuLge,
  nuLg2,
  nuLgt,
  nuL1p, nuL1p,
  nuSqt,
  nuCbt,
  nuErf,
  nuCil,
  nuFlr,
  nuRup, nuRup,
  nuRdn, nuRdn, nuRdn,
  nuAbs, nuAbs,
  nuSgn, nuSgn,
  nuExs,
  nuRnd,
  nuNrn, nuNrn, nuNrn,
  nuZer, nuZer,
  nuOne, nuOne
};

airEnum
_nrrdUnaryOp_enum = {
  "unary op",
  NRRD_UNARY_OP_MAX,
  _nrrdUnaryOpStr, NULL,
  _nrrdUnaryOpDesc,
  _nrrdUnaryOpStrEqv, _nrrdUnaryOpValEqv,
  AIR_FALSE
};
airEnum *
nrrdUnaryOp = &_nrrdUnaryOp_enum;

/* ------------------------ nrrdBinaryOp ---------------------- */

char 
_nrrdBinaryOpStr[NRRD_BINARY_OP_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_binary_op)",
  "+",
  "-",
  "x",
  "/",
  "^",
  "spow",
  "%",
  "fmod",
  "atan2",
  "min",
  "max",
  "lt",
  "lte",
  "gt",
  "gte",
  "comp",
  "eq",
  "neq",
  "exists",
  "if",
};

char 
_nrrdBinaryOpDesc[NRRD_BINARY_OP_MAX+1][AIR_STRLEN_MED] = {
  "unknown binary op",
  "add",
  "subtract",
  "multiply",
  "divide",
  "power",
  "signed power",
  "integer modulo",
  "fractional modulo",
  "two-argment arctangent based on atan2()",
  "miniumum",
  "maximum",
  "less then",
  "less then or equal",
  "greater than",
  "greater than or equal",
  "compare (resulting in -1, 0, or 1)",
  "equal",
  "not equal",
  "if exists(a), then a, else b",
  "if a, then a, else b",
};

#define nbAdd nrrdBinaryOpAdd
#define nbSub nrrdBinaryOpSubtract
#define nbMul nrrdBinaryOpMultiply
#define nbDiv nrrdBinaryOpDivide
#define nbPow nrrdBinaryOpPow
#define nbSpw nrrdBinaryOpSgnPow
#define nbMod nrrdBinaryOpMod
#define nbFmd nrrdBinaryOpFmod
#define nbAtn nrrdBinaryOpAtan2
#define nbMin nrrdBinaryOpMin
#define nbMax nrrdBinaryOpMax
#define nbLt  nrrdBinaryOpLT
#define nbLte nrrdBinaryOpLTE
#define nbGt  nrrdBinaryOpGT
#define nbGte nrrdBinaryOpGTE
#define nbCmp nrrdBinaryOpCompare
#define nbEq  nrrdBinaryOpEqual
#define nbNeq nrrdBinaryOpNotEqual
#define nbExt nrrdBinaryOpExists
#define nbIf  nrrdBinaryOpIf

char
_nrrdBinaryOpStrEqv[][AIR_STRLEN_SMALL] = {
  "+", "plus", "add",
  "-", "minus", "subtract", "sub", 
  "x", "*", "times", "multiply", "product",
  "/", "divide", "quotient",
  "^", "pow", "power",
  "spow", "sgnpow", "sgnpower",
  "%", "mod", "modulo",
  "fmod",
  "atan2", 
  "min", "minimum",
  "max", "maximum",
  "lt", "<", "less", "lessthan",
  "lte", "<=", "lessthanorequal",
  "gt", ">", "greater", "greaterthan",
  "gte", ">=", "greaterthanorequal",
  "comp", "compare",
  "eq", "=", "==", "equal",
  "neq", "ne", "!=", "notequal",
  "exists",
  "if",
  ""
};

int
_nrrdBinaryOpValEqv[] = {
  nbAdd, nbAdd, nbAdd,
  nbSub, nbSub, nbSub, nbSub, 
  nbMul, nbMul, nbMul, nbMul, nbMul, 
  nbDiv, nbDiv, nbDiv, 
  nbPow, nbPow, nbPow,
  nbSpw, nbSpw, nbSpw,
  nbMod, nbMod, nbMod, 
  nbFmd,
  nbAtn,
  nbMin, nbMin,
  nbMax, nbMax,
  nbLt, nbLt, nbLt, nbLt,
  nbLte, nbLte, nbLte,
  nbGt, nbGt, nbGt, nbGt,
  nbGte, nbGte, nbGte,
  nbCmp, nbCmp,
  nbEq, nbEq, nbEq, nbEq,
  nbNeq, nbNeq, nbNeq, nbNeq,
  nbExt,
  nbIf
};

airEnum
_nrrdBinaryOp_enum = {
  "binary op",
  NRRD_BINARY_OP_MAX,
  _nrrdBinaryOpStr, NULL,
  _nrrdBinaryOpDesc,
  _nrrdBinaryOpStrEqv, _nrrdBinaryOpValEqv,
  AIR_FALSE
};
airEnum *
nrrdBinaryOp = &_nrrdBinaryOp_enum;

/* ------------------------ nrrdTernaryOp ---------------------- */

char 
_nrrdTernaryOpStr[NRRD_TERNARY_OP_MAX+1][AIR_STRLEN_SMALL] = {
  "(unknown_ternary_op)",
  "add",
  "multiply",
  "min",
  "max",
  "clamp",
  "ifelse",
  "lerp",
  "exists",
  "in_op",
  "in_cl",
};

char 
_nrrdTernaryOpDesc[NRRD_TERNARY_OP_MAX+1][AIR_STRLEN_MED] = {
  "unknown ternary op",
  "add three values",
  "multiply three values",
  "minimum of three values",
  "maximum of three values",
  "clamp 2nd value to closed interval between 1st and 3rd",
  "if 1st value is non-zero, then 2nd value, else 3rd value",
  "linearly interpolate between 2nd value (1st = 0.0) and 3rd (1st = 1.0)",
  "if 1st value exists, the 2nd value, else the 3rd",
  "2nd value is inside OPEN interval range between 1st and 3rd",
  "2nd value is inside CLOSED interval range between 1st and 3rd",
};

#define ntAdd nrrdTernaryOpAdd
#define ntMul nrrdTernaryOpMultiply

char
_nrrdTernaryOpStrEqv[][AIR_STRLEN_SMALL] = {
  "+", "plus", "add",
  "x", "*", "times", "multiply", "product",
  "min",
  "max",
  "clamp",
  "ifelse", "if",
  "lerp",
  "exists",
  "in_op",
  "in_cl",
  ""
};

int
_nrrdTernaryOpValEqv[] = {
  ntAdd, ntAdd, ntAdd,
  ntMul, ntMul, ntMul, ntMul, ntMul, 
  nrrdTernaryOpMin,
  nrrdTernaryOpMax,
  nrrdTernaryOpClamp,
  nrrdTernaryOpIfElse, nrrdTernaryOpIfElse,
  nrrdTernaryOpLerp,
  nrrdTernaryOpExists,
  nrrdTernaryOpInOpen,
  nrrdTernaryOpInClosed,
};

airEnum
_nrrdTernaryOp_enum = {
  "ternary op",
  NRRD_TERNARY_OP_MAX,
  _nrrdTernaryOpStr, NULL,
  _nrrdTernaryOpDesc,
  _nrrdTernaryOpStrEqv, _nrrdTernaryOpValEqv, 
  AIR_FALSE
};
airEnum *
nrrdTernaryOp = &_nrrdTernaryOp_enum;

/* ---- END non-NrrdIO */
