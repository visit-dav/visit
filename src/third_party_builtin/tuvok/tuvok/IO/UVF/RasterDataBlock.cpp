#include "RasterDataBlock.h"

#include "UVF.h"
#include <math.h>
#include <Basics/MathTools.h>
#include <Basics/SysTools.h>
#include <sstream>
#include <iostream>

#include "MaxMinDataBlock.h"

using namespace std;
using namespace UVFTables;

#define BLOCK_COPY_SIZE     (UINT64(512*1024*1024))

//*************** Raster Data Block **********************

RasterDataBlock::RasterDataBlock() : 
	DataBlock(),
	ulElementDimension(0),
	ulOffsetToDataBlock(0),

	m_pTempFile(NULL)
{
	ulBlockSemantics = BS_REG_NDIM_GRID;
}

RasterDataBlock::RasterDataBlock(const RasterDataBlock &other) : 
	DataBlock(other), 
	ulDomainSemantics(other.ulDomainSemantics),
	dDomainTransformation(other.dDomainTransformation),
	ulDomainSize(other.ulDomainSize),
	ulBrickSize(other.ulBrickSize),
	ulBrickOverlap(other.ulBrickOverlap),
	ulLODDecFactor(other.ulLODDecFactor),
	ulLODGroups(other.ulLODGroups),
	ulLODLevelCount(other.ulLODLevelCount),
	ulElementDimension(other.ulElementDimension),
	ulElementDimensionSize(other.ulElementDimensionSize),
	ulElementSemantic(other.ulElementSemantic),
	ulElementBitSize(other.ulElementBitSize),
	ulElementMantissa(other.ulElementMantissa),
	bSignedElement(other.bSignedElement),
	ulOffsetToDataBlock(other.ulOffsetToDataBlock),

	m_pTempFile(NULL)
{
  if (other.m_pTempFile != NULL) m_pTempFile = new LargeRAWFile(*other.m_pTempFile);
}


RasterDataBlock& RasterDataBlock::operator=(const RasterDataBlock& other)  { 
	strBlockID = other.strBlockID;
	ulBlockSemantics = other.ulBlockSemantics;
	ulCompressionScheme = other.ulCompressionScheme;
	ulOffsetToNextDataBlock = other.ulOffsetToNextDataBlock;

	ulDomainSemantics = other.ulDomainSemantics;
	dDomainTransformation = other.dDomainTransformation;
	ulDomainSize = other.ulDomainSize;
	ulBrickSize = other.ulBrickSize;
	ulBrickOverlap = other.ulBrickOverlap;
	ulLODDecFactor = other.ulLODDecFactor;
	ulLODGroups = other.ulLODGroups;
	ulLODLevelCount = other.ulLODLevelCount;
	ulElementDimension = other.ulElementDimension;
	ulElementDimensionSize = other.ulElementDimensionSize;
	ulElementSemantic = other.ulElementSemantic;
	ulElementBitSize = other.ulElementBitSize;
	ulElementMantissa = other.ulElementMantissa;
	bSignedElement = other.bSignedElement;
	ulOffsetToDataBlock = other.ulOffsetToDataBlock;

	m_pTempFile = NULL;

	return *this;
}

RasterDataBlock::~RasterDataBlock() {
	CleanupTemp();
}

RasterDataBlock::RasterDataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) : 
	DataBlock(),
	ulElementDimension(0),
	ulOffsetToDataBlock(0),

	m_pTempFile(NULL)
{
	GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
}

DataBlock* RasterDataBlock::Clone() {
	return new RasterDataBlock(*this);
}

UINT64 RasterDataBlock::GetOffsetToNextBlock() const {
	return DataBlock::GetOffsetToNextBlock() + ComputeHeaderSize() + ComputeDataSize();
}

UINT64 RasterDataBlock::ComputeHeaderSize() const {
	UINT64 ulDomainDimension = ulDomainSemantics.size();
	UINT64 ulOverallElementSize=0;

	for (size_t i = 0;i<ulElementDimensionSize.size();i++) ulOverallElementSize += ulElementDimensionSize[i];

	return 1						* sizeof(UINT64) +		// ulDomainDimension
		   ulDomainDimension		* sizeof(UINT64) +		// ulDomainSemantics
		   (ulDomainDimension+1)*(ulDomainDimension+1)	* sizeof(UINT64) +		// dDomainTransformation
		   ulDomainDimension		* sizeof(UINT64) +		// ulDomainSize
		   ulDomainDimension		* sizeof(UINT64) +		// ulBrickSize
		   ulDomainDimension		* sizeof(UINT64) +		// ulBrickOverlap
		   ulDomainDimension		* sizeof(UINT64) +		// ulLODDecFactor
		   ulDomainDimension		* sizeof(UINT64) +		// ulLODGroups
		   ulLODLevelCount.size()	* sizeof(UINT64) +		// ulLODLevelCount

		   1						* sizeof(UINT64) +		// ulElementDimension
		   ulElementDimension		* sizeof(UINT64) +		// ulElementDimensionSize

		   ulOverallElementSize		* sizeof(UINT64) +		// ulElementSemantic
		   ulOverallElementSize		* sizeof(UINT64) +		// ulElementBitSize
		   ulOverallElementSize		* sizeof(UINT64) +		// ulElementMantissa
		   ulOverallElementSize		* sizeof(bool) +		// bSignedElement

		   1						* sizeof(UINT64);		// ulOffsetToDataBlock

}

