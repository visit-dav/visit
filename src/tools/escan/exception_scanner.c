/******************************************************************************
 *
 * Purpose: This program scans source code for incorrect exception usage.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jan 16 16:15:43 PST 2003
 *
 * Modifications:
 *
 *****************************************************************************/
#include <exception_scanner.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDENT 4

int scope_count = 0;
int line_count = 0;
int try_scope = 0;
int debugflag = 0;
int verboseflag = 0;
char *fileName = NULL;
char *funcName = NULL;

void printError(const char *errorStr)
{
    char *fn = "<input>";

    if(fileName != NULL)
        fn = fileName;

    if(funcName != NULL)
        printf("Error: %s:%d %s\n\t%s\n", fn, line_count+1, funcName, errorStr);
    else
        printf("Error: %s:%d\n\t%s\n", fn, line_count+1, errorStr);
}

void ignoreToken(void)
{
}

void printToken(const char *token)
{
    if(verboseflag)
    {
        int i;
        for(i = 0; i < scope_count * INDENT; ++i)
            printf(" ");
        printf("%s\n", token);
    }
}

void processToken(const char *token)
{
    if(strcmp(token, "TRY") == 0)
    {
        ++try_scope;
        printToken("TRY");
    }
    else if(strcmp(token, "try") == 0)
    {
        ++try_scope;
        printToken("try");
        printError("Using try instead of TRY!");
    }
    else if(strcmp(token, "throw") == 0)
    {
        printToken("throw");
        printError("Using throw instead of EXCEPTION macro!");
    }
    else if(strcmp(token, "rethrow") == 0)
    {
        printToken("rethrow");
        printError("Using rethrow instead of RETHROW macro!");
    }
    else if(strcmp(token, "ENDTRY") == 0)
    {
        printToken("ENDTRY");
        --try_scope;
    }
    else if(strcmp(token, "CATCH") == 0)
    {
        printToken("CATCH");
    }
    else if(strcmp(token, "catch") == 0)
    {
        printToken("catch");
        printError("Using catch instead of CATCH or CATCH2 macros!");
    }
    else if(strcmp(token, "CATCH2") == 0)
    {
        printToken("CATCH2");
    }
    else if(strcmp(token, "CATCHALL") == 0)
    {
        printToken("CATCHALL");
    }
    else if(strcmp(token, "return") == 0)
    {
        printToken("return");
        if(try_scope > 0)
            printError("Unwrapped return statement in exception handler!");
    }
}

void incrementScope(void)
{
    printToken("{");
    ++scope_count;
}

void decrementScope(void)
{
    --scope_count;
    printToken("}");
    if(scope_count == 0)
    {
        if(try_scope > 0)
            printError("End of function reached without seeing enough ENDTRY macros!");
        try_scope = 0;
        if(verboseflag)printf("\n");
    }
}

void advanceSourceLineCount(void)
{
    ++line_count;
}

void set_filename(const char *str)
{
    if(fileName != NULL)
        free(fileName);
    fileName = strdup(str);
    line_count = 0;
}

void set_debugflag(int val)
{
    debugflag = val;
}

void set_verboseflag(int val)
{
    verboseflag = val;
}
