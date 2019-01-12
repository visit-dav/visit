/**
 * @file VsUtils.h
 *
 * @class VsUtils
 * @brief Some useful routines for HDF5 handling.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */

#include <hdf5.h>

#ifndef VS_UTILS_H
#define VS_UTILS_H

#include <string>
#include <vector>

// Get dimensions of a dataset or attribute
//void getDims(hid_t id, bool isDataset, std::vector<int>& dims);

/**
 * Create fully qualified name from an object name
 * @param name object name
 * @return object name without leading "/"
 */
std::string makeCanonicalName(const std::string& name);

/** 
 * Create fully qualified name from object path and name,
 * @param path
 * @param name
 * @return path + "/" + name (without leading "/")
 */
std::string makeCanonicalName(const std::string& path, 
                              const std::string& name);

// Compare two object names to a target name
// @return the name that is "closest" to the target
// @note If the two object names are identical, will return the first
//std::string getClosestName(const std::string& name1, const std::string& name2, const std::string& target);

/**
 * Check data type
 * @param dataType HDF5 data type
 * @return true if type is double
 */
bool isDoubleType(hid_t dataType);

/**
 * Check data type
 * @param dataType HDF5 data type
 * @return true if type is float
 */
bool isFloatType(hid_t dataType);

/**
 * Check data type
 * @param dataType HDF5 data type
 * @return true if type is int
 */
bool isIntType(hid_t dataType);

/**
 * Check data type
 * @param dataType HDF5 data type
 * @return true if type is short
 */
bool isShortType(hid_t dataType);

/**
 * Check data type
 * @param dataType HDF5 data type
 * @return true if type is char
 */
bool isCharType(hid_t dataType);

/**
 * Check data type
 * @param dataType HDF5 data type
 * @return true if type is unsigned char
 */
bool isUnsignedCharType(hid_t dataType);

void printType(hid_t dataType);

// Break a string into pieces
void tokenize(const std::string& text, 
              char separator, std::vector<std::string>& tokens);


void adjustSize_hsize_t(hsize_t* dims, unsigned int rank, 
                        const std::vector<int>& stride, 
                        int before, int after);
void adjustSize_vector(std::vector<int>* dims, int rank, 
                       const std::vector<int>& stride, 
                       int before, int after);

#endif // VS_UTILS_H