UINT64 RasterDataBlock::GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
  UINT64 iStart = iOffset + DataBlock::GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
	pStreamFile->SeekPos(iStart);

	UINT64 ulDomainDimension;
	pStreamFile->ReadData(ulDomainDimension, bIsBigEndian);

	if (ulDomainDimension > 0) {
		vector<UINT64> uintVect;
		pStreamFile->ReadData(uintVect, ulDomainDimension, bIsBigEndian);
		ulDomainSemantics.resize(size_t(ulDomainDimension));
		for (size_t i = 0;i<uintVect.size();i++) ulDomainSemantics[i] = (DomainSemanticTable)uintVect[i];

		pStreamFile->ReadData(dDomainTransformation, (ulDomainDimension+1)*(ulDomainDimension+1), bIsBigEndian);
		pStreamFile->ReadData(ulDomainSize, ulDomainDimension, bIsBigEndian);
		pStreamFile->ReadData(ulBrickSize, ulDomainDimension, bIsBigEndian);
		pStreamFile->ReadData(ulBrickOverlap, ulDomainDimension, bIsBigEndian);
		pStreamFile->ReadData(ulLODDecFactor, ulDomainDimension, bIsBigEndian);
		pStreamFile->ReadData(ulLODGroups, ulDomainDimension, bIsBigEndian);

		RecompLODIndexCount();
	}

	UINT64 ulLODIndexCount = RecompLODIndexCount();
	pStreamFile->ReadData(ulLODLevelCount, ulLODIndexCount, bIsBigEndian);
	pStreamFile->ReadData(ulElementDimension, bIsBigEndian);
	pStreamFile->ReadData(ulElementDimensionSize, ulElementDimension, bIsBigEndian);

	ulElementSemantic.resize(size_t(ulElementDimension));
	ulElementBitSize.resize(size_t(ulElementDimension));
	ulElementMantissa.resize(size_t(ulElementDimension));
	bSignedElement.resize(size_t(ulElementDimension));
	for (size_t i = 0;i<size_t(ulElementDimension);i++) {

		vector<UINT64> uintVect;
		pStreamFile->ReadData(uintVect, ulElementDimensionSize[i], bIsBigEndian);
		ulElementSemantic[i].resize(size_t(ulElementDimensionSize[i]));
		for (size_t j = 0;j<uintVect.size();j++) ulElementSemantic[i][j] = (ElementSemanticTable)uintVect[j];

		pStreamFile->ReadData(ulElementBitSize[i], ulElementDimensionSize[i], bIsBigEndian);
		pStreamFile->ReadData(ulElementMantissa[i], ulElementDimensionSize[i], bIsBigEndian);

		// reading bools failed on windows so we are reading chars
		vector<char> charVect;
		pStreamFile->ReadData(charVect, ulElementDimensionSize[i], bIsBigEndian);
		bSignedElement[i].resize(size_t(ulElementDimensionSize[i]));
		for (size_t j = 0;j<charVect.size();j++)  bSignedElement[i][j] = charVect[j] != 0;
	}

	pStreamFile->ReadData(ulOffsetToDataBlock, bIsBigEndian);

	ComputeDataSizeAndOffsetTables();  // build the offset table

	return pStreamFile->GetPos() - iOffset;
}

UINT64 RasterDataBlock::CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock) {
  UINT64 iStart = iOffset + DataBlock::CopyToFile(pStreamFile, iOffset, bIsBigEndian, bIsLastBlock);
	pStreamFile->SeekPos(iStart);

	// write header
	UINT64 ulDomainDimension = ulDomainSemantics.size();
	pStreamFile->WriteData(ulDomainDimension, bIsBigEndian);

	if (ulDomainDimension > 0) {
		vector<UINT64> uintVect; uintVect.resize(size_t(ulDomainDimension));
		for (size_t i = 0;i<uintVect.size();i++) uintVect[i] = (UINT64)ulDomainSemantics[i];
		pStreamFile->WriteData(uintVect, bIsBigEndian);

		pStreamFile->WriteData(dDomainTransformation, bIsBigEndian);
		pStreamFile->WriteData(ulDomainSize, bIsBigEndian);
		pStreamFile->WriteData(ulBrickSize, bIsBigEndian);
		pStreamFile->WriteData(ulBrickOverlap, bIsBigEndian);
		pStreamFile->WriteData(ulLODDecFactor, bIsBigEndian);
		pStreamFile->WriteData(ulLODGroups, bIsBigEndian);
	}

	pStreamFile->WriteData(ulLODLevelCount, bIsBigEndian);
	pStreamFile->WriteData(ulElementDimension, bIsBigEndian);
	pStreamFile->WriteData(ulElementDimensionSize, bIsBigEndian);

	for (size_t i = 0;i<size_t(ulElementDimension);i++) {

		vector<UINT64> uintVect; uintVect.resize(size_t(ulElementDimensionSize[i]));
		for (size_t j = 0;j<uintVect.size();j++)  uintVect[j] = (UINT64)ulElementSemantic[i][j];
		pStreamFile->WriteData(uintVect, bIsBigEndian);				

		pStreamFile->WriteData(ulElementBitSize[i], bIsBigEndian);
		pStreamFile->WriteData(ulElementMantissa[i], bIsBigEndian);

		// writing bools failed on windows so we are writing chars
		vector<char> charVect; charVect.resize(size_t(ulElementDimensionSize[i]));
		for (size_t j = 0;j<charVect.size();j++)  charVect[j] = bSignedElement[i][j] ? 1 : 0;
		pStreamFile->WriteData(charVect, bIsBigEndian);				
	}

	pStreamFile->WriteData(ulOffsetToDataBlock, bIsBigEndian);

	UINT64 iDataSize = ComputeDataSize();

  m_pTempFile->SeekStart();
  pStreamFile->SeekPos( pStreamFile->GetPos() + ulOffsetToDataBlock);

  unsigned char* pData = new unsigned char[size_t(min(iDataSize, BLOCK_COPY_SIZE))];
	for (UINT64 i = 0;i<iDataSize;i+=BLOCK_COPY_SIZE) {
		UINT64 iCopySize = min(BLOCK_COPY_SIZE, iDataSize-i);

    m_pTempFile->ReadRAW(pData, iCopySize);
    pStreamFile->WriteRAW(pData, iCopySize);
  }
  delete [] pData;

	return pStreamFile->GetPos() - iOffset;
}

/**
 * Dumps the input data into a temp file and calls FlatDataToBrickedLOD
 * \param vElements - the input vectors of vector
 * \param iIndex - counter used internally to controll the recursion, defaults to 0 and should not be set
 * \return - the cartesian product of the ordered elements in the input vectors as a vector of vectors 
 */
