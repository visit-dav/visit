#include <ConfigManager.h>
#include <DataNode.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#endif
#include <visit-config.h>

// ****************************************************************************
// Method: ConfigManager::ConfigManager
//
// Purpose: 
//   Constructor for the ConfigManager class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:17:00 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ConfigManager::ConfigManager()
{
    fp = 0;
    putback = false;
    putbackChar = 0;
}

// ****************************************************************************
// Method: ConfigManager::~ConfigManager
//
// Purpose: 
//   Destructor for the ConfigManager class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:17:23 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ConfigManager::~ConfigManager()
{
    if(fp != 0)
        fclose(fp);
}

// ****************************************************************************
// Method: ConfigManager::WriteBack
//
// Purpose: 
//   This is a debugging method writes the DataNode tree specified by
//   root to a file called WRITEBACK. It is used to test the validity
//   of a tree that has been read from a file.
//
// Arguments:
//   root : The DataNode tree to write to the file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:17:58 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Feb 23 16:29:41 PST 2004
//   Write text file.
//
// ****************************************************************************

void
ConfigManager::WriteBack(DataNode *root)
{
    // Try to open the output file.
    if((fp = fopen("WRITEBACK", "wt")) == 0)
        return;

    // Write the output file to stdout for now.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(root);

    // close the file
    fclose(fp);
    fp = 0;
}

// ****************************************************************************
// Method: ConfigManager::WriteIndent
//
// Purpose: 
//   This method writes indentation spaces to the open file.
//
// Arguments:
//   indentLevel : The number of levels to indent.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:19:28 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ConfigManager::WriteIndent(int indentLevel)
{
    for(int i = 0; i < indentLevel; ++i)
        fprintf(fp, "    ");
}

// ****************************************************************************
// Method: ConfigManager::WriteQuotedStringData
//
// Purpose: 
//   Writes the string to the file surrounded by quotes and and quotes that
//   the string had in it are escaped.
//
// Arguments:
//   str : The string to write to the file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 3 16:18:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ConfigManager::WriteQuotedStringData(const std::string &str)
{
    fputc('"', fp);    
    if(str.size() > 0)
    {
        const char *cptr = str.c_str();
        for(int i = 0; i < str.size(); ++i)
        {
            // Add escape characters.
            if(cptr[i] == '"' || cptr[i] == '\\')
                fputc('\\', fp);
            fputc(cptr[i], fp);
        }
    }

    fputc('"', fp);
    fputc(' ', fp);
}

// ****************************************************************************
// Method: ConfigManager::WriteData
//
// Purpose: 
//   This method writes out the data contained in the specified
//   DataNode to the open file as ASCII.
//
// Arguments:
//   node : The node whose data will be written.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:20:21 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 30 14:37:21 PST 2001
//   Added unsigned char cases.
//
//   Brad Whitlock, Fri Oct 3 16:00:21 PST 2003
//   I made string vectors and arrays quote their strings so they read in
//   correctly if the strings contain spaces.
//
// ****************************************************************************

