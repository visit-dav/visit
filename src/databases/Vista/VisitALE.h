#ifndef VISIT_ALE_H
#define VISIT_ALE_H

typedef int NodeType;

typedef enum {
    BottomUp,
    TopDown
} RecurseMode;

struct Node
{
   Node *parent ;
   Node *next ;
   Node **child ;
   int numChild ;

   NodeType type ;
   char *text ;
   int len ;
   int size ;
   double min ;
   double max ;
   char datatype ;
   char *otherView ;

   Node(NodeType mytype, Node *p = 0) :
      parent(p), next(0), child(0), numChild(0), type(mytype),
      text(0), len(0), size(0), min(0.0), max(0.0), otherView(0) { }
} ;

#endif

