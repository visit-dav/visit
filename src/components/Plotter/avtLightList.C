#include <avtLightList.h>
#include <LightAttributes.h>
#include <ColorAttribute.h>
#include <BadIndexException.h>

// ****************************************************************************
// Method: avtLight::avtLight
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 22:55:53 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Aug 20 09:22:58 PDT 2002
//   I renamed some of the light types.
//
// ****************************************************************************

avtLight::avtLight()
{
    type = CAMERA;
    direction[0] = 0.;
    direction[1] = 0.;
    direction[2] = -1.;
    color[0] = 1.;
    color[1] = 1.;
    color[2] = 1.;
    brightness = 1.;
    enabled = false;
}

avtLight::avtLight(const avtLight &l)
{
    type = l.type;
    direction[0] = l.direction[0];
    direction[1] = l.direction[1];
    direction[2] = l.direction[2];
    color[0] = l.color[0];
    color[1] = l.color[1];
    color[2] = l.color[2];
    brightness = l.brightness;
    enabled = l.enabled;
}

// ****************************************************************************
// Method: avtLight::operator =
//
// Purpose: 
//   Assignment operator.
//
// Arguments:
//   l : the avtLightList to copy.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 22:56:24 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtLight::operator = (const avtLight &l)
{
    type = l.type;
    direction[0] = l.direction[0];
    direction[1] = l.direction[1];
    direction[2] = l.direction[2];
    color[0] = l.color[0];
    color[1] = l.color[1];
    color[2] = l.color[2];
    brightness = l.brightness;
    enabled = l.enabled;
}

//
// avtLightList methods.
//


// ****************************************************************************
// Method: avtLightList::avtLightList
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 22:57:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

avtLightList::avtLightList()
{
    lights[0].enabled = true;
}

avtLightList::avtLightList(const avtLightList &l)
{
    for(int i = 0; i < NumLights(); ++i)
        lights[i] = l.lights[i];
}

avtLightList::avtLightList(const LightList &ll)
{
    for(int i = 0; i < NumLights() && i < ll.NumLights(); ++i)
    {
        const LightAttributes &l = ll.GetLight(i);
        lights[i].type = (avtLightType)l.GetType();
        lights[i].direction[0] = l.GetDirection()[0];
        lights[i].direction[1] = l.GetDirection()[1];
        lights[i].direction[2] = l.GetDirection()[2];
        lights[i].brightness = l.GetBrightness();
        lights[i].color[0] = double(l.GetColor().Red()) / 255.;
        lights[i].color[1] = double(l.GetColor().Green()) / 255.;
        lights[i].color[2] = double(l.GetColor().Blue()) / 255.;
        lights[i].enabled = l.GetEnabledFlag();
    }
}

// ****************************************************************************
// Method: avtLightList::operator =
//
// Purpose: 
//   Assignment operator.
//
// Arguments:
//   l : The avtLightList to copy.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 22:57:38 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtLightList::operator = (const avtLightList &l)
{
    for(int i = 0; i < NumLights(); ++i)
        lights[i] = l.lights[i];
}

// ****************************************************************************
// Method: avtLightList::MakeAttributes
//
// Purpose: 
//   Constructs and returns an AttributeSubject version of the light list.
//
// Returns:    An AttributeSubject version of the light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 22:58:01 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 20 13:54:19 PST 2003
//   Made it work with the regenerated LightAttributes.
//
// ****************************************************************************

LightList *
avtLightList::MakeAttributes() const
{
    LightList *ret = new LightList;

    for(int i = 0; i < NumLights() && i < ret->NumLights(); ++i)
    {
        LightAttributes &l = ret->GetLight(i);
        l.SetType(LightAttributes::LightType(int(lights[i].type)));
        l.SetDirection(lights[i].direction);
        l.SetBrightness(lights[i].brightness);
        ColorAttribute C;
        C.SetRed(int(lights[i].color[0] * 255));
        C.SetGreen(int(lights[i].color[1] * 255));
        C.SetBlue(int(lights[i].color[2] * 255));
        l.SetColor(C);
        l.SetEnabledFlag(lights[i].enabled);
    }

    return ret;
}


// ****************************************************************************
// Method: avtLight::Light
//
// Purpose: 
//   Returns the specified light.
//
// Arguments:
//   i  The index of the light to return.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 13, 2002 
//
// ****************************************************************************

const avtLight & 
avtLightList::Light(int i) const
{
    if (i < 0  || i > 7)
    {
        EXCEPTION2(BadIndexException, i, 8);
    }
    return lights[i];
}
