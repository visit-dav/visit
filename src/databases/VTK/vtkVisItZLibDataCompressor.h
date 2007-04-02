/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItZLibDataCompressor.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItZLibDataCompressor - Data compression using zlib.
// .SECTION Description
// vtkVisItZLibDataCompressor provides a concrete vtkVisItDataCompressor class
// using zlib for compressing and uncompressing data.

#ifndef __vtkVisItZLibDataCompressor_h
#define __vtkVisItZLibDataCompressor_h

#include "vtkVisItDataCompressor.h"

class vtkVisItZLibDataCompressor : public vtkVisItDataCompressor
{
public:
  vtkTypeRevisionMacro(vtkVisItZLibDataCompressor,vtkVisItDataCompressor);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkVisItZLibDataCompressor* New();  
  
  // Description:  
  // Get the maximum space that may be needed to store data of the
  // given uncompressed size after compression.  This is the minimum
  // size of the output buffer that can be passed to the four-argument
  // Compress method.
  unsigned long GetMaximumCompressionSpace(unsigned long size);

  // Description:
  // Get/Set the compression level.
  vtkSetClampMacro(CompressionLevel, int, 0, 9);
  vtkGetMacro(CompressionLevel, int);
  
protected:
  vtkVisItZLibDataCompressor();
  ~vtkVisItZLibDataCompressor();
  
  int CompressionLevel;

  // Compression method required by vtkVisItDataCompressor.
  unsigned long CompressBuffer(const unsigned char* uncompressedData,
                               unsigned long uncompressedSize,
                               unsigned char* compressedData,
                               unsigned long compressionSpace);
  // Decompression method required by vtkVisItDataCompressor.
  unsigned long UncompressBuffer(const unsigned char* compressedData,
                                 unsigned long compressedSize,
                                 unsigned char* uncompressedData,
                                 unsigned long uncompressedSize);
private:
  vtkVisItZLibDataCompressor(const vtkVisItZLibDataCompressor&);  // Not implemented.
  void operator=(const vtkVisItZLibDataCompressor&);  // Not implemented.
};

#endif
