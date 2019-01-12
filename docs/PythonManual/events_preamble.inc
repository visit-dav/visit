This chapter shows a table with all events that the VisIt GUI could potentially
generate. Different plugins create different events, so the list
will depend on the user configuration. The list in this section is generated
from a call to the {\it GetCallbackNames()} function and will therefore list
just the events that are applicable to the user that generates this documentation.

The list is alphabetically ordered. The left column, labeled {\it EventName}
displays each event or callback name. The right column, labeled {\it ArgCount}
displays the result of calling {\it GetCallbackArgumentCount(EventName) }
for the corresponding event, which returns the number of arguments a callback function
for that event should accept.

