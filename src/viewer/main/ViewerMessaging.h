#ifndef VIEWER_MESSAGING_H
#define VIEWER_MESSAGING_H

// Functions to send messages
void Error(const char *message);
void Warning(const char *message);
void Message(const char *message);

// Functions to send status
void Status(const char *message);
void Status(const char *message, int milliseconds);
void Status(const char *sender, const char *message);
void Status(const char *sender, int percent, int curStage,
            const char *curStageName, int maxStage);
void ClearStatus(const char *sender = 0);

#endif
