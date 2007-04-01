#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

#include <VisitALE.h>

static const NodeType ParentNode   = 0x01;
static const NodeType ViewNode     = 0x02;
static const NodeType FieldNode    = 0x04;
static const NodeType RelationNode = 0x08;
static const NodeType AnyNode      = 0xFF;

static void AddChildNode(Node *nd)
{
   int i ;
   Node **newChild = new Node *[nd->numChild+1] ;

   // expand child array 
   for (i=0; i<nd->numChild; ++i)
      newChild[i] = nd->child[i] ;
   if (nd->numChild != 0)
         delete [] nd->child ;
   nd->child = newChild ;
}

static void VisitParseInternal(char *buf, Node **result)
{
   int candidate ;
   Node *nd = new Node(ParentNode) ;
   char *vname ;

   while (*buf != '\0') {
      while (isspace(*buf)) ++buf ;

      switch (*buf) {
         case '(' :     // Views
         {
            ++buf ; ++buf ; // advance to next token
            vname = buf ;

            // advance to next token
            while (!isspace(*buf)) ++buf ;
            *buf++ = '\0' ;

            for (candidate=0; candidate<nd->numChild; ++candidate)
               if (!strcmp(vname, nd->child[candidate]->text))
                  break ;

            if (candidate == nd->numChild) {

               AddChildNode(nd) ;

               // create the node for the subview
               nd->child[nd->numChild] = new Node(ViewNode, nd) ;
               ++nd->numChild ;

               // set the current node to the subview node
               nd = nd->child[nd->numChild-1] ;

               // get the name of the subview
               nd->text = vname ;
            }
            else {
               /* this view already exists */
               nd = nd->child[candidate] ;
            }
         }
         break ;

         case '{' :     // Relations
         {
            ++buf ; ++buf ; // advance to next token

            AddChildNode(nd) ;

            // create the node for the relation
            nd->child[nd->numChild] = new Node(RelationNode, nd) ;
            ++nd->numChild ;

            // set the current node to the relation node
            nd = nd->child[nd->numChild-1] ;

            // get the name of the relation
            nd->text = buf ;

            // advance to next token
            while (!isspace(*buf)) ++buf ;
            *buf++ = '\0' ;
         }
         break ;

         case '[' :     // Fields
         {
            ++buf ; ++buf ; // advance to next token

            AddChildNode(nd) ;

            // create the node for the field
            nd->child[nd->numChild] = new Node(FieldNode, nd) ;
            ++nd->numChild ;

            // set the current node to the field node
            nd = nd->child[nd->numChild-1] ;

            // get the name of the field
            nd->text = buf ;

            // advance to next token
            while (!isspace(*buf)) ++buf ;
            *buf++ = '\0' ;
         }
         break ;

         case ')' :
         case '}' :
         case ']' :
            // go up a level of scope
            nd = nd->parent ;

            // advance to next token
            ++buf ; ++buf ;
            break ;

         default: // add sibling to list
            nd->next = new Node(nd->type, nd->parent) ;
            nd = nd->next ;

            // point at current text
            nd->text = buf ;

            // advance to next token
            while (!isspace(*buf)) ++buf ;
            *buf++ = '\0' ;
      }
   }
   *result = nd ;
}

static void ExtractAttr(Node *nd)
{
   int i ;

   if (nd == 0)
      return ;

   if (nd->type != ParentNode) {
      char *scan = nd->text ;
      while(*scan != ':') ++scan ;
      *scan++ = '\0' ;

      switch(nd->type) {
         case ViewNode:
            nd->len = atoi(scan) ;
            if (nd->len == 0)
               nd->len = nd->parent->len ;
            break ;

         case RelationNode:
            // get the length
            nd->size = atoi(scan) ;
            nd->len = nd->parent->len ;
            while(*scan != ':') ++scan ;
            ++scan ;
            nd->otherView = scan ;
            break ;

         case FieldNode:
            nd->datatype = *scan ;
            scan += 2 ;
            nd->min = atof(scan) ;
            while(*scan != ':') ++scan ;
            ++scan ;
            nd->max = atof(scan) ;
            nd->len = nd->parent->len ;
            break ;

         default: ;
      }


      ExtractAttr(nd->next) ;
   }

   for (i=0; i<nd->numChild; ++i)
      ExtractAttr(nd->child[i]) ;
}

static void VisitDumpTreeRecurse(Node *nd)
{
   int i ;

   if (nd->type != ParentNode) { 
      Node *sibling ;
      printf ("( %s ", nd->text) ;

      for (sibling = nd->next ; sibling != 0; sibling = sibling->next)
         printf("%s ", sibling->text) ;
   }

   for (i=0; i<nd->numChild; ++i)
      VisitDumpTreeRecurse(nd->child[i]) ;

   if (nd->type != ParentNode)
      printf(") ") ;
}

