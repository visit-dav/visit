#include <avtToolInterface.h>
#include <AttributeSubject.h>

avtToolInterfaceCallback avtToolInterface::callback = 0;

avtToolInterface::avtToolInterface(const VisWindow *v)
{
    visWindow = v;
    atts = NULL;
}

avtToolInterface::~avtToolInterface()
{
    if(atts != NULL)
        delete atts;
}

