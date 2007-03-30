#include <ConfigManager.h>
#include <DataNode.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#endif

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
//   
// ****************************************************************************

void
ConfigManager::WriteBack(DataNode *root)
{
    // Try to open the output file.
    if((fp = fopen("WRITEBACK", "wb")) == 0)
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
            for(i = 0; i < node->GetLength(); ++i, ++sptr)
                fprintf(fp, "%s ", sptr->c_str());
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
                fprintf(fp, "%s ", svec[i].c_str());
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
// ****************************************************************************

void
ConfigManager::WriteObject(DataNode *node, int indentLevel)
{
    // Write the beginning tag.
    WriteIndent(indentLevel);
    if(node->GetNodeType() == INTERNAL_NODE)
        fprintf(fp, "<Object name=\"%s\" childObjects=\"%d\">\n",
            node->GetKey().c_str(), node->GetNumChildObjects());
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
// Method: ConfigManager::ReadTag
//
// Purpose: 
//   This method reads a tag from the config file that indicates the
//   start/end of an Object or Field.
//
// Arguments:
//
// Returns:    0 for "Object" tag
//             1 for "Field" tag
//             2 for "/Object" tag
//             3 for "/Field" tag
//             -1 otherwise
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:25:00 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
ConfigManager::ReadTag()
{
    char c;
    // read until there is a '<' character.
    while(!feof(fp) && ((c = ReadChar()) != '<'));

    // Read characters until ' ', '>', or eof.
    int i;
    char tagName[100];
    memset(tagName, 0, 100 * sizeof(char));
    for(i = 0;
        (i < 100) && !feof(fp) &&
        ((c = ReadChar()) != ' ') && (c != '>'); ++i)
        tagName[i] = c;

    int retval = -1;
    if(i == 0)
        retval = -1; 
    else if(strcmp(tagName, "Object") == 0)
        retval = 0;
    else if(strcmp(tagName, "Field") == 0)
        retval = 1;
    else if(strcmp(tagName, "/Object") == 0)
        retval = 2;
    else if(strcmp(tagName, "/Field") == 0)
        retval = 3;

    return retval;
}

// ****************************************************************************
// Method: ConfigManager::ReadName
//
// Purpose: 
//   Reads a name="string" sequence and returns the name string that
//   was read.
//
// Arguments:
//   name : A pointer to the buffer in which to store the name.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:27:23 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ConfigManager::ReadName(char *name)
{
    int i;
    char c = 0;
    // Read until we get a '"'
    for(i = 0; !feof(fp) && ((c = ReadChar()) != '"'); ++i);

    char buf[100];
    memset(buf, 0, 100 * sizeof(char));
    // Read until we get a '"'
    for(i = 0; (i < 100) && !feof(fp) && ((c = ReadChar()) != '"');)
    {
        if(c != '"')
            buf[i++] = c;
    }

    // Read the length from the buf string
    strncpy(name, buf, 100);
    name[99] = 0;
}

// ****************************************************************************
// Method: ConfigManager::ReadType
//
// Purpose: 
//   Reads a type="string" sequence and converts the string to a 
//   NodeTypeEnum which gets returned.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:28:27 PST 2000
//
// Modifications:
//   
// ****************************************************************************

NodeTypeEnum
ConfigManager::ReadType()
{
    int i;
    char c = 0;
    // Read until we get a '"'
    for(i = 0; !feof(fp) && ((c = ReadChar()) != '"'); ++i);

    char buf[100];
    memset(buf, 0, 100 * sizeof(char));
    // Read until we get a '"'
    for(i = 0; (i < 100) && !feof(fp) && ((c = ReadChar()) != '"');)
    {
        if(c != '"')
            buf[i++] = c;
    }

    // Return the converted type.
    return GetNodeType(buf);
}

// ****************************************************************************
// Method: ConfigManager::ReadLength
//
// Purpose: 
//   Reads a length="number" sequence and returns the number.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:29:13 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
ConfigManager::ReadLength()
{
    int i;
    char c = 0;
    // Read until we get a '"'
    for(i = 0; !feof(fp) && ((c = ReadChar()) != '"'); ++i);

    char buf[50];
    memset(buf, 0, 50 * sizeof(char));
    // Read until we get a '"'
    for(i = 0; (i < 50) && !feof(fp) && ((c = ReadChar()) != '"');)
    {
        if(c != '"')
            buf[i++] = c;
    }

    // Read the length from the buf string
    int length = 0;
    sscanf(buf, "%d", &length);

    return length;
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
// Method: ConfigManager::ReadStringList
//
// Purpose: 
//   Reads a list of strings from the open file until a '<' character
//   is encountered. The string list and its length are returned.
//
// Arguments:
//   length : The string list's return length.
//
// Returns:    A pointer to the string list.
// 
// Note:       The user must free the string list's memory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:30:22 PST 2000
//
// Modifications:
//   
// ****************************************************************************

char **
ConfigManager::ReadStringList(int *length)
{
    *length = 0;
    char **retval = 0;

    char c, tempString[100];
    int  i = 0;
    bool reading = false, keepgoing = true;

    memset(tempString, 0, 100 * sizeof(char));
    while(keepgoing)
    {
        c = ReadChar();
        keepgoing = (!feof(fp) && c != '<');

        if(c != ' ' && c != '\t' && c != '<')
        {
            // Add to the current string.
            tempString[i++] = c;
            reading = true;
        }
        else if(reading)
        {
            // copy the string
            int j, len = strlen(tempString);
            char *newStr = new char[len + 1];
            strcpy(newStr, tempString);
            newStr[len] = 0;

            // Resize the array
            char **newArray = new char*[*length + 1];
            for(j = 0; j < *length; ++j)
                newArray[j] = retval[j];
            newArray[j] = newStr;

            if(*length > 0)
                delete [] retval;
            retval = newArray;
            (*length)++;

            // Indicate that we're done with the current string.
            memset(tempString, 0, 100 * sizeof(char));
            reading = false;
            i = 0;
        }
    }
 
    if(c == '<')
       PutBackChar(c);

    return retval;
}

// ****************************************************************************
// Method: ConfigManager::FreeStringList
//
// Purpose: 
//   This routine frees the memory of a string list returned by
//   the ReadStringList method.
//
// Arguments:
//   strlist : The string list.
//   len : The string list's length.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:32:02 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
ConfigManager::FreeStringList(char **strlist, int len)
{
    if(strlist == 0)
        return;

    for(int i = 0; i < len; ++i)
        delete [] strlist[i];

    delete [] strlist;
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
// ****************************************************************************

DataNode *
ConfigManager::ReadFieldData(const char *fieldName, NodeTypeEnum type, int)
{
    DataNode *retval = 0;

    char          cval;
    unsigned char uval;
    int           ival;
    long          lval;
    float         fval;
    double        dval;
    bool          bval;

    char **strList;
    int i, len;

    // All 20, or whatever, cases.
    switch(type)
    {
    case CHAR_NODE:
        // Read a character.
        fscanf(fp, "%c", &cval);
        retval = new DataNode(fieldName, cval);
        break;
    case UNSIGNED_CHAR_NODE:
        // Read an int and turn it into an unsigned character.
        fscanf(fp, "%d", &ival);
        uval = (unsigned char)ival;
        retval = new DataNode(fieldName, uval);
        break;
    case INT_NODE:
        // Read an int.
        fscanf(fp, "%d", &ival);
        retval = new DataNode(fieldName, ival);
        break;
    case LONG_NODE:
        // Read a long.
        fscanf(fp, "%ld", &lval);
        retval = new DataNode(fieldName, lval);
        break;
    case FLOAT_NODE:
        // Read a float.
        fscanf(fp, "%g", &fval);
        retval = new DataNode(fieldName, fval);
        break;
    case DOUBLE_NODE:
        // Read a double.
        fscanf(fp, "%lg", &dval);
        retval = new DataNode(fieldName, dval);
        break;
    case STRING_NODE:
        { // new scope
            std::string temp;
            strList = ReadStringList(&len);
            for(i = 0; i < len; ++i)
            {
                temp += strList[i];
                if(i < (len - 1))
                    temp += " ";
            }
            retval = new DataNode(fieldName, temp);
            FreeStringList(strList, len);
        }
        break;
    case BOOL_NODE:
        { // new scope
            strList = ReadStringList(&len);
            bval = false;
            if(len > 0)
            {
                bval = (strcmp(strList[0], "true") == 0);
            }
            retval = new DataNode(fieldName, bval);
            FreeStringList(strList, len);
        }
        break;
    case CHAR_ARRAY_NODE:
        { // new scope
            strList = ReadStringList(&len);
            char *cvalArray = 0;
            if(len > 0)
            {
                cvalArray = new char[len];
                for(i = 0; i < len; ++i)
                    sscanf(strList[i], "%c", &cvalArray[i]);
            }
            retval = new DataNode(fieldName, cvalArray, len);
            FreeStringList(strList, len);
            if(cvalArray != 0)
                delete [] cvalArray;
        }
        break;
    case UNSIGNED_CHAR_ARRAY_NODE:
        { // new scope
            strList = ReadStringList(&len);
            unsigned char *uvalArray = 0;
            if(len > 0)
            {
                uvalArray = new unsigned char[len];
                for(i = 0; i < len; ++i)
                {
                    sscanf(strList[i], "%d", &ival);
                    uvalArray[i] = (unsigned char)ival;
                }
            }
            retval = new DataNode(fieldName, uvalArray, len);
            FreeStringList(strList, len);
            if(uvalArray != 0)
                delete [] uvalArray;
        }
        break;
    case INT_ARRAY_NODE:
        { // new scope
            strList = ReadStringList(&len);
            int *ivalArray = 0;
            if(len > 0)
            {
                ivalArray = new int[len];
                for(i = 0; i < len; ++i)
                    sscanf(strList[i], "%d", &ivalArray[i]);
            }
            retval = new DataNode(fieldName, ivalArray, len);
            FreeStringList(strList, len);
            if(ivalArray != 0)
                delete [] ivalArray;
        }
        break;
    case LONG_ARRAY_NODE:
        { // new scope
            strList = ReadStringList(&len);
            long *lvalArray = 0;
            if(len > 0)
            {
                lvalArray = new long[len];
                for(i = 0; i < len; ++i)
                    sscanf(strList[i], "%ld", &lvalArray[i]);
            }
            retval = new DataNode(fieldName, lvalArray, len);
            FreeStringList(strList, len);
            if(lvalArray != 0)
                delete [] lvalArray;
        }
        break;
    case FLOAT_ARRAY_NODE:
        { // new scope
            strList = ReadStringList(&len);
            float *fvalArray = 0;
            if(len > 0)
            {
                fvalArray = new float[len];
                for(i = 0; i < len; ++i)
                    sscanf(strList[i], "%g", &fvalArray[i]);
            }
            retval = new DataNode(fieldName, fvalArray, len);
            FreeStringList(strList, len);
            if(fvalArray != 0)
                delete [] fvalArray;
        }
        break;
    case DOUBLE_ARRAY_NODE:
        { // new scope
            strList = ReadStringList(&len);
            double *dvalArray = 0;
            if(len > 0)
            {
                dvalArray = new double[len];
                for(i = 0; i < len; ++i)
                    sscanf(strList[i], "%lg", &dvalArray[i]);
            }
            retval = new DataNode(fieldName, dvalArray, len);
            FreeStringList(strList, len);
            if(dvalArray != 0)
                delete [] dvalArray;
        }
        break;
    case STRING_ARRAY_NODE:
        { // new scope
            strList = ReadStringList(&len);
            std::string *svalArray = 0;
            if(len > 0)
            {
                svalArray = new std::string[len];
                for(i = 0; i < len; ++i)
                    svalArray[i] = strList[i];
            }
            retval = new DataNode(fieldName, svalArray, len);
            FreeStringList(strList, len);
            if(svalArray != 0)
                delete [] svalArray;
        }
        break;
    case BOOL_ARRAY_NODE:
        { // new scope
            strList = ReadStringList(&len);
            bool *bvalArray = 0;
            if(len > 0)
            {
                bvalArray = new bool[len];
                for(i = 0; i < len; ++i)
                    bvalArray[i] = (strcmp(strList[i], "true") == 0);
            }
            retval = new DataNode(fieldName, bvalArray, len);
            FreeStringList(strList, len);
            if(bvalArray != 0)
                delete [] bvalArray;
        }
        break;
    case CHAR_VECTOR_NODE:
        { // new scope
            strList = ReadStringList(&len);
            charVector temp;
            if(len > 0)
            {
                for(i = 0; i < len; ++i)
                {
                    sscanf(strList[i], "%c", &cval);
                    temp.push_back(cval);
                }
            }
            retval = new DataNode(fieldName, temp);
            FreeStringList(strList, len);
        }
        break;
    case UNSIGNED_CHAR_VECTOR_NODE:
        { // new scope
            strList = ReadStringList(&len);
            unsignedCharVector temp;
            if(len > 0)
            {
                for(i = 0; i < len; ++i)
                {
                    sscanf(strList[i], "%d", &ival);
                    uval = (unsigned char)ival;
                    temp.push_back(uval);
                }
            }
            retval = new DataNode(fieldName, temp);
            FreeStringList(strList, len);
        }
        break;
    case INT_VECTOR_NODE:
        { // new scope
            strList = ReadStringList(&len);
            intVector temp;
            if(len > 0)
            {
                for(i = 0; i < len; ++i)
                {
                    sscanf(strList[i], "%d", &ival);
                    temp.push_back(ival);
                }
            }
            retval = new DataNode(fieldName, temp);
            FreeStringList(strList, len);
        }
        break;
    case LONG_VECTOR_NODE:
        { // new scope
            strList = ReadStringList(&len);
            longVector temp;
            if(len > 0)
            {
                for(i = 0; i < len; ++i)
                {
                    sscanf(strList[i], "%ld", &lval);
                    temp.push_back(lval);
                }
            }
            retval = new DataNode(fieldName, temp);
            FreeStringList(strList, len);
        }
        break;
    case FLOAT_VECTOR_NODE:
        { // new scope
            strList = ReadStringList(&len);
            floatVector temp;
            if(len > 0)
            {
                for(i = 0; i < len; ++i)
                {
                    sscanf(strList[i], "%g", &fval);
                    temp.push_back(fval);
                }
            }
            retval = new DataNode(fieldName, temp);
            FreeStringList(strList, len);
        }
        break;
    case DOUBLE_VECTOR_NODE:
        { // new scope
            strList = ReadStringList(&len);
            doubleVector temp;
            if(len > 0)
            {
                for(i = 0; i < len; ++i)
                {
                    sscanf(strList[i], "%lg", &dval);
                    temp.push_back(dval);
                }
            }
            retval = new DataNode(fieldName, temp);
            FreeStringList(strList, len);
        }
        break;
    case STRING_VECTOR_NODE:
        { // new scope
            strList = ReadStringList(&len);
            stringVector temp;
            if(len > 0)
            {
                for(i = 0; i < len; ++i)
                {
                    temp.push_back(std::string(strList[i]));
                }
            }
            retval = new DataNode(fieldName, temp);
            FreeStringList(strList, len);
        }
        break;
    default:
        break;
    }

    return retval;
}

// ****************************************************************************
// Method: ConfigManager::ReadField
//
// Purpose: 
//   Reads a field and creates a DataNode for it under the specified
//   parentNode.
//
// Arguments:
//   parentNode : The parent of the DataNode to be created.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 29 17:35:20 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 30 14:46:14 PST 2001
//   Added a case for arrays of unsigned characters.
//
// ****************************************************************************

void
ConfigManager::ReadField(DataNode *parentNode)
{
    int  length = 0;
    char objName[100];
    NodeTypeEnum type;

    // Read the name of the field.
    ReadName(objName);

    // Read the type. If the type requires
    type = ReadType();
    switch(type)
    {
    case CHAR_ARRAY_NODE:
    case UNSIGNED_CHAR_ARRAY_NODE:
    case INT_ARRAY_NODE:
    case LONG_ARRAY_NODE:
    case FLOAT_ARRAY_NODE:
    case DOUBLE_ARRAY_NODE:
    case STRING_ARRAY_NODE:
    case BOOL_ARRAY_NODE:
        length = ReadLength();
    default:
        break;
    }
    FinishTag();

    // Read the field's data.
    DataNode *field = ReadFieldData(objName, type, length);
    if(field != 0)
        parentNode->AddNode(field);
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

void
ConfigManager::ReadObject(DataNode *parentNode)
{
    int prefix;
    int nSubObjects = 0;

    while((prefix = ReadTag()) != -1)
    {
        if(prefix == 0) // Object
        {
            char objName[100];
            ReadName(objName);
            nSubObjects = ReadLength();
            FinishTag();

            // Create a new internal node and add it to the parent
            DataNode *node = new DataNode(objName);
            parentNode->AddNode(node);

            // Read any child nodes that the object may have
            ReadObject(node);

            // Indicate that a sub-object has been read.
            --nSubObjects;
        }
        else if(prefix == 1) // Field
        {
            ReadField(parentNode);
        }
        else if(prefix == 2)
        {
            if(nSubObjects == 0)
                return;
        }
    }
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
#if defined(_WIN32)
        // Try and get the system config filename from the environment settings.
        configFileName = getenv("VISITSYSTEMCONFIG");
        if(configFileName != 0)
            filenameLength = strlen(configFileName);
        else
        {
#endif
            filenameLength = 14;
            configFileName = "config";
#if defined(_WIN32)
        }
#endif
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
            sprintf(retval, "%s\\%s for %s", realhome, configFileName, username);
        }
        else
        {
            // System config.
            retval = new char[strlen(realhome) + filenameLength + 2 + 7];
            sprintf(retval, "%s\\%s", realhome, configFileName);
        }
    }
    else
    {
        retval = new char[filenameLength + 1];
        strcpy(retval, configFileName);
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
//   
// ****************************************************************************

char *
ConfigManager::GetSystemConfigFile(const char *filename)
{
    return GetDefaultConfigFile(filename, "VISITHOME");
}
