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

#define TIME_IDENTIFIER "$time"
#define CYCLE_IDENTIFIER "$cycle"
#define INDEX_IDENTIFIER "$index"

// handle $Xxx anywhere in string
// handle multiple occurences of $Xxx
// handle $Xxx and $Yyy in same string
// handle $Xxx%Fmt

//
// 1. Find $key in formatString
//   - if present look also for %fmt and have defaults otherwise
// 2. replace $key%fmt with intended text
// 3. loop until all $key have been replaced.
//
// Algorithm...
//    - iterate through formatString look for '$'
//    - if find '$' check if subsequent chars up to some delimters match a key with optional format string
//       - if key matches, add replacement string to output replacing (lengthening or shortening) what is there.
//       - if $<txt> doesn't match any key, copy the text to the output string verbatim
//    - if '$' not found, copy the char to the output string
//

// Keys and their default formats
typedef struct _keyfmt {
    char const *key;
    char const *typ;
    char const *fmt;
} keyfmt_t;

static keyfmt_t keysAndFmts[] = {
   "time", "float", "%g",
   "cycle", "int", "%d",
   "index", "int", "%d"
};


int hasKeyMatch(char const *fmtStr, int idx)
{
    int const nkeys = (int) (sizeof(keysAndFmts)/sizeof(keysAndFmts[0]));
    for (int ki = 0; ki < nkeys; ki++)
    {
        char const *key = keysAndFmts[ki].key;
        if (!strncmp(&fmtStr[idx], key, strlen(key)))
            return ki;
    }
    return -1;
}

char const *getKeyString(char const *fmtStr, int inlen, int idx, int kid)
{
    char const *key = keysAndFmts[kid].key;
    char const *typ = keysAndFmts[kid].typ;
    char const *defaultFmt = keysAndFmts[kid].fmt;
    char fmt[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0;

    // Use default printf format or whatever was specified in the string
    int klen = strlen(key);
    if (idx + klen < inlen && fmtStr[idx+klen] == '%')
        strncpy(fmt, &fmtStr[idx+klen], strspn(&fmtStr[idx+klen], '0123456789-+ #.*lLhjztdiouxXfFeEgGaAn%')
    else
        strcpy(fmt, keysAndFmts[kid].fmt);


   
}

// Caller must delete what is returned
char *
avtAnnotationWithTextColleague::CreateAnnotationString(const char *formatString)
{
    std::string rv;

    int inlen = (int) strlen(formatString);
    for (int i = 0; i < inlen; i++)
    {
        if ((formatString[i] == '$') && i < inlen-1)
        {
            
            keyMatch = hasKeyMatch(formatString, i+1))
            if (keyMatch != -1)
                rv += keyReplace(formatString, inlen, i, keyMatch);
            else
                rv += formatString[i]
        }
        else
            rv += formatString[i]
    } 
    return strdup(rv.c_str());


#if 0
    size_t len = strlen(formatString);
    std::string fmtStr(formatString);
    std::string::size_type pos;
    if((pos=fmtStr.find(TIME_IDENTIFIER)) != std::string::npos)
    {
        size_t tlen = strlen(TIME_IDENTIFIER);
        std::string left(fmtStr.substr(0, pos));
        std::string right(fmtStr.substr(pos + tlen, fmtStr.size() - pos - tlen));
        char tmp[100];
        snprintf(tmp, 100, "%g", currentDataAttributes->GetTime());
        len = left.size() + strlen(tmp) + right.size() + 1;
        retval = new char[len + 1];
        snprintf(retval, len, "%s%s%s", left.c_str(), tmp, right.c_str());
    }
    else if((pos=fmtStr.find(CYCLE_IDENTIFIER)) != std::string::npos)
    {
        size_t tlen = strlen(CYCLE_IDENTIFIER);
        std::string left(fmtStr.substr(0, pos));
        std::string right(fmtStr.substr(pos + tlen, fmtStr.size() - pos - tlen));
        char tmp[100];
        snprintf(tmp, 100, "%d", currentDataAttributes->GetCycle());
        len = left.size() + strlen(tmp) + right.size() + 1;
        retval = new char[len + 1];
        snprintf(retval, len, "%s%s%s", left.c_str(), tmp, right.c_str());
    }
    else if((pos=fmtStr.find(INDEX_IDENTIFIER)) != std::string::npos)
    {
        size_t tlen = strlen(INDEX_IDENTIFIER);
        std::string left(fmtStr.substr(0, pos));
        std::string right(fmtStr.substr(pos + tlen, fmtStr.size() - pos - tlen));
        char tmp[100];
        snprintf(tmp, 100, "%d", currentDataAttributes->GetTimeIndex());
        len = left.size() + strlen(tmp) + right.size() + 1;
        retval = new char[len + 1];
        snprintf(retval, len, "%s%s%s", left.c_str(), tmp, right.c_str());
    }
    else
    {
        retval = new char[len + 1];
        strcpy(retval, formatString);
    }

    return retval;


#if 0
    char *retval = new char[100]();
    if (!strcmp(formatString, "$time"))
        snprintf(retval, 100, "%g", currentDataAttributes->GetTime());
    else if (!strcmp(formatString, "$cycle"))
        snprintf(retval, 100, "%d", currentDataAttributes->GetCycle());
    else if (!strcmp(formatString, "$index"))
        snprintf(retval, 100, "%d", currentDataAttributes->GetTimeIndex());
    else
        snprintf(retval, 100, "%s", formatString);
    return retval;
#endif
#endif
}
