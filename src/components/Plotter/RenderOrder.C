#include <RenderOrder.h>

static const char *RenderOrderNameLookup[] = {
    "MUST_GO_FIRST",
    "DOES_NOT_MATTER",
    "MUST_GO_LAST",
    "ABSOLUTELY_LAST",
    "MAX_ORDER"
};

// *******************************************************************
// Function: RenderOrder2Int
//
// Purpose:
//   Converts a RenderOrder variable to an integer value.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   April 3, 2001 
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003
//   Added ABSOLUTELY_LAST.
//
// *******************************************************************

int
RenderOrder2Int(RenderOrder order)
{
    if(order == MUST_GO_FIRST)
        return 0;
    else if(order == DOES_NOT_MATTER)
        return 1;
    else if(order == MUST_GO_LAST)
        return 2;        
    else if(order == ABSOLUTELY_LAST)
        return 3;        
    else
        return 4;
}

// *******************************************************************
// Function: Int2RenderOrder
//
// Purpose:
//   Converts an integer variable to a RenderOrder value.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   April 3, 2001 
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003
//   Added ABSOLUTELY_LAST.
//
// *******************************************************************

RenderOrder
Int2RenderOrder(int order)
{
    if(order == 0)
        return MUST_GO_FIRST;        
    else if(order == 1)
        return DOES_NOT_MATTER;
    else if(order == 2)
        return MUST_GO_LAST;
    else if(order == 3)
        return ABSOLUTELY_LAST;
    else
        return MAX_ORDER;
}


// *******************************************************************
// Function: RenderOrderName
//
// Purpose:
//   Returns the name of the plot order.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   April 3, 2001 
//
// *******************************************************************

const char *
RenderOrderName(RenderOrder order)
{
    return RenderOrderNameLookup[RenderOrder2Int(order)];
}


// *******************************************************************
// Function: RenderOrderName
//
// Purpose:
//   Returns the name of the plot order.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   April 3, 2001 
//
// *******************************************************************

const char *
RenderOrderName(int order)
{
    return RenderOrderNameLookup[order];
}