vector<vector<UINT64> > RasterDataBlock::GenerateCartesianProduct(const vector<vector<UINT64> >& vElements, UINT64 iIndex)  const {
	vector<vector<UINT64> > vResult;
	if (iIndex == vElements.size()-1) {
		for (size_t i = 0;i<vElements[vElements.size()-1].size();i++) {
			vector<UINT64> v;
			v.push_back(vElements[vElements.size()-1][i]);
			vResult.push_back(v);
		}
	} else {
		vector<vector<UINT64> > vTmpResult = GenerateCartesianProduct(vElements,iIndex+1);
		for (size_t j = 0;j<vTmpResult.size();j++) {
			for (size_t i = 0;i<vElements[size_t(iIndex)].size();i++) {
				vector<UINT64> v;
				v.push_back(vElements[size_t(iIndex)][i]);
				
				for (size_t k = 0;k<vTmpResult[j].size();k++) v.push_back(vTmpResult[j][k]);

				vResult.push_back(v);
			}
			
		}
	}

	return vResult;
}

/**
 * Computes a vector of vectors, where each vector holds a list of bricksizes in one dimension
 * \param vDomainSize - the size of the domain
 * \return - a vector of vectors, where each vector holds a list of bricksizes in one dimension
 */
vector<vector<UINT64> > RasterDataBlock::ComputeBricks(const vector<UINT64>& vDomainSize) const {
	vector<vector<UINT64> > vBrickLayout;

	for (size_t iDomainDimension = 0;iDomainDimension<vDomainSize.size();iDomainDimension++) {
		
		UINT64 iSize			   = vDomainSize[iDomainDimension];
		UINT64 iBrickSize		 = ulBrickSize[iDomainDimension];
		UINT64 iBrickOverlap = ulBrickOverlap[iDomainDimension];

		vector<UINT64> vBricks;

		if (iSize <= iBrickSize) {
			vBricks.push_back(iSize);
		} else {
			do {
				if (iSize+iBrickOverlap <= iBrickSize) {
					vBricks.push_back(iSize);
					break;
				} else {
					vBricks.push_back(iBrickSize);
					iSize = iSize+iBrickOverlap-iBrickSize;
				}
			} while (iSize > iBrickOverlap);
		}

		vBrickLayout.push_back(vBricks);
	}

	return vBrickLayout;
}

/**
 * Computes the size of a single elment in the data IN BITS
 * \return - the size of a single elment in the data IN BITS
 */
UINT64 RasterDataBlock::ComputeElementSize() const {
	// compute the size of a single data element
	UINT64 uiBitsPerElement = 0;
	for (size_t i = 0;i<ulElementDimension;i++)
		for (size_t j = 0;j<ulElementDimensionSize[i];j++)
			uiBitsPerElement += ulElementBitSize[i][j];
	return uiBitsPerElement;
}

UINT64 RasterDataBlock::ComputeLODLevelSizeAndOffsetTables(const vector<UINT64>& vReducedDomainSize, UINT64 iLOD) {
	UINT64 ulSize = 0;

	// compute the size of a single data element
	UINT64 uiBitsPerElement = ComputeElementSize();

	// compute brick layout
	vector<vector<UINT64> > vBricks = ComputeBricks(vReducedDomainSize);
	vector<vector<UINT64> > vBrickPermutation = GenerateCartesianProduct(vBricks);

	for (size_t i = 0;i<vBricks.size();i++)m_vBrickCount[size_t(iLOD)].push_back(vBricks[i].size());
	m_vBrickOffsets[size_t(iLOD)].push_back(0);
	m_vBrickSizes[size_t(iLOD)] = vBrickPermutation;

	ulSize = 0;
	for (size_t i = 0;i<vBrickPermutation.size();i++) {
		UINT64 ulBrickSize = vBrickPermutation[i][0];
		for (size_t j = 1;j<vBrickPermutation[i].size();j++) {
			ulBrickSize *= vBrickPermutation[i][j];
		}
		ulSize += ulBrickSize;

		if (i<vBrickPermutation.size()-1) m_vBrickOffsets[size_t(iLOD)].push_back(ulSize*uiBitsPerElement);
	}

	return  ulSize * uiBitsPerElement;
}


UINT64 RasterDataBlock::ComputeLODLevelSize(const vector<UINT64>& vReducedDomainSize) const {
	UINT64 ulSize = 0;

	// compute the size of a single data element
	UINT64 uiBitsPerElement = ComputeElementSize();

	// compute brick layout
	vector<vector<UINT64> > vBricks = ComputeBricks(vReducedDomainSize);
	vector<vector<UINT64> > vBrickPermutation = GenerateCartesianProduct(vBricks);

	ulSize = 0;
	for (size_t i = 0;i<vBrickPermutation.size();i++) {
		UINT64 ulBrickSize = vBrickPermutation[i][0];
		for (size_t j = 1;j<vBrickPermutation[i].size();j++) {
			ulBrickSize *= vBrickPermutation[i][j];
		}
		ulSize += ulBrickSize;
	}

	return  ulSize * uiBitsPerElement;
}

UINT64 RasterDataBlock::GetLODSize(vector<UINT64>& vLODIndices) const {

	UINT64 ulSize = 0;

	vector<UINT64> vReducedDomainSize;
	vReducedDomainSize.resize(ulDomainSemantics.size());

	// compute size of the domain
	for (size_t i=0;i<vReducedDomainSize.size();i++) {
		if(ulLODDecFactor[i] < 2) 
			vReducedDomainSize[i]  = ulDomainSize[i];
		else
      vReducedDomainSize[i]  = max<UINT64>(1,UINT64(floor(double(ulDomainSize[i]) / double(MathTools::Pow(ulLODDecFactor[i],vLODIndices[size_t(ulLODGroups[i])])))));
	}

	ulSize = ComputeLODLevelSize(vReducedDomainSize);

	return ulSize;
}