void
ConfigManager::WriteData(DataNode *node)
{
    int i;

    switch(node->GetNodeType())
    {
    case CHAR_NODE:
        fprintf(fp, "%c", node->AsChar());
        break;
    case UNSIGNED_CHAR_NODE:
        fprintf(fp, "%d", (int)node->AsUnsignedChar());
        break;
    case INT_NODE:
        fprintf(fp, "%d", node->AsInt());
        break;
    case LONG_NODE:
        fprintf(fp, "%ld", node->AsLong());
        break;
    case FLOAT_NODE:
        fprintf(fp, "%g", node->AsFloat());
        break;
    case DOUBLE_NODE:
        fprintf(fp, "%g", node->AsDouble());
        break;
    case STRING_NODE:
        fprintf(fp, "%s", node->AsString().c_str());
        break;
    case BOOL_NODE:
        if(node->AsBool())
           fprintf(fp, "true");
        else
           fprintf(fp, "false");
        break;
    case CHAR_ARRAY_NODE:
        { // new scope
            const char *cptr = node->AsCharArray();
            for(i = 0; i < node->GetLength(); ++i)
                fprintf(fp, "%c ", *cptr++);
        }
        break;
    case UNSIGNED_CHAR_ARRAY_NODE:
        { // new scope
            const unsigned char *uptr = node->AsUnsignedCharArray();
            for(i = 0; i < node->GetLength(); ++i)
                fprintf(fp, "%d ", (int)*uptr++);
        }
        break;
    case INT_ARRAY_NODE:
        { // new scope
            const int *iptr = node->AsIntArray();
            for(i = 0; i < node->GetLength(); ++i)
                fprintf(fp, "%d ", *iptr++);
        }
        break;
    case LONG_ARRAY_NODE:
        { // new scope
            const long *lptr = node->AsLongArray();
            for(i = 0; i < node->GetLength(); ++i)
                fprintf(fp, "%ld ", *lptr++);
        }
        break;
    case FLOAT_ARRAY_NODE:
        { // new scope
            const float *fptr = node->AsFloatArray();
            for(i = 0; i < node->GetLength(); ++i)
                fprintf(fp, "%g ", *fptr++);
        }
        break;
    case DOUBLE_ARRAY_NODE:
        { // new scope
            const double *dptr = node->AsDoubleArray();
            for(i = 0; i < node->GetLength(); ++i)
                fprintf(fp, "%g ", *dptr++);
        }
        break;
    case STRING_ARRAY_NODE:
        { // new scope
            const std::string *sptr = node->AsStringArray();
            for(i = 0; i < node->GetLength(); ++i)
                WriteQuotedStringData(*sptr++);
        }
        break;
    case BOOL_ARRAY_NODE:
        { // new scope
            const bool *bptr = node->AsBoolArray();
            for(i = 0; i < node->GetLength(); ++i)
            {
                if(*bptr++)
                    fprintf(fp, "true ");
                else
                    fprintf(fp, "false ");
            }
        }
        break;
    case CHAR_VECTOR_NODE:
       { // new scope
            const charVector &cvec = node->AsCharVector();
            for(i = 0; i < cvec.size(); ++i)
                fprintf(fp, "%c ", cvec[i]);
       }
       break;
    case UNSIGNED_CHAR_VECTOR_NODE:
       { // new scope
            const unsignedCharVector &uvec = node->AsUnsignedCharVector();
            for(i = 0; i < uvec.size(); ++i)
                fprintf(fp, "%d ", (int)uvec[i]);
       }
       break;
    case INT_VECTOR_NODE:
       { // new scope
            const intVector &ivec = node->AsIntVector();
            for(i = 0; i < ivec.size(); ++i)
                fprintf(fp, "%d ", ivec[i]);
       }
       break;
    case LONG_VECTOR_NODE:
       { // new scope
            const longVector &lvec = node->AsLongVector();
            for(i = 0; i < lvec.size(); ++i)
                fprintf(fp, "%ld ", lvec[i]);
       }
       break;
    case FLOAT_VECTOR_NODE:
       { // new scope
            const floatVector &fvec = node->AsFloatVector();
            for(i = 0; i < fvec.size(); ++i)
                fprintf(fp, "%g ", fvec[i]);
       }
       break;
    case DOUBLE_VECTOR_NODE:
       { // new scope
            const doubleVector &dvec = node->AsDoubleVector();
            for(i = 0; i < dvec.size(); ++i)
               fprintf(fp, "%g ", dvec[i]);
       }
       break;
    case STRING_VECTOR_NODE:
       { // new scope
            const stringVector &svec = node->AsStringVector();
            for(i = 0; i < svec.size(); ++i)
                WriteQuotedStringData(svec[i]);
       }
       break;
    default:
        fprintf(stderr, "ConfigManager::WriteData: Unsupported type\n");
    }
}

// ****************************************************************************
// Method: ConfigManager::WriteObject
//
// Purpose: 
//   This method writes out the DataNode tree to the open file. This
//   is the code that actually writes a config file.
//
// Arguments:
//   DataNode    : The root of the tree that is to be written.
//   indentLevel : The level of indentation to use for this part of
//                 the tree.
//
// Note:       This function is recursive!
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:21:48 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 30 14:41:13 PST 2001
//   Added a case for arrays of unsigned characters.
//
//   Brad Whitlock, Thu Jul 3 14:47:24 PST 2003
//   I removed the childObjects kludge.
//
// ****************************************************************************

