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

// Staic storage for Macro keys and their default formats
typedef struct _keyfmt {
    char const *key;
    char const *fmt;
} keyfmt_t;
static std::vector<keyfmt_t> keysAndFmts;

typedef struct _annotdata {
    double timeScale;
    double timeOffset;
} annotdata_t;

// ****************************************************************************
// static function: hasKeyMatch
//
// Purpose: Given a candidate key in the annotation string, march through the
// known keys finding the first that matches.
//
// Mark C. Miller, Thu Jun 23 10:07:38 PDT 2022
// ****************************************************************************

static int
hasKeyMatch(char const *fmtStr, int idx)
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

// ****************************************************************************
// MACRO: TEXT_MACRO
//
// Two blocks of code that either adds the associated key to keysAndFmts or
// renders the associated key to a string. The do-while design is to enforce
// semicolon termination of the macro instances.
//
// Mark C. Miller, Thu Jun 23 10:05:58 PDT 2022
// ****************************************************************************

#define TEXT_MACRO(NAME, FMT, VAL)                     \
    do                                                 \
    {                                                  \
        if (!initialized)                              \
        {                                              \
            keyfmt_t x = {#NAME, #FMT};                \
            keysAndFmts.push_back(x);                  \
        }                                              \
        else if (!strncmp(key, #NAME, sizeof(#NAME)))  \
        {                                              \
            snprintf(rv, rvsize, fmt, VAL);            \
            return;                                    \
        }                                              \
   } while (false) 

// ****************************************************************************
// static function: processMacro
//
// Purpose: Two wholly disparate purposes. One is to initialize keysAndFmts
// vector of known macro keys and their default sprintf formats. The other is
// to render a specific key according to a specified format. The main design
// goal is to enable any developer to add new keys EASILY by simply adding
// an instance of TEXT_MACRO. This leads to a somewhat bizarre coding though.
//
// The first function is performed only once at startup.
//
// Mark C. Miller, Thu Jun 23 09:37:47 PDT 2022
//
// Modifications
//   Mark C. Miller, Thu Oct  5 15:32:11 PDT 2023
//   Added annotdata_t argument for handling any additional annotation data.
//   Also deref cda here instead of inside TEXT_MACRO.
//   Handled case where $time macro may need timeScale and timeOffset.
//
// ****************************************************************************

static void
processMacro(char *rv, size_t rvsize=0, char const *key=0, char const *fmt=0,
    avtDataAttributes const *cda=0, annotdata_t const *ad=0)
{
    static bool initialized = false;

    if (initialized && rv==0) return;

    TEXT_MACRO(time, %g, (cda->GetTime()*ad->timeScale+ad->timeOffset));
    TEXT_MACRO(cycle, %d, cda->GetCycle());
    TEXT_MACRO(index, %d, cda->GetTimeIndex());
    TEXT_MACRO(numstates, %d, cda->GetNumStates());
    TEXT_MACRO(dbcomment, %s, cda->GetCommentInDB().c_str());
    TEXT_MACRO(lod, %z, cda->GetLevelsOfDetail());
    TEXT_MACRO(vardim, %d, cda->GetVariableDimension());
    TEXT_MACRO(numvar, %d, cda->GetNumberOfVariables());
    TEXT_MACRO(topodim, %d, cda->GetTopologicalDimension());
    TEXT_MACRO(spatialdim, %d, cda->GetSpatialDimension());
    TEXT_MACRO(varname, %s, cda->GetVariableName().c_str());
    TEXT_MACRO(varunits, %s, cda->GetVariableUnits().c_str());
    TEXT_MACRO(meshname, %s, cda->GetMeshname().c_str());
    TEXT_MACRO(filename, %s, cda->GetFilename().c_str());
    TEXT_MACRO(fulldbname, %s, cda->GetFullDBName().c_str());
    TEXT_MACRO(xunits, %s, cda->GetXUnits().c_str());
    TEXT_MACRO(yunits, %s, cda->GetYUnits().c_str());
    TEXT_MACRO(zunits, %s, cda->GetZUnits().c_str());
    TEXT_MACRO(xlabel, %s, cda->GetXLabel().c_str());
    TEXT_MACRO(ylabel, %s, cda->GetYLabel().c_str());
    TEXT_MACRO(zlabel, %s, cda->GetZLabel().c_str());

    initialized = true;
}

// ****************************************************************************
// static function: getKeyString
//
// Purpose: for any $Xxx matching a macro key, obtain also the format, if
// given, or use the default format and then render the value of the macro key
// to a string. The string must be used *immediately* upon return because it
// is returned on a local static var.
//
// Mark C. Miller, Thu Jun 23 09:36:41 PDT 2022
//
// Modifications
//   Mark C. Miller, Thu Oct  5 15:31:36 PDT 2023
//   Added annotdata_t arg for passing additional annotation data.
//
// ****************************************************************************

static char const *
getKeyString(char const *fmtStr, int inlen, int idx, int kid,
avtDataAttributes const *cda, annotdata_t const *ad)
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

    processMacro(retval, sizeof(retval), key, fmt, cda, ad);

    return retval;
}

// ****************************************************************************
// Method: avtAnnotationWithTextColleague::CreateAnnotationString
//
// Purpose: Does the work of creating the string to be used in the annotation.
// The key effort is handling any annotation "macros" such as $time or $cycle.
//
// Mark C. Miller, Thu Jun 23 09:34:18 PDT 2022
//
// Design goals...
//    - handle $Xxx anywhere in string
//    - handle multiple occurences of $Xxx
//    - handle $Xxx and $Yyy in same string
//    - handle $Xxx%Fmt$
//    - make it easy to add more macros (e.g. TEXT_MACRO macro)
//
// Algorithm...
//    1. Find $Xxx in formatString
//      - if present look also for %fmt and have defaults otherwise
//    2. replace $Xxx or $Xxx%fmt$ with intended text
//    3. loop until all $Xxx have been replaced.
//
//    - iterate through formatString look for '$'
//    - if find '$' check if subsequent chars match a key with optional format.
//       - if key matches, add replacement to output.
//       - if $Xxx doesn't match any key, copy to the output string verbatim.
//    - if '$' not found, copy the char to the output string.
//
// Caller must delete what is returned
//
// Modifications
//   Mark C. Miller, Thu Oct  5 15:33:40 PDT 2023
//   Added logic to capture timeScale,timeOffset and pass into key string
//   processing methods.
//
// ****************************************************************************

char *
avtAnnotationWithTextColleague::CreateAnnotationString(const char *formatString)
{
    std::string rv;

    annotdata_t ad = {timeScale, timeOffset};

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
                rv += getKeyString(formatString, inlen, i, keyMatch, currentDataAttributes, &ad);
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
