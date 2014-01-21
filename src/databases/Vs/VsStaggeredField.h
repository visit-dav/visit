/**
 * @file VsStaggeredField.h
 *      
 *  @class VsStaggeredField
 *  @brief Represents staggered fields in memory
 *  
 *  Created on: Apr 23, 2013
 *      Author: pletzer
 */

#ifndef VS_STAGGERED_FIELD_H_
#define VS_STAGGERED_FIELD_H_

#include <string>
#include "VsObject.h"
#include <vector>
#include <valarray>

template <class TYPE>
class VsStaggeredField {

public:

  /**
   * Constructor
   * @param subRes sub-grid resolution, number of sub-cells is 2^subRes in each direction
   */
  VsStaggeredField(size_t subRes);

  /** 
   * Destructor
   */
  virtual ~VsStaggeredField();

  /**
   * Set the data pointer
   * @param dataPtr data pointer
   * @param numDims number of dimensions (incl. components)
   * @param dims dimensions along each axis (incl. component axis)
   * @param indexOrder index order (e.g. VsSchema::compMajorFKey)
   * @param centering (e.g. VsSchema::faceCenteringKey)
   */
   void setDataPtr(const TYPE* dataPtr, size_t numDims, const int dims[], 
                   const std::string& indexOrder, const std::string& centering);

   /**
    * Get the interpolated field dimensions
    * @return dimensions along each axis (incl. component axis)
    */
   std::vector<int> getNewNodalDataDims() const;

   /**
    * Get the new field data values
    * @param pointer to the new data, caller is responsible to allocating and freeing
    */
   void getNewNodalData(TYPE* dataPtr) const;

  
private:

   std::valarray<int> getOriCellIndexSet(size_t bigIndx) const;
   std::valarray<int> getOriDisplacements(size_t subBigIndx) const;
   size_t getOriBigIndex(const std::valarray<int>& indx) const;
   std::valarray<TYPE> getOriFieldVals(size_t bigInx) const;
   std::valarray<int> getSubCellIndexSet(size_t subBigIndex) const;
   size_t getNewBigIndex(const std::valarray<int>& newCellInds) const;

   /**
    * Fill interpolated values 
    * @param newBigInd big (flat) index of the new data array
    * @param sigmaVals displacements (array of 1s and 0s) from the original cells
    * @param neighVals field values corresponding to the sigmaVals displacements
    * @param dataPtr pointer to the new data, some values will be filled in
    */
   void fillNewVals(size_t newBigIndx, 
                    const std::valarray<TYPE>& xiVals,
                    const std::vector< std::valarray<TYPE> >& sigmaVals,
                    const std::vector< std::valarray<TYPE> >& neighVals, 
                    TYPE* dataPtr) const; 


   void setNodalInterpFieldVals(size_t newBigIndx, 
                                const std::valarray<TYPE>& xiVals, 
                                const std::vector< std::valarray<TYPE> >& sigmaVals,
                                const std::vector< std::valarray<TYPE> >& neighVals, 
                                TYPE* dataPtr) const;
   void setEdgeInterpFieldVals(size_t newBigIndx, 
                               const std::valarray<TYPE>& xiVals, 
                               const std::vector< std::valarray<TYPE> >& sigmaVals,
                               const std::vector< std::valarray<TYPE> >& neighVals, 
                               TYPE* dataPtr) const;
   void setFaceInterpFieldVals(size_t newBigIndx, 
                               const std::valarray<TYPE>& xiVals, 
                               const std::vector< std::valarray<TYPE> >& sigmaVals,
                               const std::vector< std::valarray<TYPE> >& neighVals, 
                               TYPE* dataPtr) const;
   void setZonalInterpFieldVals(size_t newBigIndx, 
                                const std::valarray<TYPE>& xiVals, 
                                const std::vector< std::valarray<TYPE> >& sigmaVals,
                                const std::vector< std::valarray<TYPE> >& neighVals, 
                                TYPE* dataPtr) const;
   

   /** number of sub-cells is 2^subRes in each topological direction */
  size_t subRes;

  /** pointer to the original data, assumed to have dimension nodal data times number of components */
  TYPE* oriDataPtr;

  /** total number of original grid cells */
  size_t totNumOriCells;
  
  /** total number of new grid cells */
  size_t totNumNewCells;

  /** total number of data values (number of nodes times number of components) */
  size_t totNumOriVals;

  /** total number of new data values (number of nodes times number of components) */
  size_t totNumNewVals;

  /** 2^subRes */
  size_t twoPowSubRes;

  /** number neighbors forming a cell (2^d) */
  size_t numNeighbors;

  /** number of topological dimensions */
  size_t numTopoDims;
  
  /** component index */
  int indexComp;

  /** index order, see VsSchema */
  std::string indexOrder;

  /** type of field centering (nodal, edge, ...) */
  std::string centering;
  
  /** number of cells along each direction for the original data */
  std::valarray<int> oriCellDim;

  /** number of cells along each direction for the new data */
  std::valarray<int> newCellDims;

  /** array to map big index to index set for original data */
  std::valarray<int> oriCellDimProd;

  /** array to map big index to index set for new data */
  std::valarray<int> newCellDimProd;

};

#endif /* VS_STAGGERED_FIELD_H_ */