void
ConfigManager::WriteObject(DataNode *node, int indentLevel)
{
    // Write the beginning tag.
    WriteIndent(indentLevel);
    if(node->GetNodeType() == INTERNAL_NODE)
        fprintf(fp, "<Object name=\"%s\">\n", node->GetKey().c_str());
    else
    {
        // Write out the field tag based on the node type.
        switch(node->GetNodeType())
        {
        case CHAR_ARRAY_NODE:
        case UNSIGNED_CHAR_ARRAY_NODE:
        case INT_ARRAY_NODE:
        case LONG_ARRAY_NODE:
        case FLOAT_ARRAY_NODE:
        case DOUBLE_ARRAY_NODE:
        case STRING_ARRAY_NODE:
        case BOOL_ARRAY_NODE:
            fprintf(fp, "<Field name=\"%s\" type=\"%s\" length=\"%d\">",
                    node->GetKey().c_str(), NodeTypeName(node->GetNodeType()),
                    node->GetLength());
            break;
        default:
            fprintf(fp, "<Field name=\"%s\" type=\"%s\">",
                    node->GetKey().c_str(), NodeTypeName(node->GetNodeType()));
        }
    }

    // Write the children of the node if there are any.
    if(node->GetNodeType() == INTERNAL_NODE)
    {
        if(node->GetNumChildren() > 0)
        {
            DataNode **children = node->GetChildren();
            for(int i = 0; i < node->GetNumChildren(); ++i)
                WriteObject(children[i], indentLevel + 1);
        }
    }
    else
    {
        // We have to write actual values.
        WriteData(node);
    }

    // Write the ending tag.
    if(node->GetNodeType() == INTERNAL_NODE)
    {
        WriteIndent(indentLevel);
        fprintf(fp, "</Object>\n");
    }
    else
    {
        fprintf(fp, "</Field>\n");
    }
}

// ****************************************************************************
// Method: ConfigManager::ReadChar
//
// Purpose: 
//   This method reads a character from the open file. It also can
//   read one character that has been "put back" into the file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:23:34 PST 2000
//
// Modifications:
//   
// ****************************************************************************

char
ConfigManager::ReadChar()
{
    if(putback)
    {
        putback = false;
        return putbackChar;
    }
    else
    {
        char c = 0;

        // Read a character until it is greater than ' ' or we
        // encounter eof.
        do
        {
           c = fgetc(fp);
        } while(c < ' ' && !feof(fp));

        if(feof(fp))
            return 0;
        else
            return c;
    }
}

// ****************************************************************************
// Method: ConfigManager::PutBackChar
//
// Purpose: 
//   Logically puts a character back into the open file.
//
// Arguments:
//   c : The character to put back.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:24:23 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ConfigManager::PutBackChar(char c)
{
    putback = true;
    putbackChar = c;
}

// ****************************************************************************
// Method: ConfigManager::FinishTag
//
// Purpose: 
//   Read until a '>' character or eof is encountered.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:29:57 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ConfigManager::FinishTag()
{
    char c;
    for(int i = 0; !feof(fp) && ((c = ReadChar()) != '>'); ++i);

    if(c != '>')
        PutBackChar(c);
}

// ****************************************************************************
// Method: ConfigManager::ReadStringVector
//
// Purpose: 
//   Reads a vector of strings from the open file until a termination character
//   is encountered.
//
// Arguments:
//   termChar : That character at which to stop reading.
//
// Returns:    A string vector.
// 
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 09:39:11 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Oct 3 17:12:07 PST 2003
//   Added crude character escaping.
//
// ****************************************************************************

