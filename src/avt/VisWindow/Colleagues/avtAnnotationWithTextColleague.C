// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtAnnotationWithTextColleague.h>
#include <avtDataAttributes.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <AnnotationObject.h>

#include <vtkRenderer.h>
#include <vtkVisItTextActor.h>
#include <vtkTextProperty.h>

#include <memory>

std::unique_ptr<avtDataAttributes> avtAnnotationWithTextColleague::initialDataAttributes = std::unique_ptr<avtDataAttributes>(new avtDataAttributes);

// ****************************************************************************
// Method: avtAnnotationWithTextColleague::avtAnnotationWithText
//
// Purpose: 
//   Constructor for the avtAnnotationWithText class.
//
// Arguments:
//   m : The vis window proxy.
//
// Programmer: Mark C. Miller, Wed Jun 15 15:30:22 PDT 2022
// ****************************************************************************

avtAnnotationWithTextColleague::avtAnnotationWithTextColleague(VisWindowColleagueProxy &m) 
    : avtAnnotationColleague(m)
{
    textFormatString = new char[1];
    textFormatString[0] = '\0';
    textString = new char[1];
    textString[0] = '\0';
    currentDataAttributes = new avtDataAttributes;
    currentDataAttributes->Copy(*initialDataAttributes);
}

// ****************************************************************************
// Method: avtAnnotationWithTextColleague::~avtAnnotationWithText
//
// Purpose: 
//   Destructor for the avtAnnotationWithText class.
// 
// Mark C. Miller, Wed Jun 15 15:31:15 PDT 2022
// ****************************************************************************

avtAnnotationWithTextColleague::~avtAnnotationWithTextColleague()
{
    delete [] textString;
    delete [] textFormatString;
    delete currentDataAttributes;
}

// ****************************************************************************
// Method: avtAnnotationWithTextColleague::UpdatePlotList
//
// Purpose: 
//   This method is called when the plot list changes. Its job is to make sure
//   that the time slider always shows the right time.
//
// Arguments:
//   lst : The plot list.
//
// Mark C. Miller, Wed Jun 15 15:31:32 PDT 2022
// ****************************************************************************

void
avtAnnotationWithTextColleague::UpdatePlotList(std::vector<avtActor_p> &lst)
{
    if(lst.size() > 0 && textFormatString != 0)
    {
        // update current copy of data attributes
        currentDataAttributes->Copy(lst[0]->GetBehavior()->GetInfo().GetAttributes());

        // update initial data attributes to current
        initialDataAttributes->Copy(*currentDataAttributes);
    }
}

// handle $Xxx anywhere in string
// handle multiple occurences of $Xxx
// handle $Xxx and $Yyy in same string
// handle $Xxx%Fmt$

//
// 1. Find $key in formatString
//   - if present look also for %fmt and have defaults otherwise
// 2. replace $key%fmt$ with intended text
// 3. loop until all $key have been replaced.
//
// Algorithm...
//    - iterate through formatString look for '$'
//    - if find '$' check if subsequent chars up to some delimters match a key with optional format string
//       - if key matches, add replacement string to output replacing (lengthening or shortening) what is there.
//       - if $<txt> doesn't match any key, copy the text to the output string verbatim
//    - if '$' not found, copy the char to the output string
//

// Macro keys and their default formats
typedef struct _keyfmt {
    char const *key;
    char const *fmt;
} keyfmt_t;

static std::vector<keyfmt_t> keysAndFmts;

static int hasKeyMatch(char const *fmtStr, int idx)
{
    int const nkeys = (int) keysAndFmts.size();
    for (int ki = 0; ki < nkeys; ki++)
    {
        char const *key = keysAndFmts[ki].key;
        if (!strncmp(&fmtStr[idx], key, strlen(key)))
            return ki;
    }
    return -1;
}