static void VisitDumpTree(Node *nd)
{
    VisitDumpTreeRecurse(nd);
    printf("\n");
}

void VisitFreeVistaInfo(Node *nd)
{
   int i ;
   Node *tmp ;

   for (i=0; i<nd->numChild; ++i)
      VisitFreeVistaInfo(nd->child[i]) ;

   do {
      tmp = nd ;
      nd = nd->next ;
      delete tmp ;
   } while (nd != 0) ;
}

static void VisitFindNodesTopDown(const Node *root, const int nlevels,
    const regex_t *compres, const NodeType *nodetypes, int level,
    Node ***results, int *nmatches, int *maxmatches)
{
   int i,j;

   if (level == nlevels)
      return;

   /* make a pass over children that do not have next pointers */
   for (i=0; i<root->numChild; ++i)
   {
      if ((root->child[i]->next == 0) &&
          ((root->child[i]->type & nodetypes[level]) != 0x0) &&
          (regexec(&compres[level], root->child[i]->text, 0, NULL, 0) == 0))
      {
         if (level == nlevels - 1)
         {
            /* if necessary, make room for more results */
            if (*nmatches == *maxmatches)
            {
               *maxmatches = (*maxmatches)*2 + 1;
               Node **newresults = new Node*[*maxmatches];
               for (j = 0; j < *nmatches; j++)
                  newresults[j] = (*results)[j];
               if (*results != 0)
                  delete [] *results;
               *results = newresults;
            }

            /* put in the new result */
            (*results)[*nmatches] = root->child[i];
            (*nmatches)++;
         }
         else
         {
            VisitFindNodesTopDown(root->child[i], nlevels, compres, nodetypes,
               level+1, results, nmatches, maxmatches);
         }
      }
   }

   /* make a pass over children that do have next pointers */
   for (i=0; i<root->numChild; ++i)
   {
      if ((level == nlevels - 1) && (root->child[i]->next != 0))
      {
         Node *nd = root->child[i];

         while (nd != 0)
         {
            if (((nd->type & nodetypes[level]) != 0x0) &&
                (regexec(&compres[level], nd->text, 0, NULL, 0) == 0))
            {
               /* if necessary, make room for more results */
               if (*nmatches == *maxmatches)
               {
                  *maxmatches = (*maxmatches)*2 + 1;
                  Node **newresults = new Node*[*maxmatches];
                  for (j = 0; j < *nmatches; j++)
                     newresults[j] = (*results)[j];
                  if (*results != 0)
                     delete [] *results;
                  *results = newresults;
               }

               /* put in the new result */
               (*results)[*nmatches] = nd;
               (*nmatches)++;
            }

            nd = nd->next;
         }
      }
   }
}

// walk up the tree from given node and confirm all path components match
static bool BottomUpMatch(const Node *bottom, int nlevels,
    const regex_t *compres, const NodeType *nodetypes)
{
   int level = nlevels - 1;
   bool match = true;

   while ((level >= 0) && match)
   {
      if ( !(((bottom->type & nodetypes[level]) != 0x0) &&
            (regexec(&compres[level], bottom->text, 0, NULL, 0) == 0)) )
         match = false;

      // move up the tree and path to match
      level--;
      bottom = bottom->parent;

      if ((bottom == 0) && (level >= 0))
          match = false;
   }  

   return match;
}

static void VisitFindNodesBottomUp(const Node *root, const int nlevels,
    const regex_t *compres, const NodeType *nodetypes, int level,
    Node ***results, int *nmatches, int *maxmatches)
{
   int i,j;

   if (root == 0)
      return;

   // make a pass over children that do NOT have next pointers
   for (i=0; i<root->numChild; ++i)
   {
      if ((root->child[i]->next == 0) &&
          BottomUpMatch(root->child[i], nlevels, compres, nodetypes))
      {
         /* if necessary, make room for more results */
         if (*nmatches == *maxmatches)
         {
            *maxmatches = (*maxmatches)*2 + 1;
            Node **newresults = new Node*[*maxmatches];
            for (j = 0; j < *nmatches; j++)
               newresults[j] = (*results)[j];
            if (*results != 0)
               delete [] *results;
            *results = newresults;
         }

         /* put in the new result */
         (*results)[*nmatches] = root->child[i];
         (*nmatches)++;
      }

      // we have to inspect the children too
      VisitFindNodesBottomUp(root->child[i], nlevels, compres, nodetypes,
         level+1, results, nmatches, maxmatches);
   }

   /* make a pass over children that do have next pointers */
   for (i=0; i<root->numChild; ++i)
   {
      if (root->child[i]->next != 0)
      {
         Node *nd = root->child[i];

         while (nd != 0)
         {
            if (BottomUpMatch(nd, nlevels, compres, nodetypes))
            {
               /* if necessary, make room for more results */
               if (*nmatches == *maxmatches)
               {
                  *maxmatches = (*maxmatches)*2 + 1;
                  Node **newresults = new Node*[*maxmatches];
                  for (j = 0; j < *nmatches; j++)
                     newresults[j] = (*results)[j];
                  if (*results != 0)
                     delete [] *results;
                  *results = newresults;
               }

               /* put in the new result */
               (*results)[*nmatches] = nd;
               (*nmatches)++;
            }

            nd = nd->next;
         }
      }
   }
}

