#ifndef LIB_PIPELINE_STUBS_H
#define LIB_PIPELINE_STUBS_H 

// ****************************************************************************
// LibPipelineStubs 
//
// Purpose: Define references for all the symbols from libpipeline that are
// needed to link the mdserver but which are never referenced in the mdserver's
// normal execution
//
// Programmer: Mark C. Miller 
// Creation:   October 25, 2005 
//
// Modifications:
//   Kathleen Bonnell, Wed May 17 10:58:54 PDT 2006
//   Added stub for vtkWriter::SetInput(vtkDataObject*).
//
// ****************************************************************************

class vtkPolyData;
class vtkDataSet;
class vtkImageData;
class vtkDataObject;

class vtkAppendFilter {
  public:
    void  AddInput(vtkDataSet*);
    void *New();
};

class vtkAppendPolyData {
  public:
    void  AddInput(vtkPolyData*);
    void *New();
};

class vtkCleanPolyData {
  public:
    void *New();
};

class vtkDataSetReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkDataSetWriter {
  public:
    void  SetInput(vtkDataSet*);
    void *New();
};

class vtkDataWriter {
  public:
    char *RegisterAndGetOutputString();
};

class vtkWriter {
  public:
    void  SetInput(vtkDataObject*);
};

class vtkPolyDataReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkRectilinearGridReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkStructuredGridReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkStructuredPointsReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkStructuredPointsWriter {
  public:
    void SetInput(vtkImageData*);
    void *New();
};

class vtkUnstructuredGridReader {
  public:
    void *GetOutput();
    void *New();
};

class vtkCSGGrid {
  public:
    void DiscretizeSpace(int,int,int,double,double,double,double,double,double,double);
    void DiscretizeSurfaces(int, double, double, double, double, double, double, double);
    void DiscretizeSpace(int, double, double, double, double, double, double, double);
};
#endif
