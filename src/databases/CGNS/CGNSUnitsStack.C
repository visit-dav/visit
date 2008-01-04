/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#define CGNSUnitsStackMaxDepth 10

static const char *MassUnitsNames[] = {
    "MassUnitsNull", "MassUnitsUserDefined",
    "Kilogram", "Gram", "Slug", "PoundMass"
};
static const char *LengthUnitsNames[] = {
    "LengthUnitsNull", "LengthUnitsUserDefined",
    "Meter", "Centimeter", "Millimeter", "Foot", "Inch"
};
static const char *TimeUnitsNames[] = {
    "TimeUnitsNull", "TimeUnitsUserDefined", "Second"
};
static const char *TemperatureUnitsNames[] = {
    "TemperatureUnitsNull", "TemperatureUnitsUserDefined",
    "Kelvin", "Celcius", "Rankine", "Fahrenheit"
};
static const char *AngleUnitsNames[] = {
    "AngleUnitsNull", "AngleUnitsUserDefined", "Degree", "Radian"
};

// ****************************************************************************
// Class: CGNSUnitsStack
//
// Purpose: 
//   Keeps track of the units to use for fields as we traverse the CGNS
//   tree.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 1 17:11:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class CGNSUnitsStack
{
public:
    CGNSUnitsStack()
    {
        top = -1;
        for(int i = 0; i < CGNSUnitsStackMaxDepth; ++i)
        {
            massU[i] = MassUnitsNull;        
            lengthU[i] = LengthUnitsNull;      
            timeU[i] = TimeUnitsNull;        
            tempU[i] = TemperatureUnitsNull; 
            angleU[i] = AngleUnitsNull;       
        }
    }

    ~CGNSUnitsStack()
    {
    }

    bool PushUnits(int fn, int base)
    {
        bool err = true;
        if(cg_goto(fn, base, "end") == CG_OK)
            err = ReadUnits();
        return !err;
    }

    bool PushUnits(int fn, int base, int zone)
    {
        bool err = true;
        if(cg_goto(fn, base, "Zone_t", zone, "end") == CG_OK)
            err = ReadUnits();
        return !err;
    }

    bool PushUnits(int fn, int base, int zone, int sol)
    {
        bool err = true;
        if(cg_goto(fn, base, "Zone_t", zone, "FlowSolution_t",
            sol, "end") == CG_OK)
            err = ReadUnits();
        return !err;
    }

    bool PushUnits(int fn, int base, int zone, int sol,  int field)
    {
        bool err = true;
        if(cg_goto(fn, base, "Zone_t", zone, "FlowSolution_t",
            sol, "DataArray_t", field, "end") == CG_OK)
            err = ReadUnits();
        return !err;
    }

    bool PopUnits()
    {
        bool retval;
        if((retval = top >= 0) == true)
            --top;
        return retval;
    }

    bool GetUnits(std::string &units)
    {
        bool err = true;

        if(top >= 0)
        {
            int numexp = 0;
            DataType_t dt;
            if(cg_nexponents(&numexp) == CG_OK &&
               cg_exponents_info(&dt) == CG_OK &&
               numexp > 0)
            {
                stringVector N, D;
                char tmp[100];

#define MY_ABS(V, Z) (((V)<(Z)) ? -(V) : (V))
#define CHAR_ABS(V, Z) (int)(((V)<(Z)) ? -(V) : (V))

#define POPULATE_UNITS(T, Z, ONE, ABS_FUNC, FORMATSTR) \
                    {\
                        T *arr = new T[numexp];\
                        if(cg_exponents_read((void *)arr) == CG_OK)\
                        {\
                            for(int idx = 0; idx < 5; ++idx)\
                            {\
                                const char *uname = 0;\
                                if(idx == 0)\
                                    uname = MassUnitsNames[int(massU[top])];\
                                else if(idx == 1)\
                                    uname = LengthUnitsNames[int(lengthU[top])];\
                                else if(idx == 2)\
                                    uname = TimeUnitsNames[int(timeU[top])];\
                                else if(idx == 3)\
                                    uname = TemperatureUnitsNames[int(tempU[top])];\
                                else\
                                    uname = AngleUnitsNames[int(angleU[top])];\
                                if(arr[idx] != Z)\
                                {\
                                    if(arr[idx] == ONE)\
                                        SNPRINTF(tmp,100,"%s",uname);\
                                    else\
                                        SNPRINTF(tmp, 100, FORMATSTR, uname,\
                                                 ABS_FUNC(arr[idx], Z));\
                                    char *c = strchr(tmp, '^');\
                                    if(c != 0 && strcmp(c, "^1") == 0)\
                                        c[0] = '\0';\
                                    if(arr[idx] < Z)\
                                        D.push_back(tmp);\
                                    else\
                                        N.push_back(tmp);\
                                }\
                            }\
                        }\
                        delete [] arr;\
                    }

                switch(dt)
                {
                case RealSingle:
                    POPULATE_UNITS(float, 0.f, 1.f, MY_ABS, "%s^%g")
                    break;
                case RealDouble:
                    POPULATE_UNITS(double, 0., 1., MY_ABS, "%s^%g")
                    break;
                case Integer:
                    POPULATE_UNITS(int, 0, 1, MY_ABS, "%s^%d")
                    break;
                case Character:
                    POPULATE_UNITS(char, 0, 1, CHAR_ABS, "%s^%d")
                    break;
                }

                // Create the numerator string.
                std::string NS, DS;
                if(N.size() == 1)
                    NS = N[0];
                else if(N.size() > 1)
                {
                    NS = "(";
                    for(int u = 0; u < N.size(); ++u)
                    {
                        if(u > 0)
                            NS += " * ";
                        NS += N[u];
                    }
                    NS += ")";
                }

                // Create the denominator string.
                if(D.size() == 1)
                    DS = D[0];
                else if(D.size() > 1)
                {
                    DS = "(";
                    for(int u = 0; u < D.size(); ++u)
                    {
                        if(u > 0)
                            DS += " * ";
                        DS += D[u];
                    }
                    DS += ")";
                }

                // Assemble the units.
                if(NS.size() == 0 && DS.size() == 0)
                {
                    units = "";
                }
                else if(NS.size() == 0 && DS.size() > 0)
                {
                    units = std::string("1 / ") + DS;
                }
                else if(NS.size() > 0 && DS.size() == 0)
                {
                    units = NS;
                }
                else if(NS.size() > 1 && DS.size() > 1)
                {
                    units = (NS + " / ") + DS;
                }
                err = false;
            }
        }

        return !err;
    }

protected:
    bool ReadUnits()
    {
        bool err = true;
        int i = top + 1;
        int nexp = 0;
        if(i < CGNSUnitsStackMaxDepth &&
           cg_units_read(&massU[i], &lengthU[i], &timeU[i],
                         &tempU[i], &angleU[i]) == CG_OK)
        {
            ++top;
            err = false;
        }

        return err;
    }

    int                top;
    MassUnits_t        massU[CGNSUnitsStackMaxDepth];
    LengthUnits_t      lengthU[CGNSUnitsStackMaxDepth];
    TimeUnits_t        timeU[CGNSUnitsStackMaxDepth];
    TemperatureUnits_t tempU[CGNSUnitsStackMaxDepth];
    AngleUnits_t       angleU[CGNSUnitsStackMaxDepth];
};