stringVector
ConfigManager::ReadStringVector(char termChar)
{
    stringVector retval;

    std::string tempString;
    char c;
    bool reading = false, keepgoing = true;
    bool quoted = false;
    bool escaped = false;

    while(keepgoing)
    {
        c = ReadChar();
        keepgoing = (!feof(fp) && c != termChar);

        if(c == ' ')
        {
            if(quoted)
            {
                // Add to the current string.
                tempString += c;
                reading = true;
            }
            else if(reading)
            {
                retval.push_back(tempString);
                tempString = "";
                reading = false;
            }
        }
        else if(c == '\\')
        {
            if(escaped)
            {
                tempString += c;
                escaped = false;
            }
            else
                escaped = true;
        }
        else if(c == '"')
        {
            if(escaped)
            {
                escaped = false;
                tempString += c;
                reading = true;
            }
            else
            {
                quoted = !quoted;
                // Add to the current string.
                tempString += c;
                reading = true;
            }
        }
        else if(c != '\t' && c != '<' && c != '>')
        {
            if(escaped)
            {
                tempString += '\\';
                escaped = false;
            }

            // Add to the current string.
            tempString += c;
            reading = true;
        }
        else if(reading)
        {
            if(escaped)
            {
                tempString += '\\';
                escaped = false;
            }
            retval.push_back(tempString);
            tempString = "";
            reading = false;
        }
    }
 
    return retval;
}

// ****************************************************************************
// Method: ConfigManager::RemoveLeadAndTailQuotes
//
// Purpose: 
//   Removes leading and trailing quotes in all of the strings in the vector.
//
// Arguments:
//   sv : The string vector to change.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 6 08:55:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ConfigManager::RemoveLeadAndTailQuotes(stringVector &sv)
{
    for(int i = 0; i < sv.size(); ++i)
    {
        std::string &s = sv[i];
        if(s.size() > 0)
        {
            int head = (s[0] == '"') ? 1 : 0;
            int tail = (s[s.size()-1] == '"') ? 1 : 0;
            sv[i] = s.substr(head, s.size() - head - tail);
        }
    }
}

// ****************************************************************************
// Method: ConfigManager::ReadFieldData
//
// Purpose: 
//   Reads the data for the specified node type, creates a DataNode to
//   store it, and returns the created DataNode.
//
// Arguments:
//   fieldName : The name of the DataNode.
//   type      : The DataNode's desired type.
//
// Returns:    A pointer to a new DataNode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:32:56 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 30 14:41:53 PST 2001
//   Added cases to read unsigned chars.
//
//   Brad Whitlock, Thu Jul 3 16:16:26 PST 2003
//   Made it use stringVector class.
//
//   Brad Whitlock, Tue Jul 22 10:09:22 PDT 2003
//   I fixed a bug in reading in CHAR_NODE data.
//
//   Brad Whitlock, Fri Oct 3 17:19:42 PST 2003
//   Added code to strip leading and tailing quotes from string vectors.
//
// ****************************************************************************

