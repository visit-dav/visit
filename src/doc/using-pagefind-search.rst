.. _using_pagefind_search:

Using Pagefind Search
=====================

When multiple terms are specified, unquoted, Pagefind first treats the search as an ``AND`` operation on the terms.
If any term, however, is found nowhere in Pagefind's index of the pages, it will ignore that term.
Pagefind search is **NOT** case sensitive.
To search for verbatim text, including case, enclose the search string in double quotes.

VisIt's Pagefind search will make a best effort to link search results to specific text fragments matching the search terms.
However, the algorithm is not perfect.
When it fails, a Pagefind search result may take you only to the nearest section heading containing text matching the search terms.
In the worst cases, the nearest section heading may be the top of the page.
You may then need to use the browser's search feature to search forward from there to find the matching text.
If the matching text occurrs in tabbed or collapsible sections, you may need to peek into those sections manually.

If you feel like you are routinely encountering situations where these kinds of search result link failures happen, please :ref:`reach out <methods_of_contact>` to the VisIt_ team.
