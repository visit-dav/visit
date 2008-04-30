#ifndef UNICODE_HELPER_H
#define UNICODE_HELPER_H
#include <winutil_exports.h>
#include <qstring.h>
#include <MessageAttributes.h>
#include <StatusAttributes.h>

// Functions that let us access the bytes that comprise a unicode QString
// and reconstitute it from those same bytes.
void WINUTIL_API QString_to_unsignedCharVector(const QString &s, unsignedCharVector &vec);
void WINUTIL_API unsignedCharVector_to_QString(unsignedCharVector &vec, QString &s);

// Functions that Set/Get the best form of text from the MessageAttributes
// object, which may now contain both regular text and unicode versions
// of a message. The unicode version is favored if it is present.
void    WINUTIL_API MessageAttributes_SetText(MessageAttributes &ma, 
                                              const QString &text,
                                              bool hasUnicode = true);
QString WINUTIL_API MessageAttributes_GetText(MessageAttributes &ma);

// Functions that Set/Get the best form of text from the StatusAttributes
// object, which may now contain both regular text and unicode versions
// of a message. The unicode version is favored if it is present.
void    WINUTIL_API StatusAttributes_SetStatusMessage(StatusAttributes &ma,
                                                      const QString &text,
                                                      bool hasUnicode = true);
QString WINUTIL_API StatusAttributes_GetStatusMessage(StatusAttributes &ma);

#endif
