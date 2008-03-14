/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkCEAucdReader.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCEAucdReader - reads a dataset in CEA "UCD" format
// .SECTION Description
// vtkCEAucdReader creates an unstructured grid dataset. It reads binary or
// ASCII files stored in UCD format, with optional data stored at the nodes
// or at the cells of the model. A cell-based fielddata stores the material
// id. The class can automatically detect the endian-ness of the binary files.

// .SECTION Thanks
// Thanks to Guenole Harel and Emmanuel Colin (Supelec engineering school,
// France) and Jean M. Favre (CSCS, Switzerland) who co-developed this class.
// Thanks to Isabelle Surin (isabelle.surin at cea.fr, CEA-DAM, France) who
// supervised the internship of the first two authors. Thanks to Daniel
// Aguilera (daniel.aguilera at cea.fr, CEA-DAM, France) who contributed code
// and advice. Please address all comments to Jean Favre (jfavre at cscs.ch)

// .SECTION See Also
// vtkGAMBITReader

#ifndef __vtkCEAucdReader_h
#define __vtkCEAucdReader_h

#include <vtkUnstructuredGridAlgorithm.h>
#include <fstream>
#include <vector>

class vtkIntArray;
class vtkFloatArray;
class vtkIdTypeArray;
class vtkDataArraySelection;
class vtkUnsignedCharArray;
class vtkCellArray;

class vtkCEAucdReader : public vtkUnstructuredGridAlgorithm
{
   public:
      static vtkCEAucdReader *New();
      vtkTypeRevisionMacro(vtkCEAucdReader,vtkUnstructuredGridAlgorithm);
      void PrintSelf(ostream& os, vtkIndent indent);
      
     // Description:
     // Specify file name of CEA UCD datafile to read
      vtkSetStringMacro(FileName);
      vtkGetStringMacro(FileName);
      vtkSetMacro(GlobalOffset, long);
      vtkSetMacro(FileSize, long);
      
     // Description :
     // If true, multiple outputs (one per material id) are generated,
     // otherwise a single output for all cells is generated.
      vtkSetMacro(PerMaterialOutput, int);
      vtkGetMacro(PerMaterialOutput, int);
      vtkBooleanMacro(PerMaterialOutput, int);
      
     // Description:
     // Is the file to be read written in binary format (as opposed to ascii).
      vtkSetMacro(BinaryFile, int);
      vtkGetMacro(BinaryFile, int);
      vtkBooleanMacro(BinaryFile, int);
      
     // Description:
     // Get the total number of cells.
      vtkGetMacro(NumberOfCells,int);
      
     // Description:
     // Get the total number of nodes.
      vtkGetMacro(NumberOfNodes,int);
      
     // Description:
     // Get the number of data fields at the nodes.
      vtkGetMacro(NumberOfNodeFields,int);
      
     // Description:
     // Get the number of data fields at the cell centers.
      vtkGetMacro(NumberOfCellFields,int);
      
     // Description:
     // Get the number of data fields for the model. Unused because VTK
     // has no methods for it.
      vtkGetMacro(NumberOfFields,int);
      
     // Description:
     // Get the number of data components at the nodes and cells.
      vtkGetMacro(NumberOfNodeComponents,int);
      vtkGetMacro(NumberOfCellComponents,int);
      
     // Description:
     // Get number of material ids used in file
      vtkGetMacro(NumberOfMaterials,int);
      
     // Description:
     // Set/Get the endian-ness of the binary file.
      void SetByteOrderToBigEndian();
      void SetByteOrderToLittleEndian();
      const char *GetByteOrderAsString();
      
      vtkSetMacro(ByteOrder, int);
      vtkGetMacro(ByteOrder, int);
      
     // Description:
     // Set/Get name of default cell scalars
      vtkSetStringMacro(ActiveCellArray);
      vtkGetStringMacro(ActiveCellArray);
      
     // Description:
     // Set/Get name of default cell scalars
      vtkSetStringMacro(ActivePointArray);
      vtkGetStringMacro(ActivePointArray);
      
      void SetFileStream( ifstream* istr );

     // Description:
     // The following methods allow selective reading of solutions fields.  by
     // default, ALL data fields are the nodes and cells are read, but this can
     // be modified.
      int GetNumberOfPointArrays();
      int GetNumberOfCellArrays();
      const char* GetPointArrayName(int index);
      const char* GetCellArrayName(int index);
      int GetPointArrayStatus(const char* name);
      int GetCellArrayStatus(const char* name);
      void SetPointArrayStatus(const char* name, int status);  
      void SetCellArrayStatus(const char* name, int status);
      