DataNode *
ConfigManager::ReadFieldData(const std::string &tagName, NodeTypeEnum type,
    int tagLength)
{
    DataNode *retval = 0;

    int           i;
    char          cval;
    unsigned char uval;
    int           ival;
    long          lval;
    float         fval;
    double        dval;
    bool          bval;

    // Read strings until we get a '<' character.
    stringVector  sv = ReadStringVector('<');

    int minSize = (tagLength == 0) ? sv.size() :
                  ((tagLength < sv.size()) ? tagLength : sv.size());

    // All 20, or whatever, cases.
    switch(type)
    {
    case CHAR_NODE:
        // Read a character.
        if(minSize > 0)
        {
            retval = new DataNode(tagName, sv[0][0]);
        }
        break;
    case UNSIGNED_CHAR_NODE:
        // Read an int and turn it into an unsigned character.
        if(minSize > 0)
        {
            sscanf(sv[0].c_str(), "%d", &ival);
            uval = (unsigned char)ival;
            retval = new DataNode(tagName, uval);
        }
        break;
    case INT_NODE:
        // Read an int.
        if(minSize > 0)
        {
            sscanf(sv[0].c_str(), "%d", &ival);
            retval = new DataNode(tagName, ival);
        }
        break;
    case LONG_NODE:
        // Read a long.
        if(minSize > 0)
        {
            sscanf(sv[0].c_str(), "%ld", &lval);
            retval = new DataNode(tagName, lval);
        }
        break;
    case FLOAT_NODE:
        // Read a float.
        if(minSize > 0)
        {
            sscanf(sv[0].c_str(), "%g", &fval);
            retval = new DataNode(tagName, fval);
        }
        break;
    case DOUBLE_NODE:
        // Read a double.
        if(minSize > 0)
        {
            sscanf(sv[0].c_str(), "%lg", &dval);
            retval = new DataNode(tagName, dval);
        }
        break;
    case STRING_NODE:
        { // new scope
            std::string temp;
            for(i = 0; i < minSize; ++i)
            {
                temp += sv[i];
                if(i < (minSize - 1))
                    temp += " ";
            }
            retval = new DataNode(tagName, temp);
        }
        break;
    case BOOL_NODE:
        { // new scope
            bval = false;
            if(minSize > 0)
            {
                bval = (sv[0] == "true");
            }
            retval = new DataNode(tagName, bval);
        }
        break;
    case CHAR_ARRAY_NODE:
        { // new scope
            char *cvalArray = 0;
            if(minSize > 0)
            {
                cvalArray = new char[minSize];
                for(i = 0; i < minSize; ++i)
                    sscanf(sv[i].c_str(), "%c", &cvalArray[i]);

                retval = new DataNode(tagName, cvalArray, minSize);
            }
            
            if(cvalArray != 0)
                delete [] cvalArray;
        }
        break;
    case UNSIGNED_CHAR_ARRAY_NODE:
        { // new scope
            unsigned char *uvalArray = 0;
            if(minSize > 0)
            {
                uvalArray = new unsigned char[minSize];
                for(i = 0; i < minSize; ++i)
                {
                    sscanf(sv[i].c_str(), "%d", &ival);
                    uvalArray[i] = (unsigned char)ival;
                }

                retval = new DataNode(tagName, uvalArray, minSize);
            }
            
            if(uvalArray != 0)
                delete [] uvalArray;
        }
        break;
    case INT_ARRAY_NODE:
        { // new scope
            int *ivalArray = 0;
            if(minSize > 0)
            {
                ivalArray = new int[minSize];
                for(i = 0; i < minSize; ++i)
                    sscanf(sv[i].c_str(), "%d", &ivalArray[i]);

                retval = new DataNode(tagName, ivalArray, minSize);
            }
            
            if(ivalArray != 0)
                delete [] ivalArray;
        }
        break;
    case LONG_ARRAY_NODE:
        { // new scope
            long *lvalArray = 0;
            if(minSize > 0)
            {
                lvalArray = new long[minSize];
                for(i = 0; i < minSize; ++i)
                    sscanf(sv[i].c_str(), "%ld", &lvalArray[i]);

                retval = new DataNode(tagName, lvalArray, minSize);
            }
            
            if(lvalArray != 0)
                delete [] lvalArray;
        }
        break;
    case FLOAT_ARRAY_NODE:
        { // new scope
            float *fvalArray = 0;
            if(minSize > 0)
            {
                fvalArray = new float[minSize];
                for(i = 0; i < minSize; ++i)
                    sscanf(sv[i].c_str(), "%g", &fvalArray[i]);

                retval = new DataNode(tagName, fvalArray, minSize);
            }
            
            if(fvalArray != 0)
                delete [] fvalArray;
        }
        break;
    case DOUBLE_ARRAY_NODE:
        { // new scope
            double *dvalArray = 0;
            if(minSize > 0)
            {
                dvalArray = new double[minSize];
                for(i = 0; i < minSize; ++i)
                    sscanf(sv[i].c_str(), "%lg", &dvalArray[i]);

                retval = new DataNode(tagName, dvalArray, minSize);
            }
            
            if(dvalArray != 0)
                delete [] dvalArray;
        }
        break;
    case STRING_ARRAY_NODE:
        { // new scope
            std::string *svalArray = 0;
            if(minSize > 0)
            {
                RemoveLeadAndTailQuotes(sv);
                svalArray = new std::string[minSize];
                for(i = 0; i < minSize; ++i)
                    svalArray[i] = sv[i];

                retval = new DataNode(tagName, svalArray, minSize);
            }
            
            if(svalArray != 0)
                delete [] svalArray;
        }
        break;
    case BOOL_ARRAY_NODE:
        { // new scope
            bool *bvalArray = 0;
            if(minSize > 0)
            {
                bvalArray = new bool[minSize];
                for(i = 0; i < minSize; ++i)
                    bvalArray[i] = (sv[i] == "true");

                retval = new DataNode(tagName, bvalArray, minSize);
            }
            
            if(bvalArray != 0)
                delete [] bvalArray;
        }
        break;
    case CHAR_VECTOR_NODE:
        { // new scope
            charVector temp;
            if(minSize > 0)
            {
                for(i = 0; i < minSize; ++i)
                {
                    sscanf(sv[i].c_str(), "%c", &cval);
                    temp.push_back(cval);
                }

                retval = new DataNode(tagName, temp);
            }
        }
        break;
    case UNSIGNED_CHAR_VECTOR_NODE:
        { // new scope
            unsignedCharVector temp;
            if(minSize > 0)
            {
                for(i = 0; i < minSize; ++i)
                {
                    sscanf(sv[i].c_str(), "%d", &ival);
                    uval = (unsigned char)ival;
                    temp.push_back(uval);
                }

                retval = new DataNode(tagName, temp);
            }
        }
        break;
    case INT_VECTOR_NODE:
        { // new scope
            intVector temp;
            if(minSize > 0)
            {
                for(i = 0; i < minSize; ++i)
                {
                    sscanf(sv[i].c_str(), "%d", &ival);
                    temp.push_back(ival);
                }

                retval = new DataNode(tagName, temp);
            }
        }
        break;
    case LONG_VECTOR_NODE:
        { // new scope
            longVector temp;
            if(minSize > 0)
            {
                for(i = 0; i < minSize; ++i)
                {
                    sscanf(sv[i].c_str(), "%ld", &lval);
                    temp.push_back(lval);
                }

                retval = new DataNode(tagName, temp);
            }
        }
        break;
    case FLOAT_VECTOR_NODE:
        { // new scope
            floatVector temp;
            if(minSize > 0)
            {
                for(i = 0; i < minSize; ++i)
                {
                    sscanf(sv[i].c_str(), "%g", &fval);
                    temp.push_back(fval);
                }

                retval = new DataNode(tagName, temp);
            }
        }
        break;
    case DOUBLE_VECTOR_NODE:
        { // new scope
            doubleVector temp;
            if(minSize > 0)
            {
                for(i = 0; i < minSize; ++i)
                {
                    sscanf(sv[i].c_str(), "%lg", &dval);
                    temp.push_back(dval);
                }

                retval = new DataNode(tagName, temp);
            }
        }
        break;
    case STRING_VECTOR_NODE:
        if(minSize > 0)
        {
            RemoveLeadAndTailQuotes(sv);
            retval = new DataNode(tagName, sv);
        }
        break;
    default:
        break;
    }

    return retval;
}

