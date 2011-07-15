Most users launch VisIt as:
\begin{verbatim}
% visit -cli
\end{verbatim}
In this case, all functions are immediately available.

However, some users launch a standard Python interpreter and then import
VisIt, like:
\begin{verbatim}
% python
>>> from visit import *    # either this way
>>> import visit           # or this way
\end{verbatim}

In this case, most of VisIt's functions are not loaded, since VisIt itself
has not been launched.  (When Launch is called, then all of the functions
will be available.  They are described in chapter \ref{chapter:functions}).

This chapter documents the functions available before Launch() is called 
when importing VisIt from a Python interpreter.

\newpage
