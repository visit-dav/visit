// ************************************************************************* //
//                                avtLevelsMapper.h                          //
// ************************************************************************* //

#ifndef AVT_LEVELS_MAPPER_H
#define AVT_LEVELS_MAPPER_H
#include <plotter_exports.h>


#include <avtMapper.h>
#include <LineAttributes.h>
#include <ColorAttributeList.h>
#include <maptypes.h>

// ****************************************************************************
//  Class: avtLevelsMapper
//
//  Purpose:
//      A mapper that will take a lookup table of key values and assign colors
//      to them not based on where they are located in the table.
//
//  Programmer: Hank Childs  [From Kathleen Bonnell's avtMaterialPlotter]
//  Creation:   December 28, 2000
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 26 15:50:18 PST 2001
//    Changed SetLookupTable* to use unsigned char arrays.
//
//    Kathleen Bonnell, Fri Mar  2 14:41:37 PST 2001 
//    Added method to retrieve lookup table.
//
//    Kathleen Bonnell, Tue Mar 20 15:53:14 PST 2001 
//    Added method to retrieve data range from the input.
//
//    Kathleen Bonnell, Tue Mar 27 08:59:23 PST 2001 
//    Added methods to set line style and point size. 
//
//    Kathleen Bonnell, Mon Apr  2 18:30:33 PDT 2001 
//    Added method to retrieve the original data extents of the input. 
//
//    Kathleen Bonnell, Mon Jun 25 14:33:59 PDT 2001 
//    Added stipplePattern. 
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Added SetLookupTable method. Replace numColors with numLevels,
//    added members useConsantColor and constantColor.  Replaced 
//    SetLookupTableWithMultiColor with MultiColorOn and 
//    SetLookupTableWithSingleColor with SingleColorOn.
//    
//    Kathleen Bonnell, Mon Sep 24 08:27:42 PDT 2001
//    No longer use vtkLookupTable for coloring,  use ColorAttributeList
//    instead.  Added/removed related methods.  Store the entire levels
//    list in levelNames.  Store the currently used levels in 
//    labelsForColorMapping.
//
//    Kathleen Bonnell, Wed Sep 11 09:01:37 PDT 2002 
//    Add member labelColorMap, which allows labels to be mapped consistently
//    to their corresponding color index. Added set method, and flag to specify
//    whether the map should be used.
//
//    Brad Whitlock, Fri Nov 15 10:05:38 PDT 2002
//    I changed the map type to LevelColorMap.
//
// ****************************************************************************

class PLOTTER_API avtLevelsMapper : public avtMapper
{
  public:
                              avtLevelsMapper();
    virtual                  ~avtLevelsMapper();

    void                      SetLineWidth(_LineWidth lw);
    void                      SetLineStyle(_LineStyle ls);
    void                      SetPointSize(float);
    void                      GetOriginalDataRange(double &, double &);

    void                      SetColors(const ColorAttributeList &c);
    void                      GetLevelColor(const int, float[4]);
    void                      GetLevelColor(const std::string &, float[4]);

    void                      SetLabelColorMap(const LevelColorMap &);

  protected:
    _LineWidth                lineWidth;
    _LineStyle                lineStyle;
    std::vector<std::string>  labelsForColorMapping;
    std::vector<std::string>  levelNames;
    ColorAttributeList        cal;
    LevelColorMap             labelColorMap; 
    bool                      useColorMap;
 
    virtual void              CustomizeMappers(void);
    virtual void              SetLabels(std::vector<std::string> &, bool);
};

#endif


