/*
 * VsStaggeredField.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: pletzer
 */

#include "VsStaggeredField.h"
#include "VsLog.h"
#include "VsSchema.h"
#include <valarray>

template<class TYPE>
VsStaggeredField<TYPE>::VsStaggeredField(size_t subRes) {

  this->subRes = subRes;
  this->oriDataPtr = 0;
  this->totNumOriCells = 0;
  this->totNumNewCells = 0;
  this->totNumOriVals = 0;
  this->totNumNewVals = 0;
  this->twoPowSubRes = 0;
  this->numNeighbors = 0;
  this->numTopoDims = 0;
  this->indexComp = -1;
  this->indexOrder = VsSchema::compMajorCKey;
  this->centering = VsSchema::nodalCenteringKey;

  this->oriCellDims.clear();
  this->oriCellDimProd.clear();
  this->newCellDims.clear();
  this->newCellDimProd.clear();
}

template<class TYPE>
VsStaggeredField<TYPE>::~VsStaggeredField() {  
}

template<class TYPE>
void
VsStaggeredField<TYPE>::setDataPtr(const TYPE* dataPtr, 
                                   size_t numDims, 
                                   const int dims[], 
                                   const std::string& indexOrder, 
                                   const std::string& centering) {

  this->oriDataPtr = dataPtr;
  this->numTopoDims = numDims - 1; // assume dims also contains the number of components
  this->indexOrder = indexOrder;
  this->centering = centering;

  this->indexComp = 0; 
  if (indexOrder == VsSchema::compMinorCKey || 
      indexOrder == VsSchema::compMinorFKey) {
    this->indexComp = numDims - 1;
  }
  
  this->twoPowSubRes = 1;
  for (size_t i = 0; i < this->subRes; ++i) {
    this->twoPowSubRes *= 2;
  }

  this->numNeighbors = 1;
  for (size_t i = 0; i < this->numTopoDims; ++i) {
    this->numNeighbors *= 2;
  }

  this->oriCellDims.resize(this->numTopoDims);
  size_t offset = 0;
  for (size_t i = 0; i < numDims; ++i) {
    if (i != this->indexComp) {
      this->oriCellDims[i] = dims[i + offset];
    }
    else {
      offset = 1;
    }
  }

  this->newCellDims.resize(this->numTopoDims);
  for (size_t i = 0; i < this->numTopoDims; ++i) {
    this->newCellDims[i] = this->oriCellDims[i] * this->twoPowSubRes;
  }
  
  this->oriCellDimProd.resize(this->numTopoDims);
  this->oriCellDimProd[this->numTopoDims - 1] = 1;
  this->newCellDimProd.resize(this->numTopoDims);
  this->newCellDimProd[this->numTopoDims - 1] = 1;
  for (int i = this->numTopoDims - 2; i >= 0; --i) {
    this->oriCellDimProd[i] = this->oriCellDimProd[i + 1] * this->oriCellDims[i + 1];
    this->newCellDimProd[i] = this->newCellDimProd[i + 1] * this->newCellDims[i + 1];
  }

  this->totNumOriCells = 1;
  this->totNumNewCells = 1;
  this->totNumOriVals = 1;
  this->totNumNewVals = 1;
  for (size_t i = 0; i < this->numTopoDims; ++i) {
    this->totNumOriCells *= this->oriCellDims[i];
    this->totNumNewCells *= this->newCellDims[i];
    this->totNumOriVals *= (this->oriCellDims[i] + 1);
    this->totNumNewVals *= (this->newCellDims[i] + 1);
  }
  
}

template <class TYPE>
std::vector<int>
VsStaggeredField<TYPE>::getNewNodalDataDims() const {
  std::vector<int> dims(this->numTopoDims + 1);
  int offset = 0;
  for (int i = 0; i < this->numTopoDims + 1; ++i) {
    if (i != this->indexComp) {
      dims[i] = this->newCellDims[i - offset] + 1; // data always have dimension of nodal data
    }
    else {
      offset = 1;
    }
  }
  return dims;
}

template <class TYPE>
void
VsStaggeredField<TYPE>::getNewNodalData(TYPE* dataPtr) const 
{
  std::vector< std::valarray<TYPE> > neighVals(this->numNeighbors);
  std::vector< std::valarray<int> > sigmaVals(this->numNeighbors);
  for (size_t bigIndx = 0; bigIndx < this->totNumOriCells; ++bigIndx) {
    std::valarray<int> oriCellInds = this->getOriCellIndexSet(bigIndx);
    for (size_t n = 0; n < this->numNeighbors; ++n) {
      sigmaVals[n] = this->getOriDisplacements(n);
      size_t bigIndxNeigh = this->getOriBigIndex(oriCellInds + sigmaVals[n]);
      neighVals[n] = this->getOriFieldVals(bigIndxNeigh);
    }

    // iterate over subgrid
    for (size_t subBigIndx = 0; subBigIndx < this->numNeighbors; ++subBigIndx) {
      std::valarray<int> subCellInds = this->getSubCellIndexSet(subBigIndx);
      std::valarray<int> newCellInds = oriCellInds*this->numNeighbors + subCellInds;
      size_t newBigIndx = this->getNewBigIndex(newCellInds);
      std::valarray<TYPE> xi = std::valarray<TYPE>(subCellInds) / this->twoPowSubRes;
      this->setNewFieldVals(newBigIndx, xi, sigmaVals, neighVals, dataPtr);
    }
  }
}

