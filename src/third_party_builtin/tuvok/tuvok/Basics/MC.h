/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


//!    File   : MC.h
//!    Author : Jens Krueger
//!             SCI Institute
//!             University of Utah
//!             MC tables by others (see remarks below)
//!    Date   : January 2009
//
//!    Copyright (C) 2008 SCI Institute

#pragma once

#include "Vectors.h"

#define EPSILON 0.000001f
#define DATA_INDEX(I, J, K, IDIM, JDIM) ((I) + ((IDIM) * (J)) + ((IDIM * JDIM) * (K)))
#define EDGE_INDEX(E, I, J, IDIM) ((E) + (12 * (I)) + ((12 * (IDIM)) * (J)))
#define NO_EDGE -1

template <class T=float> class LayerTempData {
public:
	T*	  pTBotData;
	T*	  pTTopData;
	int*	piEdges;  // tag indexing into vertex list

  LayerTempData<T>(INTVECTOR3 vVolSize, T* pTVolume);
	virtual ~LayerTempData();
	void NextIteration();

private:
	INTVECTOR3 m_vVolSize;
};

class Isosurface {
public:
	FLOATVECTOR3*	vfVertices;
	FLOATVECTOR3*	vfNormals;
	INTVECTOR3*		viTriangles;
	int				    iVertices;
	int				    iTriangles;

	Isosurface();
	Isosurface(int iMaxVertices, int iMaxTris);
	virtual ~Isosurface();

	int AddTriangle(int a, int b, int c);
	int AddVertex(FLOATVECTOR3 v, FLOATVECTOR3 n);
	void AppendData(const Isosurface* other);
	void Transform(const FLOATMATRIX4& matrix);
};


template <class T=float> class MarchingCubes {
public:
	Isosurface*		m_Isosurface;

	MarchingCubes<T>(void);
	virtual ~MarchingCubes<T>(void);

	virtual void SetVolume(int iSizeX, int iSizeY, int iSizeZ, T* pTVolume);
	virtual void Process(T TIsoValue);

protected:
	static int		ms_edgeTable[256];
	static int		ms_triTable[256][16];

	INTVECTOR3	  m_vVolSize;
  INTVECTOR3	  m_vOffset;
	T*			      m_pTVolume;
	T			        m_TIsoValue;

	virtual void MarchLayer(LayerTempData<T> *layer, int iLayer);
	virtual int MakeVertex(int whichEdge, int i, int j, int k, Isosurface* sliceIso);
	virtual FLOATVECTOR3 InterpolateNormal(T fValueAtPos, INTVECTOR3 vPosition);

};

#include "MC.inl"
