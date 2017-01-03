/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <Namescheme.h>
#include <Utility.h>

#define FREE(M) if(M){free(M);M=0;}

// ****************************************************************************
//  Function: helper functions and types for namescheme expression evaluation. 
//
//  Purpose: Build simple, binary evaluation tree to facilitate evaluation 
//      of expressions used in nameschemes.
//
//  This code was adapted from silo library's namescheme function, where it
//  originated as C code hence prolific use of char* instead of std::string.
//
//  Also, periodically, the code here in VisIt should be brought up to date
//  w.r.t. the original Silo source code.
//
//  For examples of use, see the test client, Namescheme_test.C.
//
//  Programmer: Mark C. Miller
//  Creation:   Wed Aug 26 15:34:45 PDT 2009
//
// ****************************************************************************

void Namescheme::FreeTree(Namescheme::DBexprnode *tree)
{
    if (!tree)
        return;
    FreeTree(tree->left);
    FreeTree(tree->right);
    free(tree);
}

Namescheme::DBexprnode *Namescheme::UpdateTree(Namescheme::DBexprnode *tree, const char t, int v, char *s)
{
    Namescheme::DBexprnode *retval = 0;
    Namescheme::DBexprnode *newnode = (Namescheme::DBexprnode *) calloc(1,sizeof(Namescheme::DBexprnode));
    newnode->type = t;
    if (t == 'c')
        newnode->val = v;
    else 
    {
        if (s)
            strncpy(newnode->sval, s, sizeof(newnode->sval)-1);
        else
            strcpy(newnode->sval, "(null)");
    }

    newnode->left = 0;
    newnode->right = 0;

    if (tree == 0)
    {
        retval = newnode;
    }
    else if (tree->left == 0 && tree->right == 0)
    {
        // t better be an operator
        newnode->left = tree;
        retval = newnode;
    }
    else if (tree->left != 0 && tree->right == 0)
    {
        // t better be a constant
        tree->right = newnode;
        retval = tree;
    }
    else if (tree->left == 0 && tree->right != 0)
    {
        // should never happen
        ;
    }
    else
    {
        // t better be an operator
        newnode->left = tree;
        retval = newnode;
    }

    return retval;
}

Namescheme::DBexprnode *Namescheme::BuildExprTree(const char **porig)
{
    Namescheme::DBexprnode *tree = 0;
    const char *p = *porig;

    while (*p != '\0')
    {
        switch (*p)
        {
            case ' ':
            {
                break; // ignore spaces
            }

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            {
                char tokbuf[256];
                char *tp = tokbuf;
                long int val;
                while ('0' <= *p && *p <= '9')
                    *tp++ = *p++;
                p--;
                *tp = '\0';
                errno = 0;
                val = strtol(tokbuf, 0, 0);
                if (errno == 0 && val != LONG_MIN && val != LONG_MAX)
                    tree = UpdateTree(tree, 'c', (int) val, 0);
                break;
            }

            case '\'': // beginning of string
            {
                char tokbuf[129];
                char *tp = tokbuf;
                p++;
                while (*p != '\'')
                    *tp++ = *p++;
                *tp = '\0';
                errno = 0;
                tree = UpdateTree(tree, 's', 0, tokbuf);
                break;
            }

            case '$': // array ref
            case '#':
            {
                char typec = *p;
                char tokbuf[129];
                char *tp = tokbuf;
                p++;
                while (*p != '[')
                    *tp++ = *p++;
                p--;
                *tp = '\0';
                errno = 0;
                tree = UpdateTree(tree, typec, 0, tokbuf);
                break;
            }

            case 'n':
            case '+': case '-': case '*': case '/': case '%':
            case '|': case '&': case '^':
            {
                tree = UpdateTree(tree, *p, 0, 0);
                break;
            }

            case '(': case '[':
            {
                Namescheme::DBexprnode *subtree;
                p++;
                subtree = BuildExprTree(&p);
                if (tree == 0)
                    tree = subtree;
                else if (tree->left == 0)
                    tree->left = subtree;
                else if (tree->right == 0)
                    tree->right = subtree;
                break;
            }

            case ']': // terminating array ref
            case ')': // terminating subtree
            {
                *porig = p;
                return tree;
            }

            case '?':
            {
                Namescheme::DBexprnode *newtreeq = 0;
                Namescheme::DBexprnode *newnodec = 0;
                Namescheme::DBexprnode *subtreel, *subtreer;
                newtreeq = UpdateTree(newtreeq, *p, 0, 0);
                newtreeq->left = tree;
                p++;
                subtreel = BuildExprTree(&p);
                p++;
                subtreer = BuildExprTree(&p);
                newnodec = UpdateTree(newnodec, ':', 0, 0);
                newnodec->left = subtreel;
                newnodec->right = subtreer;
                newtreeq->right = newnodec;
                tree = newtreeq;
                break;
            }

            case ':': // terminating if-then-else (?::)
            {
                *porig = p;
                return tree;
            }


        }
        p++;
    }
    *porig = p;
    return tree;
}

