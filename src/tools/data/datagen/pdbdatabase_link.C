// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// This is a hack that we need in order to link against certain PDB libraries
// that have not been built with g++. We add it to the link line after the
// PDB libraries have been added so we can finish linking against those
// PDB libraries.
extern "C" void ieee_handler(int) { };