UINT64 RasterDataBlock::GetLODSizeAndOffsetTables(vector<UINT64>& vLODIndices, UINT64 iLOD) {

	UINT64 ulSize = 0;

	vector<UINT64> vReducedDomainSize;
	vReducedDomainSize.resize(ulDomainSemantics.size());

	// compute size of the domain
	for (size_t i=0;i<vReducedDomainSize.size();i++) {
		if(ulLODDecFactor[i] < 2) 
			vReducedDomainSize[i]  = ulDomainSize[i];
		else
      vReducedDomainSize[i]  = max<UINT64>(1,UINT64(floor(double(ulDomainSize[i]) / double(MathTools::Pow(ulLODDecFactor[i],vLODIndices[size_t(ulLODGroups[i])])))));
	}

	ulSize = ComputeLODLevelSizeAndOffsetTables(vReducedDomainSize, iLOD);

	return ulSize;
}

vector<vector<UINT64> > RasterDataBlock::CountToVectors(vector<UINT64> vCountVector) const {
	vector<vector<UINT64> > vResult;

	vResult.resize(vCountVector.size());
	for (size_t i=0;i<vCountVector.size();i++) {
		for (size_t j=0;j<vCountVector[i];j++) {
			vResult[i].push_back(j);
		}
	}

	return vResult;
}


UINT64 RasterDataBlock::ComputeDataSize(string* pstrProblem) const {
	if (!Verify(pstrProblem)) return UVF_INVALID;

	UINT64 iDataSize = 0;
	
	// iterate over all LOD-Group Combinations
	vector<vector<UINT64> > vLODCombis = GenerateCartesianProduct(CountToVectors(ulLODLevelCount));

	for (size_t i = 0;i<vLODCombis.size();i++) {
		UINT64 iLODLevelSize = GetLODSize(vLODCombis[i]);
		iDataSize += iLODLevelSize;
	}
	
	return iDataSize/8;
}

UINT64 RasterDataBlock::ComputeDataSizeAndOffsetTables() {
	if (!Verify()) return UVF_INVALID;

	UINT64 iDataSize = 0;
	
	// iterate over all LOD-Group Combinations
	vector<vector<UINT64> > vLODCombis = GenerateCartesianProduct(CountToVectors(ulLODLevelCount));

	m_vLODOffsets.resize(vLODCombis.size());
	m_vBrickCount.resize(vLODCombis.size());
	m_vBrickOffsets.resize(vLODCombis.size());
	m_vBrickSizes.resize(vLODCombis.size());
	m_vLODOffsets[0] = 0;

	for (size_t i = 0;i<vLODCombis.size();i++) {
		UINT64 iLODLevelSize = GetLODSizeAndOffsetTables(vLODCombis[i],i);
		iDataSize += iLODLevelSize;
		
		if (i<vLODCombis.size()-1) m_vLODOffsets[i+1] = m_vLODOffsets[i] + iLODLevelSize;
	}
	
	return iDataSize/8;
}

UINT64 RasterDataBlock::RecompLODIndexCount() const {
	UINT64 ulLODIndexCount = 1;
	for (size_t i = 0;i<ulLODGroups.size();i++) if (ulLODGroups[i] >= ulLODIndexCount) ulLODIndexCount = ulLODGroups[i]+1;		
	return ulLODIndexCount;
}


bool RasterDataBlock::Verify(string* pstrProblem) const {
	UINT64 ulDomainDimension = ulDomainSemantics.size();

	UINT64 ulLODIndexCount = RecompLODIndexCount();

	if ( dDomainTransformation.size() != (ulDomainDimension+1)*(ulDomainDimension+1) ||
		 ulDomainSize.size() != ulDomainDimension ||
		 ulBrickSize.size() != ulDomainDimension ||
		 ulBrickOverlap.size() != ulDomainDimension ||
		 ulLODDecFactor.size() != ulDomainDimension ||
		 ulLODGroups.size() != ulDomainDimension ||
		 ulLODDecFactor.size() != ulDomainDimension ||
		 ulLODLevelCount.size() != ulLODIndexCount ||
		 ulElementDimensionSize.size() != ulElementDimension) {
	
			if (pstrProblem != NULL) *pstrProblem = "RasterDataBlock::Verify ulDomainDimension mismatch";
			return false;
	}

	for (size_t i = 0;i<size_t(ulDomainDimension);i++) {
		if (ulBrickSize[i] <= ulBrickOverlap[i]) {
			if (pstrProblem != NULL) {
				stringstream s;
				s << "RasterDataBlock::Verify ulBrickSize[" << i << "] > ulBrickOverlap[" << i << "]";
				*pstrProblem = s.str();
			}
			return false;
		}
	}

	for (size_t i = 0;i<size_t(ulElementDimension);i++) {
		 
		if (ulElementSemantic[i].size() != ulElementDimensionSize[i] ||
			ulElementBitSize[i].size() != ulElementDimensionSize[i] ||
			ulElementMantissa[i].size() != ulElementDimensionSize[i] ||
			bSignedElement[i].size() != ulElementDimensionSize[i]) {
				if (pstrProblem != NULL) {
					stringstream s;
					s << "RasterDataBlock::Verify ulElementDimensionSize[" << i << "] mismatch";
					*pstrProblem = s.str();
				}
				return false;
		}
		 
	}

	return true;
}

bool RasterDataBlock::Verify(UINT64 iSizeofData, string* pstrProblem) const {
	if (pstrProblem != NULL && iSizeofData != UVF_INVALID) *pstrProblem = "RasterDataBlock::Verify iSizeofData != UVF_INVALID";

	return (iSizeofData != UVF_INVALID) && (ComputeDataSize(pstrProblem) == iSizeofData);  // remember: ComputeDataSize calls Verify() which does all the other checks
}


bool RasterDataBlock::SetBlockSemantic(BlockSemanticTable bs) {
	if (bs != BS_REG_NDIM_GRID &&	
		bs != BS_NDIM_TRANSFER_FUNC &&
		bs != BS_PREVIEW_IMAGE) return false;

	ulBlockSemantics = bs;
	return true;
}


// **************** CONVENIANCE FUNCTIONS *************************