      void DisableAllCellArrays();
      void EnableAllCellArrays();
      void DisableAllPointArrays();
      void EnableAllPointArrays();
      
     // get min and max value for the index-th value of a cell component
     // index varies from 0 to (veclen - 1)
      void GetCellDataRange(int cellComp, int index, float *min, float *max);
      
     // get min and max value for the index-th value of a node component
     // index varies from 0 to (veclen - 1)
      void GetNodeDataRange(int nodeComp, int index, float *min, float *max);
      
   protected:
      vtkCEAucdReader();
      ~vtkCEAucdReader();
      int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
      int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
      
      char *FileName;
      int BinaryFile;
      int PerMaterialOutput;
      int NumberOfMaterials;
      
      int NumberOfNodes;
      int NumberOfCells;
      int NumberOfNodeFields;
      int NumberOfNodeComponents;
      int NumberOfCellComponents;
      int NumberOfCellFields;
      int NumberOfFields;
      int NlistNodes;
      
      vtkIntArray * CellMaterialId;
      vtkIdType * CellsInMaterial;
      vtkIdType * PointsInMaterial;
      vtkIdType * CellIndexInMaterial;
      
      ifstream *FileStream;
      long GlobalOffset;
      long FileSize;
      bool OwnStream;
      vtkDataArraySelection* PointDataArraySelection;
      vtkDataArraySelection* CellDataArraySelection;
      
      char * ActiveCellArray;
      char * ActivePointArray;
      
      int DecrementNodeIds;
      int ByteOrder;
      int GetLabel(char *string, int number, char *label);
     //BTX
      enum
      {
	 FILE_BIG_ENDIAN=0,
	 FILE_LITTLE_ENDIAN=1
      };
      enum UCDCell_type
      {
	 PT    = 0,
	 LINE  = 1,
	 TRI   = 2,
	 QUAD  = 3,
	 TET   = 4,
	 PYR   = 5,
	 PRISM = 6,
	 HEX   = 7
      };
      
      struct DataInfo {
	    long foffset; // offset in binary file
	    int  veclen;  // number of components in the node or cell variable
	    float min[3]; // pre-calculated data minima (max size 3 for vectors)
	    float max[3]; // pre-calculated data maxima (max size 3 for vectors)
      };
     //ETX
      
      DataInfo *NodeDataInfo;
      DataInfo *CellDataInfo;
      
      size_t FileStreamPos;
      void FileStreamSeek( size_t pos );
      void FileStreamSeekEnd();

     //BTX
      int CachedPerMaterialOutput;
      char* CachedFileName;
      const std::ifstream* CachedFileStream;
      long CachedGlobalOffset;
      std::vector<vtkUnstructuredGrid*> CachedOutputs;

      void CacheOutputs();
      void ClearCache();
      void PruneCache();

      vtkDataArray* GetPointsDataFromCache(int outputPort);
      int GetCellsFromCache( int outputPort, vtkUnsignedCharArray* &cellTypes, vtkIdTypeArray* &cellLocations, vtkCellArray* &cellArray );
      vtkDataArray* GetCellDataArrayFromCache(int outputPort, const char* arrayName);
      vtkDataArray* GetPointDataArrayFromCache(int outputPort, const char* arrayName);
     //ETX

   private:
      void ReadFile(vtkInformationVector * outputVector);
      void ReadGeometry(vtkInformationVector *outputVector);
      void ReadNodeData(vtkUnstructuredGrid *output);
      void ReadCellData(vtkUnstructuredGrid *output);
      
      int ReadFloatBlock(int n, float *block);
      int ReadIntBlock(int n, int *block);
      void ReadXYZCoords(vtkFloatArray *coords);
      void ReadBinaryCellTypes(int* ctypes, bool buildTables=true);
      void ReadBinaryCellTopology(const int * ctypes);
      void ReadASCIICellTopology(vtkIntArray *material, vtkUnstructuredGrid *output);
      
      vtkCEAucdReader(const vtkCEAucdReader&);  // Not implemented.
      void operator=(const vtkCEAucdReader&);  // Not implemented.

};

#endif