template <class TYPE>
std::valarray<int>
VsStaggeredField<TYPE>::getOriCellIndexSet(size_t bigIndex) const {
  return (bigIndex / this->oriCellDimProd) % this->oriCellDims;
}

template <class TYPE>
std::valarray<int>
VsStaggeredField<TYPE>::getOriDisplacements(size_t subBigIndex) const {
  return (subBigIndex / this->twoPowSubRes) % 2;
}

template <class TYPE>
size_t
VsStaggeredField<TYPE>::getOriBigIndex(const std::valarray<int>& indx) const {
  size_t res = 0;
  for (size_t i = 0; i < this->numTopoDims; ++i) {
    res += indx[i] * this->dimCellProd[i];
  }
  return res;
}

template <class TYPE>
std::valarray<TYPE>
VsStaggeredField<TYPE>::getOriFieldVals(size_t bigInx) const {
  std::valarray<TYPE> res(this->numComps);
  if (this->compIndex == 0) {
    for (size_t j = 0; j < this->numComps; ++j) {
      res[j] = this->oriData[j*this->totNumOriVals + bigInx];
    }
  }
  else {
    for (size_t j = 0; j < this->numComps; ++j) {
      res[j] = this->oriData[bigInx*this->numComps + j];
    }
  }
  return res;
}

template <class TYPE>
std::valarray<int>
VsStaggeredField<TYPE>::getSubCellIndexSet(size_t subBigIndex) const {
  return (subBigIndex / this->subCellDimProd) % this->twoPowSubRes;
}

template <class TYPE>
size_t
VsStaggeredField<TYPE>::getNewBigIndex(const std::valarray<int>& newCellInds) const {
  size_t res = 0;
  for (size_t i = 0; i < this->numTopoDims; ++i) {
    res += newCellInds[i] * this->newCellDimProd[i];
  }
  return res;
}

template <class TYPE>
void
VsStaggeredField<TYPE>::fillNewVals(size_t newBigIndx, 
                                    const std::valarray<TYPE>& xiVals,
                                    const std::vector< std::valarray<TYPE> >& sigmaVals,
                                    const std::vector< std::valarray<TYPE> >& neighVals, 
                                    TYPE* dataPtr)  const {
  switch (this->centering) {
  case 'n':
    this->setNodalInterpFieldVals(newBigIndx, xiVals, sigmaVals, neighVals, dataPtr);
    break;
  case 'e':
    this->setEdgeInterpFieldVals(newBigIndx, xiVals, sigmaVals, neighVals, dataPtr);
    break;
  case 'f':
    this->setFaceInterpFieldVals(newBigIndx, xiVals, sigmaVals, neighVals, dataPtr);
    break;
  default:
    this->setZonalInterpFieldVals(newBigIndx, xiVals, sigmaVals, neighVals, dataPtr);
  }
}

template <class TYPE>
void
VsStaggeredField<TYPE>::setNodalInterpFieldVals(size_t newBigIndx, 
                                                const std::valarray<TYPE>& xiVals, 
                                                const std::vector< std::valarray<TYPE> >& sigmaVals,
                                                const std::vector< std::valarray<TYPE> >& neighVals, 
                                                TYPE* dataPtr) const {
  // TO IMPLEMENT
}

template <class TYPE>
void
VsStaggeredField<TYPE>::setEdgeInterpFieldVals(size_t newBigIndx, 
                                               const std::valarray<TYPE>& xiVals, 
                                               const std::vector< std::valarray<TYPE> >& sigmaVals,
                                               const std::vector< std::valarray<TYPE> >& neighVals, 
                                               TYPE* dataPtr) const {
  // TO IMPLEMENT
}

template <class TYPE>
void
VsStaggeredField<TYPE>::setFaceInterpFieldVals(size_t newBigIndx, 
                                               const std::valarray<TYPE>& xiVals, 
                                               const std::vector< std::valarray<TYPE> >& sigmaVals,
                                               const std::vector< std::valarray<TYPE> >& neighVals, 
                                               TYPE* dataPtr) const {
  // TO IMPLEMENT
}

template <class TYPE>
void
VsStaggeredField<TYPE>::setZonalInterpFieldVals(size_t newBigIndx, 
                                                const std::valarray<TYPE>& xiVals, 
                                                const std::vector< std::valarray<TYPE> >& sigmaVals,
                                                const std::vector< std::valarray<TYPE> >& neighVals, 
                                                TYPE* dataPtr) const {
  // TO IMPLEMENT
}

// explicit instantiations
//template class VsStaggeringField<double>;
//template class VsStaggeringField<float>;
//template class VsStaggeringField<int>;