void RasterDataBlock::SetScaleOnlyTransformation(const vector<double>& vScale) {
	UINT64 ulDomainDimension = ulDomainSemantics.size();
	
	dDomainTransformation.resize(size_t((ulDomainDimension+1)*(ulDomainDimension+1)));

	for (size_t y = 0;y < size_t(ulDomainDimension+1);y++) 
		for (size_t x = 0;x < size_t(ulDomainDimension+1);x++) 
			dDomainTransformation[x+y*size_t(ulDomainDimension+1)] = (x == y) ? (x < vScale.size() ? vScale[x] : 1.0) : 0.0;

}

void RasterDataBlock::SetIdentityTransformation() {
	UINT64 ulDomainDimension = ulDomainSemantics.size();
	
	dDomainTransformation.resize(size_t((ulDomainDimension+1)*(ulDomainDimension+1)));

	for (size_t y = 0;y < size_t(ulDomainDimension+1);y++) 
		for (size_t x = 0;x < size_t(ulDomainDimension+1);x++) 
			dDomainTransformation[x+y*size_t(ulDomainDimension+1)] = (x == y) ? 1.0 : 0.0;

}

void RasterDataBlock::SetTypeToScalar(UINT64 iBitWith, UINT64 iMantissa, bool bSigned, ElementSemanticTable semantic) {
	vector<ElementSemanticTable> vSemantic;
	vSemantic.push_back(semantic);
	SetTypeToVector(iBitWith, iMantissa, bSigned, vSemantic);
}

void RasterDataBlock::SetTypeToVector(UINT64 iBitWith, UINT64 iMantissa, bool bSigned, vector<ElementSemanticTable> semantic) {

	vector<UINT64> vecB;
	vector<UINT64> vecM;
	vector<bool> vecSi;

	for (UINT64 i = 0;i<semantic.size();i++) {
		vecB.push_back(iBitWith);
		vecM.push_back(iMantissa);
		vecSi.push_back(bSigned);
	}

	ulElementDimension = 1;
	
	ulElementDimensionSize.push_back(semantic.size());
	ulElementSemantic.push_back(semantic);
	ulElementMantissa.push_back(vecM);
	bSignedElement.push_back(vecSi);
	ulElementBitSize.push_back(vecB);
}

void RasterDataBlock::SetTypeToUByte(ElementSemanticTable semantic) {
	SetTypeToScalar(8,8,false,semantic);
}

void RasterDataBlock::SetTypeToUShort(ElementSemanticTable semantic) {
	SetTypeToScalar(16,16,false,semantic);
}

void RasterDataBlock::SetTypeToInt32(ElementSemanticTable semantic) {
	SetTypeToScalar(32,31,true,semantic);
}

void RasterDataBlock::SetTypeToInt64(ElementSemanticTable semantic) {
	SetTypeToScalar(64,63,true,semantic);
}

void RasterDataBlock::SetTypeToUInt32(ElementSemanticTable semantic) {
	SetTypeToScalar(32,32,false,semantic);
}

void RasterDataBlock::SetTypeToUInt64(ElementSemanticTable semantic) {
	SetTypeToScalar(64,64,false,semantic);
}

void RasterDataBlock::SetTypeToFloat(ElementSemanticTable semantic) {
	SetTypeToScalar(32,23,true,semantic);
}

void RasterDataBlock::SetTypeToDouble(ElementSemanticTable semantic) {
	SetTypeToScalar(64,52,true,semantic);
}


const std::vector<UINT64> RasterDataBlock::GetSmallestBrickIndex() const {
  std::vector<UINT64> vSmallestLOD = ulLODLevelCount;
  for (size_t i = 0;i<vSmallestLOD.size();i++) vSmallestLOD[i] -= 1; // convert "size" to "maxindex"      
  return vSmallestLOD;
}

const std::vector<UINT64>& RasterDataBlock::GetSmallestBrickSize() const {
  std::vector<UINT64> vSmallestLOD = GetSmallestBrickIndex();
  std::vector<UINT64> vFirstBrick(GetBrickCount(vSmallestLOD).size());
  for (size_t i = 0;i<vFirstBrick.size();i++) vFirstBrick[i] = 0; // get the size of the first brick
  return GetBrickSize(vSmallestLOD, vFirstBrick);
}

UINT64 RasterDataBlock::Serialize(const vector<UINT64>& vec, const vector<UINT64>& vSizes) const {
	UINT64 index = 0;
	UINT64 iPrefixProd = 1;
	for (size_t i = 0;i<vSizes.size();i++) {
		index += vec[i] * iPrefixProd;
		iPrefixProd *= vSizes[i];
	}

	return index;
}

const vector<UINT64>& RasterDataBlock::GetBrickCount(const vector<UINT64>& vLOD) const {
  return m_vBrickCount[size_t(Serialize(vLOD, ulLODLevelCount))];
}

const vector<UINT64>& RasterDataBlock::GetBrickSize(const vector<UINT64>& vLOD, const vector<UINT64>& vBrick) const {
  UINT64 iLODIndex = Serialize(vLOD, ulLODLevelCount);
  return m_vBrickSizes[size_t(iLODIndex)][size_t(Serialize(vBrick, m_vBrickCount[size_t(iLODIndex)]))];
}

UINT64 RasterDataBlock::GetLocalDataPointerOffset(const vector<UINT64>& vLOD, const vector<UINT64>& vBrick) const {
	if (vLOD.size() != ulLODLevelCount.size()) return 0;
	UINT64 iLODIndex = Serialize(vLOD, ulLODLevelCount);
	if (iLODIndex > m_vLODOffsets.size()) return 0;

	if (vBrick.size() != ulBrickSize.size()) return 0;
	UINT64 iBrickIndex = Serialize(vBrick, m_vBrickCount[size_t(iLODIndex)]);

	return GetLocalDataPointerOffset(iLODIndex,iBrickIndex);
}