/* very simple circular cache for a handful of embedded nameschemes used internally */
int Namescheme::SaveInternalString(Namescheme *ns, char const * const sval)
{
    int modn = ns->nembed++ % Namescheme::max_expstrs;
    if (ns->embedns[modn])
        delete ns->embedns[modn];
    ns->embedns[modn] = new Namescheme(sval);
    return modn;
}

/* very simple circular cache for strings returned to caller from GetName */
char * Namescheme::SaveReturnedString(char const * const retstr)
{
    static int n = 0;
    int modn = n++ % Namescheme::max_retstrs;
    if (retstr == 0)
    {
        for (n = 0; n < Namescheme::max_retstrs; n++)
            FREE(retstrbuf[n]);
        n = 0;
        return 0;
    }
    FREE(retstrbuf[modn]);
    retstrbuf[modn] = strdup(retstr);
    return retstrbuf[modn];
}

int Namescheme::EvalExprTree(Namescheme *ns, Namescheme::DBexprnode *tree, int n)
{
    if (tree == 0)
        return 0;
    else if ((tree->type == '$' || tree->type == '#') && tree->left != 0)
    {
        int i, q = EvalExprTree(ns, tree->left, n);
        for (i = 0; i < ns->narrefs; i++)
        {
            if (strcmp(tree->sval, ns->arrnames[i]) == 0)
            {
                if (tree->type == '$')
                    return SaveInternalString(ns,  ((char**)ns->arrvals[i])[q]);
                else
                    return ((int*)ns->arrvals[i])[q];
            }
        }
    }
    else if (tree->left == 0 && tree->right == 0)
    {
        if (tree->type == 'c')
            return tree->val;
        else if (tree->type == 'n')
            return n;
        else if (tree->type == 's')
            return SaveInternalString(ns, tree->sval);
    }
    else if (tree->left != 0 && tree->right != 0)
    {
        int vc = 0, vl = 0, vr = 0;
        if (tree->type == '?')
        {
            vc = EvalExprTree(ns, tree->left, n);
            tree = tree->right;
            if (vc) 
                vl = EvalExprTree(ns, tree->left, n);
            else
                vr = EvalExprTree(ns, tree->right, n);
        }
        else
        {
            vl = EvalExprTree(ns, tree->left, n);
            vr = EvalExprTree(ns, tree->right, n);
        }
        switch (tree->type)
        {
            case '+': return vl + vr;
            case '-': return vl - vr;
            case '*': return vl * vr;
            case '/': return (vr != 0 ? vl / vr : 1);
            case '%': return (vr != 0 ? vl % vr : 1);
            case '|': return vl | vr;
            case '&': return vl & vr;
            case '^': return vl ^ vr;
            case ':': return vc ? vl : vr; 
        }
    }
    return 0;
}

const int Namescheme::max_expstrs = 8;
const int Namescheme::max_fmtlen  = 4096;
char * Namescheme::retstrbuf[max_retstrs];

