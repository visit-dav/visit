// Copyright (c) 2011, Allinea
// All rights reserved.
//
// This file has been contributed to the VisIt project, which is
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DDTSIM_CONSTANTS_H
#define DDTSIM_CONSTANTS_H

//! The maximum number of characters in the name of a mesh or variable. Last character must be '\0'.
#define MAX_NAME_LENGTH 128
//! The maximum number of characers in a file path
#define MAX_PATH_LENGTH 512
//! The maximum number of characters in the absolute path to the libsim .sim file
#define MAX_SIMFILE_NAME_LENGTH 1024

enum VisItMeshType { DDTSIM_MESH_RECTILINEAR, DDTSIM_MESH_POINT };
enum VisItVarType { DDTSIM_VARTYPE_SCALAR };
enum VisItCenteringType { DDTSIM_CENTERING_NODE, DDTSIM_CENTERING_ZONE };
enum VisItDataType { DDTSIM_TYPE_FLOAT, DDTSIM_TYPE_DOUBLE, DDTSIM_TYPE_CHAR, DDTSIM_TYPE_INT };

#endif // DDTSIM_CONSTANTS_H