#define TEXT_MACRO(NAME, FMT, GETTER)                  \
    do                                                 \
    {                                                  \
        if (!initialized)                              \
        {                                              \
            keyfmt_t x = {#NAME, #FMT};                \
            keysAndFmts.push_back(x);                  \
        }                                              \
        else if (!strncmp(key, #NAME, sizeof(#NAME)))  \
        {                                              \
            snprintf(rv, rvsize, fmt, cda->GETTER);    \
            return;                                    \
        }                                              \
   } while (false) 

static void processMacro(char *rv, size_t rvsize=0, char const *key=0, char const *fmt=0, avtDataAttributes const *cda=0)
{
    static bool initialized = false;

    if (initialized && rv==0) return;

    TEXT_MACRO(time, %g, GetTime());
    TEXT_MACRO(cycle, %d, GetCycle());
    TEXT_MACRO(index, %d, GetTimeIndex());
    TEXT_MACRO(numstates, %d, GetNumStates());
    TEXT_MACRO(dbcomment, %s, GetCommentInDB().c_str());
    TEXT_MACRO(lod, %z, GetLevelsOfDetail());
    TEXT_MACRO(vardim, %d, GetVariableDimension());
    TEXT_MACRO(numvar, %d, GetNumberOfVariables());
    TEXT_MACRO(topodim, %d, GetTopologicalDimension());
    TEXT_MACRO(spatialdim, %d, GetSpatialDimension());
    TEXT_MACRO(varname, %s, GetVariableName().c_str());
    TEXT_MACRO(varunits, %s, GetVariableUnits().c_str());
    TEXT_MACRO(meshname, %s, GetMeshname().c_str());
    TEXT_MACRO(filename, %s, GetFilename().c_str());
    TEXT_MACRO(fulldbname, %s, GetFullDBName().c_str());
    TEXT_MACRO(xunits, %s, GetXUnits().c_str());
    TEXT_MACRO(yunits, %s, GetYUnits().c_str());
    TEXT_MACRO(zunits, %s, GetZUnits().c_str());
    TEXT_MACRO(xlabel, %s, GetXLabel().c_str());
    TEXT_MACRO(ylabel, %s, GetYLabel().c_str());
    TEXT_MACRO(zlabel, %s, GetZLabel().c_str());

    initialized = true;
}


static char const *getKeyString(char const *fmtStr, int inlen, int idx, int kid, avtDataAttributes const *cda)
{
    static char retval[256];
    char const *key = keysAndFmts[kid].key;
    char const *defaultFmt = keysAndFmts[kid].fmt;
    char fmt[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    // Use default printf format or whatever was specified in the string
    int klen = strlen(key);
    if (idx + klen < (inlen-1) && fmtStr[idx+klen] == '%' && fmtStr[idx+klen+1] != '%')
    {
        strncpy(fmt, &fmtStr[idx+klen], strcspn(&fmtStr[idx+klen], "$"));
    }
    else
    {
        strcpy(fmt, defaultFmt); // (e.g. fmt = "%g")
    }

    processMacro(retval, sizeof(retval), key, fmt, cda);

    return retval;
}

// Caller must delete what is returned
char *
avtAnnotationWithTextColleague::CreateAnnotationString(const char *formatString)
{
    std::string rv;

    processMacro(0); // initialize keysAndFmts;

    int inlen = (int) strlen(formatString);
    for (int i = 0; i < inlen; i++)
    {
        if (formatString[i] == '$')
        {
            
            int keyMatch = hasKeyMatch(formatString, i+1);
            if (keyMatch != -1)
            {
                i++; // move to next char after '$'                
                rv += getKeyString(formatString, inlen, i, keyMatch, currentDataAttributes);
                i += (int) strlen(keysAndFmts[keyMatch].key); // skip over key
                if (formatString[i] == '%')
                {
                    while (i < inlen && formatString[i] != '$')
                        i++;
                }
                else
                {
                    i--;
                }
            }
            else
            {
                rv += formatString[i];
            }
        }
        else
        {
            rv += formatString[i];
        }
    } 

    return strdup(rv.c_str()); // caller must free
}