// ****************************************************************************
//  Method: Constructor 
//
//  Purpose: Construct a Namescheme object.
//
//  Programmer: Mark C. Miller
//  Creation:   Wed Aug 26 15:34:45 PDT 2009
//
//  Modifications:
//    Brad Whitlock, Wed Sep  2 15:59:27 PDT 2009
//    I removed a dependence on strndup, which isn't on Mac.
//
//    Mark C. Miller, Thu Dec 18 13:02:29 PST 2014
//    Enhanced embedded strings to themselves be namescheme strings.
//    Also, to support constant valued strings (e.g. namescheme strings with
//    no conversion specs (%), allowed for them to NOT require being lead
//    with the delimiter character.
//
//    Mark C. Miller, Wed Feb 11 16:58:15 PST 2015
//    Fix a problem with loop termination with string ends with delim char.
// ****************************************************************************
Namescheme::Namescheme(const char *fmt, ...)
{
    // Initialize members
    this->fmt = 0;
    this->fmtptrs = 0;
    this->fmtlen = 0;
    this->ncspecs = 0;
    this->delim = 0;
    this->nembed = 0;
    this->embedns = (Namescheme **) calloc(max_expstrs, sizeof(Namescheme*));
    this->narrefs = 0;
    this->arrnames = 0;
    this->arrvals = 0;
    this->exprstrs = 0;
    this->exprtrees = 0;

    va_list ap;
    int i, j, k, n, pass, ncspecs, done;

    // We have nothing to do for a null or empty format string
    if (fmt == 0 || *fmt == '\0')
        return;

    // set the delimeter character
    n = 0;
    while (fmt[n] != '\0')
    {
        if (fmt[n] == '%' && fmt[n+1] != '%')
            break;
        n++;
    }
    if (fmt[n] == '%') // have at least one conversion spec
        this->delim = fmt[0];
    else
        this->delim = '\0';

    // compute length up to max of 4096 of initial segment of fmt representing
    // the printf-style format string.
    n = 1;
    while (n < max_fmtlen && fmt[n] != '\0' && fmt[n] != this->delim)
        n++;
    if (n == max_fmtlen) // we pick arb. upper bound in length of 4096
        return;

    // grab just the part of fmt that is the printf-style format string
    this->fmt = C_strndup(&fmt[1],n-1);
    this->fmtlen = n-1;

    // In 2 passes, count conversion specs. and then setup pointers to each 
    for (pass = 0; pass < 2; pass++)
    {
        if (pass == 1)
        {
            this->fmtptrs = (const char **) calloc(this->ncspecs+1, sizeof(char*));
            this->ncspecs = 0;
        }
        for (i = 0; i < this->fmtlen-1; i++)
        {
            if (this->fmt[i] == '%' && 
                this->fmt[i+1] != '%')
            {
                if (pass == 1)
                    this->fmtptrs[this->ncspecs] = &(this->fmt[i]);
                this->ncspecs++;
            }
        }
    }
    this->fmtptrs[this->ncspecs] = &(this->fmt[n+1]);

    // If there are no conversion specs., we have nothing more to do.
    // However, in this case, assume the first char is a real char.
    if (this->ncspecs == 0)
    {
        free(this->fmt);
        this->fmt = C_strndup(&fmt[0],n);
        this->fmtlen = n;
        return;
    }

    // Make a pass through rest of fmt string to count array refs in the
    // expression substrings.
    i = n+1;
    while (i < max_fmtlen && fmt[i] != '\0')
    {
        if (fmt[i] == '$' || fmt[i] == '#')
            this->narrefs++;
        i++;
    }
    if (i == max_fmtlen)
        return;

    // allocate various arrays needed by the naming scheme
    this->exprstrs = (char **) calloc(this->ncspecs, sizeof(char*));
    this->exprtrees = (Namescheme::DBexprnode **) calloc(this->ncspecs,
        sizeof(Namescheme::DBexprnode*));
    if (this->narrefs > 0)
    {
        this->arrnames = (char **) calloc(this->narrefs, sizeof(char*));
        this->arrvals  = (const int **) calloc(this->narrefs, sizeof(int*));
    }

    // Ok, now go through rest of fmt string a second time and grab each
    // expression that goes with each conversion spec. Also, handle array refs
    i = n+1;
    this->narrefs = 0;
    ncspecs = 0;
    va_start(ap, fmt);
    done = 0;
    while (!done)
    {
        if (fmt[i] == '$' || fmt[i] == '#')
        {
            for (j = 1; fmt[i+j] != '['; j++)
                ;
            for (k = 0; k < this->narrefs; k++)
            {
                if (strncmp(&fmt[i+1],this->arrnames[k],j-1) == 0)
                    break;
            }
            if (k == this->narrefs)
            {
                this->arrnames[k] = (char*)calloc(j, sizeof(char));
                strncpy(this->arrnames[k], &fmt[i+1], j-1);
                this->arrvals[k] = va_arg(ap, const int *);
                this->narrefs++;
            }
        }
        else if (fmt[i] == this->delim || fmt[i] == '\0')
        {
            char *exprstr1, *exprstr2;
            this->exprstrs[ncspecs] = (char*)calloc(i-(n+1)+1, sizeof(char));
            strncpy(this->exprstrs[ncspecs], &fmt[n+1], i-(n+1));
            exprstr1 = exprstr2 = strdup(this->exprstrs[ncspecs]);
            this->exprtrees[ncspecs] = BuildExprTree((const char **) &exprstr1);
            free(exprstr2);
            ncspecs++;
            if ((fmt[i] == '\0') ||
                (fmt[i] == this->delim && fmt[i+1] == '\0'))
                done = 1;
            n = i;
        }
        i++;
    }
    va_end(ap);
}

