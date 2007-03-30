#ifndef AVT_LIGHT_LIST_H
#define AVT_LIGHT_LIST_H
#include <plotter_exports.h>
#include <LightList.h>

typedef enum {AMBIENT=0, OBJECT=1, CAMERA=2} avtLightType;

struct PLOTTER_API avtLight
{
    avtLight();
    avtLight(const avtLight &);
    ~avtLight() { ; };
    void operator = (const avtLight &);

    avtLightType type;
    double       direction[3];
    double       color[3];
    double       brightness;
    bool         enabled;
};

// ****************************************************************************
// Class: avtLightList
//
// Purpose:
//   This class contains a list of lights that are used when rendering.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 22:54:38 PST 2001
//
// Modifications:
//   
// ****************************************************************************

class PLOTTER_API avtLightList
{
  public:
    avtLightList();
    avtLightList(const avtLightList &);
    avtLightList(const LightList &);
    ~avtLightList() { ; };
    void operator = (const avtLightList &);

    LightList *MakeAttributes() const;
    int NumLights() const { return 8; };
    const avtLight &Light(int) const;
  private:
    avtLight lights[8];
};

#endif