// ****************************************************************************
// Method: ConfigManager::ReadObject
//
// Purpose: 
//   This method reads the top level and all child DataNodes from the
//   config file and makes them children of parentNode.
//
// Arguments:
//   parentNode : This is the parent of top level node that will be
//                read from the file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:36:21 PST 2000
//
// Modifications:
//   
// ****************************************************************************

bool
ConfigManager::ReadObject(DataNode *parentNode)
{
    bool te = false;
    return ReadObjectHelper(parentNode, te);
}

// ****************************************************************************
// Method: ConfigManager::ReadObjectHelper
//
// Purpose: 
//   This method recursively reads all objects and fields from the file.
//
// Arguments:
//   parentNode : The node to which new objects and fields are added.
//   te         : Whether we encountered an ending tag.
//
// Returns:    True if we should keep reading, false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 16:09:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ConfigManager::ReadObjectHelper(DataNode *parentNode, bool &te)
{
    bool keepReading = true;
    bool tagIsEndTag = false;
    std::string  tagName;
    NodeTypeEnum tagType = INTERNAL_NODE;
    int          tagLength = 0;

    // Read the opening tag.
    keepReading = ReadTag(tagName, tagType, tagLength, tagIsEndTag);

    if(tagIsEndTag && keepReading)
    {
        te = true;
        return keepReading;
    }

    if(tagType == INTERNAL_NODE)
    {
        DataNode *node = new DataNode(tagName);
        parentNode->AddNode(node);

        while(keepReading && !tagIsEndTag)
        {
            keepReading = ReadObjectHelper(node, tagIsEndTag);
        }

        if(tagIsEndTag)
            return keepReading;
    }
    else
        keepReading = ReadField(parentNode, tagName, tagType, tagLength);

    // Read the ending tag.
    stringVector sv = ReadStringVector('>');
    keepReading = sv.size() > 0;

    te = false;
    return keepReading;
}

