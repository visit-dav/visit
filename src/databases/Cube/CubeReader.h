// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// *************************************************************************
//   CubeReader.h
// *************************************************************************

#ifndef _CUBEREADER_H
#define _CUBEREADER_H

#include <vector>
#include <string>
#include <fstream>


// ****************************************************************************
//  Class: CubeReader
//
//  Purpose:
//      Reads in Cube files as a plugin to VisIt.
//
//  Modifications:
//    Kathleen Biagas, Mon Jan 8, 2024
//    Added 'orbital_offset'.  Removed unused data_pos;
//
// ****************************************************************************

class CubeReader
{
 public:
  CubeReader(const char* filename);
  ~CubeReader();

  void readMetaData(bool keepFileOpen=false);

  int GetNTimesteps();

  int GetNAtoms();

  void GetGridSize(int &xs, int &ys, int &zs);

  void GetGridLocations(float* x, float* y, float* z);

  void GetShearedGridLocations(float* x, float* y, float* z);
  void GetShearedGridLocations2(float* xyz);

  void GetGridValues(float* vals);
  void GetGridValues2(float* vals);

  void GetAtomLocations(std::vector<float> &locations);

  void GetAtomTypes(float* types);

  bool isGridSheared() const;

  bool isFileValid() const;

  std::string getErrorString() const;

  // default units are in angstorm
  void SetAtomUnitsToBohr();
  void SetGridUnitsToBohr();

  int  GetNumOrbitals();
  int  GetOrbitalNumber(int i);
  void GetOrbitalValues(float* ptr, const char* varname);

  void GetUnitCell(float *UCO, float *UCV, bool extend); // unit cell origin and unit cell vector
 private:
  //The name of the input file
  std::string filename;
  //ifstream of the file
  std::ifstream file;

  int x_size, y_size, z_size;

  float x_origin, y_origin, z_origin;

  float dx[3], dy[3], dz[3];

  float atom_units;
  float grid_units;

  bool isSheared;
  //String indicating whether an error occured while parsing the header
  std::string errorString;

  int natoms;
  std::vector<float> atom_locations;
  std::vector<int>   atom_types;

  std::vector<int> orbitals;
  int orbital_offset;

};

#endif
