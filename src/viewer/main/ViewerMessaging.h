#ifndef VIEWER_MESSAGING_H
#define VIEWER_MESSAGING_H
#include <viewer_exports.h>

// Functions to send messages
VIEWER_API void Error(const char *message);
VIEWER_API void Warning(const char *message);
VIEWER_API void Message(const char *message);
VIEWER_API void ErrorClear();

// Functions to send status
void Status(const char *message);
void Status(const char *message, int milliseconds);
void Status(const char *sender, const char *message);
void Status(const char *sender, int percent, int curStage,
            const char *curStageName, int maxStage);
void ClearStatus(const char *sender = 0);

#endif
