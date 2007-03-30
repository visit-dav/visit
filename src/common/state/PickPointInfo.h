#ifndef PICKPOINT_INFO_H
#define PICKPOINT_INFO_H


// ****************************************************************************
// Class: PickPointInfo
//
// Purpose:
//   A struct to hold basic pick information. (Passed between viewer &
//   vis window.)
//
// Programmer: Kathleen Bonnell 
// Creation:   November 12, 2001 
//
// Modifications:
//   Kathleen Bonnell, Mon May 20 13:31:45 PDT 2002 
//   Added domain.
//
//   Kathleen Bonnell, Thu Dec 12 13:44:14 PST 2002  
//   Added rayPoint1, rayPoint2. Removed domain, zoneNumber, pickLetter,
//   pickPoint.
//
// ****************************************************************************

typedef struct PickPointInfo 
{
    bool    validPick;
    float   rayPt1[3];
    float   rayPt2[3];
    void   *callbackData;
} PICK_POINT_INFO;

#endif