static void VisitFindNodes(const Node *root, const char *slash_delimited_re,
   Node ***results, int *nmatches, RecurseMode rmode)
{
   const char *p = slash_delimited_re;
   int nlevels = 0;
   static char tmpstr[256];
   static regex_t compres[64];
   static NodeType nodetypes[64];

   /* compile each component's regular expression */
   while (*p != '\0')
   {
      int i = 0;
      if (*p == '/') p++;
      nodetypes[nlevels] = AnyNode;

      /* for any component that begins with '%' character, the
         following V,F and/or R character(s) are used to specify
         the node type we are interested in */
      if (*p == '%')
      {
         nodetypes[nlevels] = 0;
         p++;
         while ((*p == 'V') || (*p == 'F') || (*p == 'R'))
         {
            switch (*p)
            {
               case 'V': nodetypes[nlevels] |= ViewNode; break;
               case 'F': nodetypes[nlevels] |= FieldNode; break;
               case 'R': nodetypes[nlevels] |= RelationNode; break;
            }
            p++;
         }
      }

      /* copy this path component's re into tmpstr */ 
      while ((*p != '/') && (*p != '\0') && (i < sizeof(tmpstr)-1))
         tmpstr[i++] = *p++;
      if (i == sizeof(tmpstr)-1)
         printf("WARNING: Exceeded maximum size of regular expression\n");
      tmpstr[i] = '\0';

      /* compile the re for this path component */
      if (regcomp(&compres[nlevels], tmpstr, REG_EXTENDED|REG_NOSUB))
      {
         printf("WARNING: Error compiling regular expression from %s\n", tmpstr);
         break;
      }

      /* incriment the level */
      nlevels++;
      if (nlevels == sizeof(compres) / sizeof(compres[0]))
      {
         printf("WARNING: Exceeded maximum number of levels\n");
         break;
      }
   }

   /* make the call into the recursion routine */
   int maxmatches = *nmatches;
   if (rmode == TopDown)
       VisitFindNodesTopDown(root, nlevels, compres, nodetypes, 0,
          results, nmatches, &maxmatches);
   else
       VisitFindNodesBottomUp(root, nlevels, compres, nodetypes, 0,
          results, nmatches, &maxmatches);

}

const Node *VisitGetNodeFromPath(const Node *start, const char *path)
{

   const Node *root = start;
   do {
      int i ;
      bool found = false ;

      if (*path == '/') ++path ;

      for (i=0; i<root->numChild; ++i) {
         int len = strlen(root->child[i]->text) ;
         int plen = strlen(path);
         if ((strncmp(path, root->child[i]->text, len) == 0) &&
            (len <= plen) && ((path[len] == '/') || (path[len] == '\0')))
         {
            root = root->child[i] ;
            path += len ;
            found = true ;
            break ;
         }
      }
      if (found == false) {
         root = 0 ;
         break ;
      }
   } while (*path != '\0') ;

   return root ;
}

char *VisitGetPathFromNode(const Node *root, const Node *node)
{
   const Node *tmp;

   // make a pass, walking up the tree, and
   // compute size of returned string
   tmp = node;
   int len = 0;
   while ((tmp != 0) && (tmp != root))
   {
      if (tmp->text != 0)
          len += (strlen(tmp->text)+1); // +1 for '/'

      tmp = tmp->parent;
   }

   if (len == 0)
       return 0;

   char *retval = new char[len+1];
   retval[len] = '\0';

   // make a second pass and populate the returned string
   tmp = node;
   while ((tmp != 0) && (tmp != root))
   {
      if (tmp->text != 0)
      {
          int complen = strlen(tmp->text);
          len -= (complen+1);
          retval[len] = '/';
          while (complen)
              retval[len+complen] = tmp->text[--complen];
      }

      tmp = tmp->parent;
   }

   return retval;

}
