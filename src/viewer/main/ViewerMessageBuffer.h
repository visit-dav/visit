#ifndef VIEWER_MESSAGE_BUFFER_H
#define VIEWER_MESSAGE_BUFFER_H
#include <viewer_exports.h>

// *******************************************************************
// Class: ViewerMessageBuffer
//
// Purpose:
//   This class buffers a list of message strings.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Fri Oct 27 13:37:49 PST 2000
//
// Modifications:
//   
// *******************************************************************

class VIEWER_API ViewerMessageBuffer
{
  public:
    ViewerMessageBuffer();
    ~ViewerMessageBuffer();

    void AddString(const char *str);

    int ReadMessage(char *msg);

  protected:
    int Grow();

  private:
    int       head;
    int       tail;
    int       size;
    int       mask;
    char     *chrs;
};

#endif
