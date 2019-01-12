#ifndef vtkElementLabelArray_h
#define vtkElementLabelArray_h

#include "vtkCharArray.h"
#include <visit_vtk_light_exports.h>

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include <map>

class VISIT_VTK_LIGHT_API vtkElementLabelArray : public vtkCharArray 
{
public:
  static vtkElementLabelArray *New();
  vtkTypeMacro(vtkElementLabelArray,vtkCharArray);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  bool AddName(const std::string &name, 
               const std::vector<int> &labelRangesBegin,
               const std::vector<int> &labelRangesEnd,
               const std::vector<int> &elementIdsBegin,
               const std::vector<int> &elementIdsEnd);
  bool GetElementId(const std::string &name,
                    int &id);
  static void Destruct(void *ptr);
protected:
  struct LabelToZoneMapping
  {
      std::vector<int> LabelRangesBegin;
      std::vector<int> LabelRangesEnd;
      std::vector<int> ElementIdsBegin;
      std::vector<int> ElementIdsEnd;
  };
  vtkElementLabelArray();
  virtual ~vtkElementLabelArray();
  std::map<std::string, LabelToZoneMapping> NameMapping; 
private:
  vtkElementLabelArray(const vtkElementLabelArray&);   // not implemented
  void operator=(const vtkElementLabelArray&);  // not implemented
};

#endif