void RasterDataBlock::SubSample(LargeRAWFile* pSourceFile, LargeRAWFile* pTargetFile, vector<UINT64> sourceSize, vector<UINT64> targetSize, void (*combineFunc)(vector<UINT64> vSource, UINT64 iTarget, const void* pIn, const void* pOut)) {

  pSourceFile->SeekStart();
  pTargetFile->SeekStart();
  
  UINT64 iTargetElementCount = 1;
  UINT64 iReduction = 1;
	vector<UINT64> vReduction;
	for (size_t i = 0;i<targetSize.size();i++) {
		iTargetElementCount*= targetSize[i];
		vReduction.push_back(sourceSize[i] / targetSize[i]);
    iReduction *= vReduction[i];
	}

	// generate offset vector
	vector<vector<UINT64> > vOffsetVectors = GenerateCartesianProduct(CountToVectors(vReduction));
	
	// generate 1D offset coords into serialized source data
	vector<UINT64> vPrefixProd;
	vPrefixProd.push_back(1);
	for (size_t i = 1;i<sourceSize.size();i++) {
		vPrefixProd.push_back(*(vPrefixProd.end()-1) *  sourceSize[i-1]);
	}

	vector<UINT64> vOffsetVector;
	vOffsetVector.resize(vOffsetVectors.size());
	for (size_t i = 0;i<vOffsetVector.size();i++) {
		vOffsetVector[i]  = vOffsetVectors[i][0];
		for (size_t j = 1;j<vPrefixProd.size();j++) vOffsetVector[i] += vOffsetVectors[i][j] * vPrefixProd[j];
	}

	vector<UINT64> sourceElementsSerialized;
	sourceElementsSerialized.resize(vOffsetVector.size());

  unsigned char* pSourceData = NULL;
  unsigned char* pTargetData = NULL;

  UINT64 iSourceMinWindowSize = vOffsetVector[vOffsetVector.size()-1]+1;
  UINT64 iSourceWindowSize = iSourceMinWindowSize+(sourceSize[0]-vReduction[0]);
  UINT64 iTargetWindowSize = targetSize[0];

  vector<UINT64> vSourcePos;
	for (size_t i = 0;i<sourceSize.size();i++) vSourcePos.push_back(0);

  UINT64 iElementSize = ComputeElementSize()/8;
	UINT64 iSourcePos = 0;
  UINT64 iWindowSourcePos = 0;
  UINT64 iWindowTargetPos = 0;
	for (UINT64 i = 0;i<iTargetElementCount;i++) {

    if (i==0 || iWindowTargetPos >= iTargetWindowSize) {
      if (i==0) {
          pSourceData = new unsigned char[size_t(iSourceWindowSize*iElementSize)];
          pTargetData = new unsigned char[size_t(iTargetWindowSize*iElementSize)];
      } else {
        pTargetFile->WriteRAW(pTargetData, iTargetWindowSize*iElementSize);
      }

      if (pSourceFile == pTargetFile) {
        UINT64 iFilePos = pSourceFile->GetPos();   // save and later restore position for inplace subsampling
        pSourceFile->SeekPos(iSourcePos*iElementSize); 
        pSourceFile->ReadRAW(pSourceData, iSourceWindowSize*iElementSize);
        pSourceFile->SeekPos(iFilePos);
      } else {
        pSourceFile->SeekPos(iSourcePos*iElementSize);
        pSourceFile->ReadRAW(pSourceData, iSourceWindowSize*iElementSize);
      }

      iWindowSourcePos = 0;
      iWindowTargetPos = 0;
    }

    // gather data in source array and combine into target array
		for (size_t j = 0;j<vOffsetVector.size();j++) sourceElementsSerialized[j] = vOffsetVector[j] + iWindowSourcePos;
    combineFunc(sourceElementsSerialized, iWindowTargetPos, pSourceData, pTargetData);

		// advance to next position in source array
    iWindowSourcePos += vReduction[0];
    iWindowTargetPos++;

    iSourcePos = 0;
    vSourcePos[0]+=vReduction[0];
    for (size_t j = 1;j<sourceSize.size();j++) {
      if (vSourcePos[j-1]+vReduction[j-1] > sourceSize[j-1]) {
        vSourcePos[j-1] = 0;
        vSourcePos[j] += vReduction[j-1];
      }
      iSourcePos += vPrefixProd[j-1] * vSourcePos[j-1];
    }
    iSourcePos += vPrefixProd[sourceSize.size()-1] * vSourcePos[sourceSize.size()-1];
	}

  pTargetFile->WriteRAW(pTargetData, iTargetWindowSize*iElementSize);
  delete [] pSourceData;
  delete [] pTargetData;
}


void RasterDataBlock::AllocateTemp(const string& strTempFile, bool bBuildOffsetTables) {
	CleanupTemp();

	UINT64 iDataSize = (bBuildOffsetTables) ? ComputeDataSizeAndOffsetTables(): ComputeDataSize();

  m_pTempFile = new LargeRAWFile(strTempFile);
  if (!m_pTempFile->Create(iDataSize)) {
    delete m_pTempFile;
		throw "Unable To create Temp File";
	}
}

/**
 * Dumps the input data into a temp file and calls FlatDataToBrickedLOD
 * \param pSourceData - void pointer to the flat input data
 * \param strTempFile - filename of a temp files during the conversion
 * \param combineFunc - the function used to compute the LOD, this is mostly an average function
 * \return void
 * \see FlatDataToBrickedLOD
 */
void RasterDataBlock::FlatDataToBrickedLOD(const void* pSourceData, const string& strTempFile, 
                                              void (*combineFunc)(vector<UINT64> vSource, UINT64 iTarget, const void* pIn, const void* pOut),
                                              void (*maxminFunc)(const void* pIn, size_t iStart, size_t iCount, double *pfMin, double *pfMa),
                                              MaxMinDataBlock* pMaxMinDatBlock, AbstrDebugOut* pDebugOut) {
  // size of input data
  UINT64 iInPointerSize = ComputeElementSize()/8;
  for (size_t i = 0;i<ulDomainSize.size();i++) iInPointerSize *= ulDomainSize[i];

  // create temp file and dump data into it
  LargeRAWFile pSourceFile(SysTools::AppendFilename(strTempFile,"0"));

  if (!pSourceFile.Create(	iInPointerSize )) {
    throw "Unable To create Temp File ";
  }

  pSourceFile.WriteRAW((unsigned char*)pSourceData, iInPointerSize);


  // call FlatDataToBrickedLOD
  FlatDataToBrickedLOD(&pSourceFile, strTempFile, combineFunc, maxminFunc, pMaxMinDatBlock, pDebugOut);

  // delete tempfile
  pSourceFile.Delete();
}


