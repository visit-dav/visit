:orphan:

Using Pagefind Search
=====================

When multiple terms are given, unquoted, Pagefind first treats the search as an ``AND`` operation on the terms.
However, if any term given is found nowhere in Pagefind's index of the pages, it will delete that term from the search.
Pagefind search is **NOT** case sensitive.
To search for verbatim text, including case-sensitivity, enclose the search string in double quotes (``"``).

VisIt_'s Pagefind search will make a best effort to link search results to specific text fragments matching the search terms.
However, the algorithm is not perfect.
When it fails, a Pagefind search result may take you only to the nearest section heading containing text matching the search terms.
In the worst cases, the nearest section heading may be the top of the page and far away from the matching text.
In that case, you may then need to use the browser's search feature to find the specific matching text.
Unfortunately, if the matching text occurs in tabbed or collapsible sections that are not displayed by default when the page is loaded, you may need to peek into those sections manually.

If you feel like search results routinely fail to link to relevant places in the documentation, please :ref:`reach out <methods_of_contact>` to the VisIt_ team.
