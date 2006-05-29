/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLDataParser.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLDataParser - Used by vtkVisItXMLReader to parse VTK XML files.
// .SECTION Description
// vtkVisItXMLDataParser provides a subclass of vtkVisItXMLParser that
// constructs a representation of an XML data format's file using
// vtkVisItXMLDataElement to represent each XML element.  This
// representation is then used by vtkVisItXMLReader and its subclasses to
// traverse the structure of the file and extract data.

// .SECTION See Also
// vtkVisItXMLDataElement

#ifndef __vtkVisItXMLDataParser_h
#define __vtkVisItXMLDataParser_h

#include "vtkVisItXMLParser.h"

class vtkVisItXMLDataElement;
class vtkVisItInputStream;
class vtkVisItDataCompressor;

class vtkVisItXMLDataParser : public vtkVisItXMLParser
{
public:
  vtkTypeRevisionMacro(vtkVisItXMLDataParser,vtkVisItXMLParser);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkVisItXMLDataParser* New();

  // Description:
  // Get the root element from the XML document.
  vtkVisItXMLDataElement* GetRootElement();

  //BTX
  // Description:
  // Enumerate big and little endian byte order settings.
  enum { BigEndian, LittleEndian };

  // Description:
  // A type used for data sizes and offsets for stream i/o.  Using
  // vtkIdType should satisfy most users.  This could be streamoff if
  // it is deemed portable.  It could also be split into OffsetType
  // (streamoff) and PositionType (streampos).
  typedef vtkIdType OffsetType;

  // Description:
  // Read inline data from inside the given element.  Returns the
  // number of words read.
  OffsetType ReadInlineData(vtkVisItXMLDataElement* element, int isAscii,
                            void* buffer, OffsetType startWord,
                            OffsetType numWords, int wordType);
  OffsetType ReadInlineData(vtkVisItXMLDataElement* element, int isAscii,
                            char* buffer, OffsetType startWord,
                            OffsetType numWords)
    { return this->ReadInlineData(element, isAscii, buffer, startWord,
                                  numWords, VTK_CHAR); }

  // Description:
  // Read from an appended data section starting at the given appended
  // data offset.  Returns the number of words read.
  OffsetType ReadAppendedData(OffsetType offset, void* buffer,
                              OffsetType startWord,
                              OffsetType numWords, int wordType);
  OffsetType ReadAppendedData(OffsetType offset, char* buffer,
                              OffsetType startWord,
                              OffsetType numWords)
    { return this->ReadAppendedData(offset, buffer, startWord, numWords,
                                    VTK_CHAR); }

  // Description:
  // Read from an ascii data section starting at the current position in
  // the stream.  Returns the number of words read.
  OffsetType ReadAsciiData(void* buffer, OffsetType startWord,
                           OffsetType numWords, int wordType);

  // Description:
  // Read from a data section starting at the current position in the
  // stream.  Returns the number of words read.
  OffsetType ReadBinaryData(void* buffer, OffsetType startWord,
                            OffsetType maxWords, int wordType);
  //ETX

  // Description:
  // Get/Set the compressor used to decompress binary and appended data
  // after reading from the file.
  virtual void SetCompressor(vtkVisItDataCompressor*);
  vtkGetObjectMacro(Compressor, vtkVisItDataCompressor);

  // Description:
  // Get the size of a word of the given type.
  unsigned long GetWordTypeSize(int wordType);

  // Description:
  // Parse the XML input and check that the file is safe to read.
  // Returns 1 for okay, 0 for error.
  virtual int Parse();

  // Description:
  // Get/Set flag to abort reading of data.  This may be set by a
  // progress event observer.
  vtkGetMacro(Abort, int);
  vtkSetMacro(Abort, int);

  // Description:
  // Get/Set progress of reading data.  This may be checked by a
  // progress event observer.
  vtkGetMacro(Progress, float);
  vtkSetMacro(Progress, float);