// ****************************************************************************
//  Method: Destructor 
//
//  Purpose: Destruct a Namescheme object. 
//
//  Programmer: Mark C. Miller
//  Creation:   Wed Aug 26 15:34:45 PDT 2009
//
//  Modifications:
//
//    Mark C. Miller, Thu Dec 18 13:04:31 PST 2014
//    Changed embedded strings to be embedded nameschemes.
// ****************************************************************************
Namescheme::~Namescheme()
{
    int i;

    // Always free up the old name scheme and clear it out
    FREE(fmt);
    FREE(fmtptrs);
    for (i = 0; i < max_expstrs; i++)
    {
        if (embedns[i])
            delete embedns[i];
    }
    FREE(embedns);
    for (i = 0; i < ncspecs; i++)
    {
        FREE(exprstrs[i]);
        FreeTree(exprtrees[i]);
    }
    FREE(exprstrs);
    FREE(exprtrees);
    for (i = 0; i < narrefs; i++)
    {
        FREE(arrnames[i]);
        //FREE(arrvals[i]); user allocates these
    }
    FREE(arrnames);
    FREE(arrvals);
}

// ****************************************************************************
//  Method: GetName
//
//  Purpose: Compute a name from the namescheme object using natural number. 
//
//  Programmer: Mark C. Miller
//  Creation:   Wed Aug 26 15:34:45 PDT 2009
//
//  Modifications:
//
//    Mark C. Miller, Thu Dec 18 13:05:08 PST 2014
//    Changed embedded strings to act as embedded nameschemes.
// ****************************************************************************
const char *Namescheme::GetName(int natnum)
{
    char *currentExpr, *tmpExpr;
    char retval[1024];
    int i;

    /* a hackish way to cleanup the saved returned string buffer */
    if (natnum < 0) return SaveReturnedString(0);

    if (!this->fmt) return "";

    retval[0] = '\0';
    strncat(retval, this->fmt, this->fmtptrs[0] - this->fmt);
    for (i = 0; i < this->ncspecs; i++)
    {
        char tmp[256];
        char tmpfmt[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        DBexprnode *exprtree;
        int theVal;

        currentExpr = strdup(this->exprstrs[i]);
        tmpExpr = currentExpr;
        exprtree = BuildExprTree((const char **)&currentExpr);
        theVal = EvalExprTree(this, exprtree, natnum);
        FreeTree(exprtree);
        strncpy(tmpfmt, this->fmtptrs[i], this->fmtptrs[i+1] - this->fmtptrs[i]);
        if (strncmp(tmpfmt, "%s", 2) == 0 && 0 <= theVal && theVal < max_retstrs)
            sprintf(tmp, tmpfmt, this->embedns[theVal]->GetName(natnum));
        else
            sprintf(tmp, tmpfmt, theVal);
        strcat(retval, tmp);
        FREE(tmpExpr);
    }
    return SaveReturnedString(retval);
}

int Namescheme::GetIndex(int natnum)
{
    char const *name_str = this->GetName(natnum);
    int i = 0;

    if (!name_str) return -1;

    while (name_str[i] && !(strchr("0123456789+-",                name_str[i  ]) &&
                            strchr("0123456789.aAbBcCdDeEfFxX+-", name_str[i+1])))
        i++;

    if (!name_str[i]) return -1;

    return (int) strtol(&name_str[i], 0, 10);
}

void Namescheme::FreeClassStaticResources(void)
{
    SaveReturnedString(0);
}
