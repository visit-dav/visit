#include "avtVector.h"

#include <stdio.h>

ostream &operator<<(ostream& out, const avtVector &r)
{
    out << "<" << r.x << "," << r.y << "," << r.z << ">";
    return out;
}

const char*
avtVector::getAsText()
{
    sprintf(text,"%.2f %.2f %.2f",x,y,z);
    return text;
}

void
avtVector::setAsText(const char *s)
{
    sprintf(text,"%s",s);
    sscanf(text,"%lf %lf %lf",&x,&y,&z);
}
