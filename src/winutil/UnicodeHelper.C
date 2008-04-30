#include <UnicodeHelper.h>

// ****************************************************************************
// Function: QString_to_unsignedCharVector
//
// Purpose: 
//   Stores the bytes that make up a unicode QString into a unsignedCharVector so we 
//   can effectively send a QString across the network.
//
// Arguments:
//   s   : The string to be stored.
//   vec : The destination vector for the string's bytes.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 10:02:50 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QString_to_unsignedCharVector(const QString &s, unsignedCharVector &vec)
{
    vec.clear();
    vec.reserve(s.length() * 2);
    const QChar *u = s.unicode();
    for(int i = 0; i < s.length(); ++i)
    {
        vec.push_back(u->cell());
        vec.push_back(u->row());
        ++u;
    }
}

// ****************************************************************************
// Function: unsignedCharVector_to_QString
//
// Purpose: 
//   Converts the bytes in the unsignedCharVector back into a QString.
//
// Arguments:
//   vec : The vector of bytes that make up the string.
//   s   : The output string
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 10:02:50 PDT 2008
//
// Modifications:
//   
// ****************************************************************************


void
unsignedCharVector_to_QString(unsignedCharVector &vec, QString &s)
{
    s = QString();
    for(int i = 0; i < vec.size(); i += 2)
        s += QString(QChar(vec[i], vec[i+1]));
}

// ****************************************************************************
// Method: MessageAttributes_SetText
//
// Purpose: 
//   Set the message text into the MessageAttributes in both text form and 
//   in unicode form.
//
// Arguments:
//   ma   : The destination message attributes.
//   text : The text to store.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 10:05:18 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
MessageAttributes_SetText(MessageAttributes &ma, const QString &text, 
    bool hasUnicode)
{
    // Set the text (this is a fallback now)
    ma.SetText(std::string(text.latin1()));

    // Store the unicode version of the text as a unsignedCharVector. We'll 
    // use this if it is available.
    unsignedCharVector unicode;
    QString_to_unsignedCharVector(text, unicode);
    ma.SetUnicode(unicode);
    ma.SetHasUnicode(hasUnicode);
}

// ****************************************************************************
// Method: MessageAttributes_GetText
//
// Purpose: 
//   Retrieves the message string from the MessageAttributes, favoring
//   unicode if it is available.
//
// Arguments:
//   ma : The message attributes that contain the text.
//
// Returns:    A QString representation of the text.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 10:06:21 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
MessageAttributes_GetText(MessageAttributes &ma)
{
    QString retval;

    if(ma.GetHasUnicode())
        unsignedCharVector_to_QString(ma.GetUnicode(), retval);
    else
        retval = QString(ma.GetText().c_str());

    return retval;
}

// ****************************************************************************
// Method: StatusAttributes_SetStatusMessage
//
// Purpose: 
//   Set the message text into the StatusAttributes in both text form and 
//   in unicode form.
//
// Arguments:
//   sa   : The destination status attributes.
//   text : The text to store.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 10:05:18 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
StatusAttributes_SetStatusMessage(StatusAttributes &sa, const QString &text, 
    bool hasUnicode)
{
    // Set the text (this is a fallback now)
    sa.SetStatusMessage(std::string(text.latin1()));

    // Store the unicode version of the text as a unsignedCharVector. We'll 
    // use this if it is available.
    unsignedCharVector unicode;
    QString_to_unsignedCharVector(text, unicode);
    sa.SetUnicode(unicode);
    sa.SetHasUnicode(hasUnicode);
}

// ****************************************************************************
// Method: StatusAttributes_GetStatusMessage
//
// Purpose: 
//   Retrieves the message string from the StatusAttributes, favoring
//   unicode if it is available.
//
// Arguments:
//   sa : The status attributes that contain the text.
//
// Returns:    A QString representation of the text.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 10:06:21 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QString
StatusAttributes_GetStatusMessage(StatusAttributes &sa)
{
    QString retval;

    if(sa.GetHasUnicode())
        unsignedCharVector_to_QString(sa.GetUnicode(), retval);
    else
        retval = QString(sa.GetStatusMessage().c_str());

    return retval;
}