// ****************************************************************************
// Method: ConfigManager::ReadTag
//
// Purpose: 
//   Reads a tag header and determines the type of object being read.
//
// Arguments:
//   tagName        : The return name for the tag.
//   tagType        : The return type for the tag.
//   tagLength      : The return length for the tag.
//   tafIsReturnTag : Whether or not the tag is an ending tag.
//
// Returns:    True if a tag was read.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 16:11:08 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ConfigManager::ReadTag(std::string &tagName, NodeTypeEnum &tagType,
    int &tagLength, bool &tagIsReturnTag)
{
    // Read strings.
    stringVector sv = ReadStringVector('>');

    std::string tagTypeStr("");
    tagName = "";
    tagType = INTERNAL_NODE;
    tagLength = 0;

    for(int i = 0; i < sv.size(); ++i)
    {
        std::string::size_type pos = sv[i].find('=') + 1;
        std::string token(sv[i].substr(0, pos));
        std::string tokenValue;

        if(pos != std::string::npos && token.size() > 0)
            tokenValue = sv[i].substr(pos + 1, sv[i].size() - pos - 2);

        if(sv[i][0] == '/')
        {
            tagIsReturnTag = true;
            return true;
        }
        else if(token == "type=")
        {
            tagTypeStr = tokenValue;
        }
        else if(token == "name=")
        {
            tagName = tokenValue;
        }
        else if(token == "length=")
        {
            int len;
            if(sscanf(tokenValue.c_str(), "%d", &len) == 1)
            {
                tagLength = (len > 0) ? len : tagLength;
            }
        }
        else if(token == "childObjects=")
        {
            // Skip this tag, it is obsolete.
        }
        else
            tagTypeStr = sv[i];
    }

    // Get the NodeTypeEnum from the tag's type name.
    tagType = GetNodeType(tagTypeStr.c_str());

    return sv.size() > 0;    
}

// ****************************************************************************
// Method: ConfigManager::ReadField
//
// Purpose: 
//   Reads a field and adds it to the parentNode.
//
// Arguments:
//   parentNode : The node to which the field is added.
//   tagName    : The name of the field.
//   tagType    : The type of the field.
//   tagLength  : The length of the field.
//
// Returns:    True if a field was added to the parentNode.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 16:15:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ConfigManager::ReadField(DataNode *parentNode, const std::string &tagName,
    NodeTypeEnum tagType, int tagLength)
{
    DataNode *retval = ReadFieldData(tagName, tagType, tagLength);

    if(retval != 0)
    {
        parentNode->AddNode(retval);
    }

    return retval != 0;
}

// ****************************************************************************
// Method: ConfigManager::GetDefaultConfigFile
//
// Purpose: 
//   Returns the name and path of the default configuration file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 18:26:30 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 29 17:24:20 PST 2001
//   Modified the routine to accept an optional filename argument.
//
//   Brad Whitlock, Fri Apr 27 13:03:58 PST 2001
//   Fixed some string length problems that caused array bound writes.
//
//   Brad Whitlock, Tue Feb 19 12:35:46 PDT 2002
//   Added second argument.
//
//   Jeremy Meredith, Fri Jul  5 18:08:25 PDT 2002
//   I made it look for a "config" file in the ".visit" directory 
//   instead of a ".visitconfig" file.
//
//   Brad Whitlock, Thu Sep 5 16:26:11 PST 2002
//   I made config files store in a predictable place on Windows.
//
//   Jeremy Meredith, Thu Nov 21 10:16:49 PST 2002
//   I made it not look in the current directory unless there is no home
//   directory.
//
//   Brad Whitlock, Wed Apr 23 09:20:11 PDT 2003
//   I made it possible to use different system config files on Windows.
//
//   Brad Whitlock, Mon Feb 23 16:28:31 PST 2004
//   I changed how the config file names are determined on Windows so
//   they can be opened in Notepad by double-clicking.
//
//   Brad Whitlock, Fri Sep 24 13:46:10 PST 2004
//   I fixed a memory overwrite on Windows.
//
// ****************************************************************************

