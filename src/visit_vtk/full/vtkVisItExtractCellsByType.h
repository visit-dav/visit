// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
   * determining if a cell is in the set are also provided.
   */
  void AddCellType(unsigned int type);
  void AddAllCellTypes();
  void RemoveCellType(unsigned int type);
  void RemoveAllCellTypes();
  bool ExtractCellType(unsigned int type);
  //@}

protected:
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int FillInputPortInformation(int port, vtkInformation *info) override;

  void ExtractUnstructuredData(vtkDataSet *input, vtkDataSet *output);
  void ExtractPolyDataCells(vtkDataSet *input, vtkDataSet *output,
                            vtkIdType *ptMap, vtkIdType &numNewPts);
  void ExtractUnstructuredGridCells(vtkDataSet *input, vtkDataSet *output,
                                    vtkIdType *ptMap, vtkIdType &numNewPts);

  vtkVisItExtractCellsByType();
  ~vtkVisItExtractCellsByType() override;

private:
  vtkVisItExtractCellsByType(const vtkVisItExtractCellsByType&) = delete;
  void operator=(const vtkVisItExtractCellsByType&) = delete;

  vtkCellTypeSet *CellTypes;
};

#endif
