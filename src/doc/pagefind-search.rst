Search
======

VisIt_ uses PageFind search.

.. raw:: html

   <link href="pagefind/pagefind-ui.css" rel="stylesheet">
   <script src="pagefind/pagefind-ui.js"></script>

   <div id="pagefind-search"></div>

   <script>
   window.addEventListener('DOMContentLoaded', function () {
     new PagefindUI({
       element: "#pagefind-search",
       showSubResults: true,
       showImages: false,
       highlightParam: "highlight"
     });

     const params = new URLSearchParams(window.location.search);
     const q = params.get("q");
     if (q) {
       const input = document.querySelector("#pagefind-search input");
       if (input) {
         input.value = q;
         input.dispatchEvent(new Event("input", { bubbles: true }));
       }
     }
   });
   </script>