vector<UINT64> RasterDataBlock::GetLODDomainSize(const vector<UINT64>& vLOD) const {

	vector<UINT64> vReducedDomainSize;
	vReducedDomainSize.resize(ulDomainSemantics.size());

  for (size_t j=0;j<vReducedDomainSize.size();j++)
    vReducedDomainSize[j] = (ulLODDecFactor[j] < 2) 
                             ? ulDomainSize[j] 
                             : max<UINT64>(1,UINT64(floor(double(ulDomainSize[j]) / double((MathTools::Pow(ulLODDecFactor[j],vLOD[size_t(ulLODGroups[j])]))))));

  return vReducedDomainSize;
}

/**
 * Converts data stored in a file to a bricked LODed formtat
 * \param pSourceData - pointer to the source data file
 * \param strTempFile - filename of a temp files during the conversion
 * \param combineFunc - the function used to compute the LOD, this is mostly an average function
 * \return void
 * \see FlatDataToBrickedLOD
 */
void RasterDataBlock::FlatDataToBrickedLOD(LargeRAWFile* pSourceData, const string& strTempFile,
                                           void (*combineFunc)(vector<UINT64> vSource, UINT64 iTarget, const void* pIn, const void* pOut),
                                           void (*maxminFunc)(const void* pIn, size_t iStart, size_t iCount, double *pfMin, double *pfMa),
                                           MaxMinDataBlock* pMaxMinDatBlock, AbstrDebugOut* pDebugOut) {
	UINT64 uiBytesPerElement = ComputeElementSize()/8;

	if (m_pTempFile == NULL) AllocateTemp(SysTools::AppendFilename(strTempFile,"1"),m_vLODOffsets.size() == 0);

  LargeRAWFile* tempFile = NULL;

	// iterate over all LOD-Group Combinations
	vector<vector<UINT64> > vLODCombis = GenerateCartesianProduct(CountToVectors(ulLODLevelCount));

	vector<UINT64> vLastReducedDomainSize;
	vLastReducedDomainSize.resize(ulDomainSemantics.size());

	vector<UINT64> vReducedDomainSize;

	for (size_t i = 0;i<vLODCombis.size();i++) {

		if (pDebugOut) pDebugOut->Message("FlatDataToBrickedLOD", "Generating data for lod level %i of %i",i, vLODCombis.size()); 

    // compute size of the domain
    vReducedDomainSize = GetLODDomainSize(vLODCombis[i]);

		LargeRAWFile* pBrickSource;
		// in the first iteration 0 (outer else case) do not subsample at all but brick the input data at fullres
    // in the second iteration 1 (inner else case) use the input data as source for the subsampling
    // in all other cases use the previously subsampled data to generate the next subsample level
		if (i > 0) {
			if (i > 1) {
				SubSample(tempFile, tempFile, vLastReducedDomainSize, vReducedDomainSize, combineFunc);
      } else {
        tempFile = new LargeRAWFile(SysTools::AppendFilename(strTempFile,"2"));
        if (!tempFile->Create(ComputeDataSize())) {
          delete tempFile;
          throw "Unable To create Temp File";
        }
				SubSample(pSourceData, tempFile, ulDomainSize, vReducedDomainSize, combineFunc);
      }
			pBrickSource = tempFile;
			vLastReducedDomainSize = vReducedDomainSize;
    } else {
			pBrickSource = pSourceData;
		}

		//Console::printf("\n");
		//UINT64 j = 0;
		//for (UINT64 t = 0;t<vReducedDomainSize[2];t++) {
		//	for (UINT64 y = 0;y<vReducedDomainSize[1];y++) {
		//		for (UINT64 x = 0;x<vReducedDomainSize[0];x++) {
		//			Console::printf("%g ", ((double*)pBrickSource)[j++]);
		//		}
		//		Console::printf("\n");
		//	}
		//	Console::printf("\n");
		//	Console::printf("\n");
		//}
		//Console::printf("\n    Generating bricks:\n");

		// compute brick layout
		vector< vector<UINT64> > vBricks = ComputeBricks(vReducedDomainSize);
		vector< vector<UINT64> > vBrickPermutation = GenerateCartesianProduct(vBricks);

		// compute positions of bricks in source data
		vector<UINT64> vBrickLayout;
		for (size_t j=0;j<vBricks.size();j++) vBrickLayout.push_back(vBricks[j].size());
		vector< vector<UINT64> > vBrickIndices = GenerateCartesianProduct(CountToVectors(vBrickLayout));

		vector<UINT64> vPrefixProd;
		vPrefixProd.push_back(1);
		for (size_t j = 1;j<vReducedDomainSize.size();j++) vPrefixProd.push_back(*(vPrefixProd.end()-1) *  vReducedDomainSize[j-1]);

		vector<UINT64> vBrickOffset;
		vBrickOffset.resize(vBrickPermutation.size());
		vBrickOffset[0] = 0;
		for (size_t j=1;j<vBrickPermutation.size();j++) {
			vBrickOffset[j] = 0;
			for (size_t k=0;k<vBrickIndices[j].size();k++) 
				vBrickOffset[j] += vBrickIndices[j][k] * (ulBrickSize[k] - ulBrickOverlap[k]) * vPrefixProd[k] * uiBytesPerElement;
		}

		// ********** fill bricks with data
    unsigned char* pData = new unsigned char[BLOCK_COPY_SIZE];
		for (size_t j=0;j<vBrickPermutation.size();j++) {

   		if (pDebugOut) pDebugOut->Message("FlatDataToBrickedLOD", "Processing brick %i of %i in lod level %i of %i",j,vBrickPermutation.size(),i, vLODCombis.size()); 
			//Console::printf("      Brick %i (",j);
			//for (UINT64 k=0;k<vBrickPermutation[j].size();k++) Console::printf("%i ",vBrickPermutation[j][k]);

			UINT64 iBrickSize = vBrickPermutation[j][0];
			vector<UINT64> vBrickPrefixProd;
			vBrickPrefixProd.push_back(1);
			for (size_t k=1;k<vBrickPermutation[j].size();k++) {
				iBrickSize *= vBrickPermutation[j][k];
				vBrickPrefixProd.push_back(*(vBrickPrefixProd.end()-1) * vBrickPermutation[j][k-1]);
			}

			UINT64 iTargetOffset = GetLocalDataPointerOffset(i,j)/8;
			UINT64 iSourceOffset = vBrickOffset[j];
			UINT64 iPosTargetArray = 0;

      if (pMaxMinDatBlock) pMaxMinDatBlock->StartNewValue();

			for (UINT64 k=0;k<iBrickSize/vBrickPermutation[j][0];k++) {

        m_pTempFile->SeekPos(iTargetOffset);
        pBrickSource->SeekPos(iSourceOffset);


        UINT64 iDataSize = vBrickPermutation[j][0] * uiBytesPerElement;
	      for (UINT64 l = 0;l<iDataSize;l+=BLOCK_COPY_SIZE) {
		      UINT64 iCopySize = min(BLOCK_COPY_SIZE, iDataSize-l);

          pBrickSource->ReadRAW(pData, iCopySize);
          m_pTempFile->WriteRAW(pData, iCopySize);

          if (pMaxMinDatBlock) {
            double fMin, fMax, fMinGrad=-std::numeric_limits<double>::max(), fMaxGrad=std::numeric_limits<double>::max();
            /// \todo compute gradients
            maxminFunc(pData, 0, size_t(iCopySize/uiBytesPerElement), &fMin, &fMax);
            pMaxMinDatBlock->MergeData(fMin, fMax, fMinGrad, fMaxGrad);
          }
        }

        iTargetOffset += vBrickPermutation[j][0] * uiBytesPerElement;

				iPosTargetArray += vBrickPermutation[j][0];
				if (iPosTargetArray % vBrickPrefixProd[1] == 0) iSourceOffset += vReducedDomainSize[0] * uiBytesPerElement;

				for (size_t l = 2;l<vReducedDomainSize.size();l++)
					if (iPosTargetArray % vBrickPrefixProd[l] == 0) {
						iSourceOffset -=	(vBrickPermutation[j][l-1] * vPrefixProd[l-1]) * uiBytesPerElement;
						iSourceOffset +=   vPrefixProd[l] * uiBytesPerElement;
					}
			}


			//Console::printf(")\n");

			//Console::printf("\n");
			//UINT64 xxx = 0;
			//UINT64 yyy = GetLocalDataPointerOffset(i,j)/64;
			//for (UINT64 t = 0;t<vBrickPermutation[j][2];t++) {
			//	for (UINT64 y = 0;y<vBrickPermutation[j][1];y++) {
			//		for (UINT64 x = 0;x<vBrickPermutation[j][0];x++) {
			//			Console::printf("%g ", ((double*)pData+yyy)[xxx++]);
			//		}
			//		Console::printf("\n");
			//	}
			//	Console::printf("\n");
			//}

		}
    delete [] pData;

//		Console::printf("\n");
	}

  if (tempFile != NULL) {
    tempFile->Delete();
    delete tempFile;
    tempFile = NULL;
  }
}