  // Description:
  // Get/Set the character encoding that will be used to set the attributes's
  // encoding type of each vtkVisItXMLDataElement created by this parser (i.e.,
  // the data element attributes will use that encoding internally).
  // If set to VTK_ENCODING_NONE (default), the attribute encoding type will
  // not be changed and will default to the vtkVisItXMLDataElement default encoding
  // type (see vtkVisItXMLDataElement::AttributeEncoding).
  vtkSetClampMacro(AttributesEncoding,int,VTK_ENCODING_NONE,VTK_ENCODING_UNKNOWN);
  vtkGetMacro(AttributesEncoding, int);

protected:
  vtkVisItXMLDataParser();
  ~vtkVisItXMLDataParser();

  // This parser does not support parsing from a string.
  virtual int Parse(const char*);
  virtual int Parse(const char*, unsigned int);

  // Implement parsing methods.
  void StartElement(const char* name, const char** atts);
  void EndElement(const char*);
  int ParsingComplete();
  int CheckPrimaryAttributes();
  void FindAppendedDataPosition();
  OffsetType FindInlineDataPosition(OffsetType start);
  int ParseBuffer(const char* buffer, unsigned int count);

  void AddElement(vtkVisItXMLDataElement* element);
  void PushOpenElement(vtkVisItXMLDataElement* element);
  vtkVisItXMLDataElement* PopOpenElement();
  void FreeAllElements();
  void PerformByteSwap(void* data, OffsetType numWords, int wordSize);

  // Data reading methods.
  void ReadCompressionHeader();
  unsigned int FindBlockSize(unsigned int block);
  int ReadBlock(unsigned int block, unsigned char* buffer);
  unsigned char* ReadBlock(unsigned int block);
  OffsetType ReadUncompressedData(unsigned char* data,
                                  OffsetType startWord,
                                  OffsetType numWords,
                                  int wordSize);
  OffsetType ReadCompressedData(unsigned char* data,
                                OffsetType startWord,
                                OffsetType numWords,
                                int wordSize);

  // Ascii data reading methods.
  int ParseAsciiData(int wordType);
  void FreeAsciiBuffer();

  // Progress update methods.
  void UpdateProgress(float progress);

  // The root XML element.
  vtkVisItXMLDataElement* RootElement;

  // The stack of elements currently being parsed.
  vtkVisItXMLDataElement** OpenElements;
  unsigned int NumberOfOpenElements;
  unsigned int OpenElementsSize;

  // The position of the appended data section, if found.
  OffsetType AppendedDataPosition;

  // How much of the string "<AppendedData" has been matched in input.
  int AppendedDataMatched;

  // The byte order of the binary input.
  int ByteOrder;

  // The input stream used to read data.  Set by ReadAppendedData and
  // ReadInlineData methods.
  vtkVisItInputStream* DataStream;

  // The input stream used to read inline data.  May transparently
  // decode the data.
  vtkVisItInputStream* InlineDataStream;

  // The stream to use for appended data.
  vtkVisItInputStream* AppendedDataStream;

  //BTX
  // We need a 32 bit unsigned integer type for platform-independent
  // binary headers.  Note that this is duplicated in vtkVisItXMLWriter.h.
#if VTK_SIZEOF_SHORT == 4
  typedef unsigned short HeaderType;
#elif VTK_SIZEOF_INT == 4
  typedef unsigned int HeaderType;
#elif VTK_SIZEOF_LONG == 4
  typedef unsigned long HeaderType;
#else
# error "No native data type can represent an unsigned 32-bit integer."
#endif
  //ETX

  // Decompression data.
  vtkVisItDataCompressor* Compressor;
  unsigned int NumberOfBlocks;
  unsigned int BlockUncompressedSize;
  unsigned int PartialLastBlockUncompressedSize;
  HeaderType* BlockCompressedSizes;
  OffsetType* BlockStartOffsets;

  // Ascii data parsing.
  unsigned char* AsciiDataBuffer;
  OffsetType AsciiDataBufferLength;
  int AsciiDataWordType;
  OffsetType AsciiDataPosition;

  // Progress during reading of data.
  float Progress;

  // Abort flag checked during reading of data.
  int Abort;

  int AttributesEncoding;

private:
  vtkVisItXMLDataParser(const vtkVisItXMLDataParser&);  // Not implemented.
  void operator=(const vtkVisItXMLDataParser&);  // Not implemented.
};

#endif
