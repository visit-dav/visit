/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAVSucdReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkAVSucdReader
 * @brief   reads a dataset in AVS "UCD" format
 *
 * VisItAVSucdReader creates an unstructured grid dataset. It reads binary or
 * ASCII files stored in UCD format, with optional data stored at the nodes
 * or at the cells of the model. A cell-based fielddata stores the material
 * id. The class can automatically detect the endian-ness of the binary files.
 *
 * @par Thanks:
 * Thanks to Guenole Harel and Emmanuel Colin (Supelec engineering school,
 * France) and Jean M. Favre (CSCS, Switzerland) who co-developed this class.
 * Thanks to Isabelle Surin (isabelle.surin at cea.fr, CEA-DAM, France) who
 * supervised the internship of the first two authors. Thanks to Daniel
 * Aguilera (daniel.aguilera at cea.fr, CEA-DAM, France) who contributed code
 * and advice. Please address all comments to Jean Favre (jfavre at cscs.ch)
 *
 * @sa
 * vtkGAMBITReader
 *
 * Changes made by KSB at LLNL for use with VisIt:  Renamed to avoid collisions.
 * Don't store node/cell data arrays in the output, but in the DataInfo structs.
 * The DataArraySelection path for choosing which vars to load only works with Binary
 * files and only if the variable names are known in advance.
 * Add method to retrieve the node/cell data arrays when needed.
*/

#ifndef VisItAVSucdReader_h
#define VisItAVSucdReader_h

#include <vtkNew.h>
#include <vtkUnstructuredGridAlgorithm.h>

class vtkIntArray;
class vtkFloatArray;
class vtkIdTypeArray;
class vtkDataArraySelection;

class VisItAVSucdReader : public vtkUnstructuredGridAlgorithm
{
public:
  static VisItAVSucdReader *New();
  vtkTypeMacro(VisItAVSucdReader,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
   * Specify file name of AVS UCD datafile to read
   */
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  //@}

  //@{
  /**
   * Is the file to be read written in binary format (as opposed to ascii).
   */
  vtkSetMacro(BinaryFile, int);
  vtkGetMacro(BinaryFile, int);
  vtkBooleanMacro(BinaryFile, int);
  //@}

  //@{
  /**
   * Get the total number of cells.
   */
  vtkGetMacro(NumberOfCells,int);
  //@}

  //@{
  /**
   * Get the total number of nodes.
   */
  vtkGetMacro(NumberOfNodes,int);
  //@}

  //@{
  /**
   * Get the number of data fields at the nodes.
   */
  vtkGetMacro(NumberOfNodeFields,int);
  //@}

  //@{
  /**
   * Get the number of data fields at the cell centers.
   */
  vtkGetMacro(NumberOfCellFields,int);
  //@}

  //@{
  /**
   * Get the number of data fields for the model. Unused because VTK
   * has no methods for it.
   */
  vtkGetMacro(NumberOfFields,int);
  //@}

  //@{
  /**
   * Get the number of data components at the nodes and cells.
   */
  vtkGetMacro(NumberOfNodeComponents,int);
  vtkGetMacro(NumberOfCellComponents,int);
  //@}

  //@{
  /**
   * Set/Get the endian-ness of the binary file.
   */
  void SetByteOrderToBigEndian();
  void SetByteOrderToLittleEndian();
  const char *GetByteOrderAsString();
  //@}

  vtkSetMacro(ByteOrder, int);
  vtkGetMacro(ByteOrder, int);

  //@{
  /**
   * By default, ALL data fields in the nodes and cells are read, but not added
   * to the output.  The following methods allow selective retrieval of solution fields,
   * and attributes.  Added by KSB: retrieval of units, veclen and data array.
   */
  int GetNumberOfPointArrays();
  int GetNumberOfCellArrays();
  const char* GetPointArrayName(int index);
  const char* GetCellArrayName(int index);
  const char* GetPointArrayUnits(int index);
  const char* GetCellArrayUnits(int index);
  int GetPointArrayVecLength(int index);
  int GetCellArrayVecLength(int index);
  vtkDataArray *GetPointArray(int index);
  vtkDataArray *GetCellArray(int index);
  vtkDataArray *GetPointArray(const char *name);
  vtkDataArray *GetCellArray(const char *name);
  //@}

  // get min and max value for the index-th value of a cell component
  // index varies from 0 to (veclen - 1)
  void GetCellDataRange(int cellComp, int index, float *min, float *max);

  // get min and max value for the index-th value of a node component
  // index varies from 0 to (veclen - 1)
  void GetNodeDataRange(int nodeComp, int index, float *min, float *max);

  // Added by KSB, LLNL
  vtkGetMacro(TopologicalDimension,int);

protected:
  VisItAVSucdReader();
  ~VisItAVSucdReader() override;
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  char *FileName;
  int BinaryFile;

  int NumberOfNodes;
  int NumberOfCells;
  int NumberOfNodeFields;
  int NumberOfNodeComponents;
  int NumberOfCellComponents;
  int NumberOfCellFields;
  int NumberOfFields;
  int NlistNodes;

  std::ifstream *FileStream;

  int ByteOrder;
  int GetLabel(char *string, int number, char *label);

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

  // Modified by KSB, LLNL, added name, units, data
  struct DataInfo {
    long foffset; // offset in binary file
    int  veclen;  // number of components in the node or cell variable
    float min[3]; // pre-calculated data minima (max size 3 for vectors)
    float max[3]; // pre-calculated data maxima (max size 3 for vectors)
    std::string name;  // name label for the node or cell variable
    std::string units; // units label for the node or cell variable
    vtkNew<vtkFloatArray> data;
  };

  DataInfo *NodeDataInfo;
  DataInfo *CellDataInfo;

private:
  struct idMapping;

  void ReadFile(vtkUnstructuredGrid *output);
  void ReadGeometry(vtkUnstructuredGrid *output,
                    idMapping& nodeMap,
                    idMapping& cellMap);

  // Modified by KSB, LLNL, removed output argument as data arrays no longer added to output.
  void ReadNodeData(const idMapping& nodeMap);
  void ReadCellData(const idMapping& cellMap);

  // Added by KSB, LLNL. These were split from ReadNodeData and ReadCellData
  // to consolidate duplicate code
  void ReadBinaryData(const int numTuples, const int numComp,
                      DataInfo *&dataInfo);
  void ReadASCIIData(const int numTuples, int &numComp,
                     DataInfo *&dataInfo , const idMapping &idMap);

  int ReadFloatBlock(int n, float *block);
  int ReadIntBlock(int n, int *block);
  void ReadXYZCoords(vtkFloatArray *coords, idMapping& nodeMap);
  void ReadBinaryCellTopology(vtkIntArray *material, int *types,
                              vtkIdTypeArray *listcells);
  void ReadASCIICellTopology(vtkIntArray *material,
                             vtkUnstructuredGrid *output,
                             const idMapping& nodeMap,
                             idMapping& cellMap);

  // Added by KSB, LLNL
  int TopologicalDimension;
  vtkNew<vtkIntArray> Materials;

  VisItAVSucdReader(const VisItAVSucdReader&) = delete;
  void operator=(const VisItAVSucdReader&) = delete;
};

#endif