void RasterDataBlock::CleanupTemp() { 
  if (m_pTempFile != NULL) {
    m_pTempFile->Delete(); 
    delete m_pTempFile;
    m_pTempFile = NULL;
  }
}


bool RasterDataBlock::GetData(unsigned char** ppData, const vector<UINT64>& vLOD, const vector<UINT64>& vBrick) const {
  if (m_pTempFile == NULL && m_pStreamFile == NULL) return false;
  if (m_vLODOffsets.size() == 0) return false;

  LargeRAWFile*  pStreamFile;
 	UINT64 iOffset = GetLocalDataPointerOffset(vLOD, vBrick)/8;

  if (m_pStreamFile != NULL) {
	  // add global offset
	  iOffset += m_iOffset;
	  // add size of header
	  iOffset += DataBlock::GetOffsetToNextBlock() + ComputeHeaderSize();
    pStreamFile = m_pStreamFile;
  } else {
    pStreamFile = m_pTempFile;
  }

  // new - memory if needed
  vector<UINT64> vSize = GetBrickSize(vLOD,vBrick);
  UINT64 iSize = ComputeElementSize()/8;
  for (size_t i = 0;i<vSize.size();i++) iSize *= vSize[i];
  if (*ppData == NULL) *ppData = new unsigned char[size_t(iSize)];
  if (*ppData == NULL) return false;


  // copy data from file
  pStreamFile->SeekPos(iOffset);
  pStreamFile->ReadRAW((*ppData), iSize);

  return true;
}


bool RasterDataBlock::SetData(unsigned char* pData, const vector<UINT64>& vLOD, const vector<UINT64>& vBrick) {
  if (m_pStreamFile == NULL || !m_pStreamFile->IsWritable() || m_vLODOffsets.size() == 0) return false;

	UINT64 iOffset = GetLocalDataPointerOffset(vLOD, vBrick)/8;

	// add global offset
	iOffset += m_iOffset;

	// add size of header
	iOffset += DataBlock::GetOffsetToNextBlock() + ComputeHeaderSize();

  // new - memory if needed
  vector<UINT64> vSize = GetBrickSize(vLOD,vBrick);
  UINT64 iSize = ComputeElementSize()/8;
  for (size_t i = 0;i<vSize.size();i++) iSize *= vSize[i];

  // copy data from file
  m_pStreamFile->SeekPos(iOffset);
  m_pStreamFile->WriteRAW(pData, iSize);

  return true;
}
