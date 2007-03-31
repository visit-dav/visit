#include "Vector.h"

#include <stdio.h>


ostream &operator<<(ostream& out, const Vector &r)
{
    out << "<" << r.x << "," << r.y << "," << r.z << ">";
    return out;
}

const char*
Vector::getAsText()
{
    sprintf(text,"%.2f %.2f %.2f",x,y,z);
    return text;
}

void
Vector::setAsText(const char *s)
{
    sprintf(text,"%s",s);
    sscanf(text,"%f %f %f",&x,&y,&z);
}