char *
ConfigManager::GetDefaultConfigFile(const char *filename, const char *home)
{
    char *retval;
    char *configFileName;
    int  filenameLength;

    // Figure out the proper filename to use. If no filename was given, use
    // "config" as the default filename.
    if(filename == 0)
    {
        filenameLength = 7;
        configFileName = "config";
    }
    else
    {
        filenameLength = strlen(filename);
        configFileName = (char *)filename;
    }

#if defined(_WIN32)
    char *realhome = getenv("VISITHOME");

    if(realhome != NULL)
    {
        if(home == NULL)
        {
            // User config. Get the username so we can append it to
            // the filename.
            DWORD namelen = 100;
            char username[100];
            GetUserName(username, &namelen);

            retval = new char[strlen(realhome) + namelen + 5 + filenameLength + 2 + 7];
            sprintf(retval, "%s\\%s for %s.ini", realhome, configFileName, username);
        }
        else
        {
            // System config.
            retval = new char[strlen(realhome) + filenameLength + 2 + 7];
            sprintf(retval, "%s\\%s.ini", realhome, configFileName);
        }
    }
    else
    {
        retval = new char[filenameLength + 1 + 4];
        sprintf(retval, "%s.ini", configFileName);
    }
#else
    // The file it is assumed to be in the home directory unless the home
    // directrory doesn't exist, in which case we will say it is
    // in the current directory.
    char *realhome = getenv((home == 0) ? "HOME" : home);
    if(realhome != NULL)
    {
        retval = new char[strlen(realhome) + filenameLength + 2 + 7];
        sprintf(retval, "%s/.visit/%s", realhome, configFileName);
    }
    else
    {
        retval = new char[filenameLength + 1];
        strcpy(retval, configFileName);
    }
#endif

    return retval;
}

// ****************************************************************************
// Method: ConfigManager::GetSystemConfigFile
//
// Purpose: 
//   Returns the system config file name.
//
// Arguments:
//   filename : The base name of the system filename.
//
// Returns:    The system config file name.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 19 12:33:06 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Feb 23 15:53:34 PST 2004
//   I added logic to try and determine the name of the appropriate config
//   file.
//
// ****************************************************************************

char *
ConfigManager::GetSystemConfigFile(const char *filename)
{
    const char *sysConfigName = filename;

    //
    // If no system config file name was given, check the VISITSYSTEMCONFIG
    // environment variable if we're on Windows. Otherwise, just use the
    // name "config".
    //
    if(sysConfigName == 0)
    {
#if defined(_WIN32)
        // Try and get the system config filename from the environment settings.
        sysConfigName = getenv("VISITSYSTEMCONFIG");
#endif

        // If we still don't have the name of a system config file, use 
        // the name "config".
        if(sysConfigName == 0)
            sysConfigName = "config";
    }

    return GetDefaultConfigFile(sysConfigName, "VISITHOME");
}

// ****************************************************************************
// Method: ConfigManager::GetUserVisItDirectory
//
// Purpose: 
//   Returns the user's .visit directory or equivalent.
//
// Returns:    The directory where VisIt likes to put stuff.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 17:44:59 PST 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
ConfigManager::GetUserVisItDirectory() const
{
#if defined(_WIN32)
    const char *home = getenv("VISITHOME");
#else
    const char *home = getenv("HOME");
#endif

    std::string homedir;

    if(home != 0)
    {
#if defined(_WIN32)
        homedir = std::string(home);
#else
        homedir = std::string(home) + "/.visit";
#endif

        if(homedir[homedir.size() - 1] != SLASH_CHAR)
            homedir += SLASH_STRING;
    }

    return homedir;
}
