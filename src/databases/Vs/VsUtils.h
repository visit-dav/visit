/**
 * @file VsUtils.h
 *
 * @class VsUtils
 * @brief Some useful routines for HDF5 handling.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */

#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1, 8, 1)

#ifndef VS_UTILS
#define VS_UTILS

#include <string>
#include <vector>

// Get dimensions of a dataset or attribute
//void getDims(hid_t id, bool isDataset, std::vector<int>& dims);

// Create fully qualified name from an object name
// This means strip the leading "/" if it exists
std::string makeCanonicalName(std::string name);

// Create fully qualified name from object path and name
// i.e. path + "/" + name
// Then strip the leading "/" if it exists
std::string makeCanonicalName(std::string path, std::string name);

// Compare two object names to a target name
// Return the name that is "closest" to the target
// If the two object names are identical, returns the first
//std::string getClosestName(std::string name1, std::string name2, std::string target);

bool isDoubleType(hid_t dataType);
bool isFloatType(hid_t dataType);
bool isIntegerType(hid_t dataType);
void printType(hid_t dataType);

// Break a string into pieces
void tokenize(std::string text, char separator, std::vector<std::string>& tokens);


void adjustSize_hsize_t(hsize_t* dims, unsigned int rank, std::vector<int> stride, int before, int after);
void adjustSize_vector(std::vector<int>* dims, int rank, std::vector<int> stride, int before, int after);
#endif
#endif

