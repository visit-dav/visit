
#ifndef _CUBEREADER_H
#define _CUBEREADER_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iterator>

using namespace std;

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
  
  void GetGridValues(float* vals);
  
  void GetAtomLocations(vector<float> &locations);
  
  void GetAtomTypes(float* types);

  bool isGridSheared() const;
  
  bool isFileValid() const;
  
  string getErrorString() const;

  // default units are in angstorm
  void SetAtomUnitsToBohr();
  void SetGridUnitsToBohr();

  int  GetNumOrbitals();
  int  GetOrbitalNumber(int i);
  void GetOrbitalValues(float* ptr, const char* varname);

 private:
  //The name of the input file
  string filename;
  //ifstream of the file
  ifstream file;

  long int data_pos;
  int x_size, y_size, z_size;

  float x_origin, y_origin, z_origin;
  
  float dx[3], dy[3], dz[3];
  
  float atom_units;
  float grid_units;
  
  bool isSheared;
  //String indicating whether an error occured while parsing the header
  string errorString;
  
  int natoms;
  vector<float> atom_locations;
  vector<int>   atom_types;

  vector<int> orbitals;

};

#endif
