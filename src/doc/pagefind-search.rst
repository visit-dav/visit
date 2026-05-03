Search
======

VisIt_ uses :ref:`Pagefind search <using_pagefind_search>`, a client-side search solution for static pages.
If you are encountering issues with search, please :ref:`reach out <methods_of_contact>` to the VisIt_ team.

.. raw:: html

   <link href="pagefind/pagefind-ui.css" rel="stylesheet">
   <script src="pagefind/pagefind-ui.js"></script>

   <div id="pagefind-search"></div>

   <script>
   //
   // By default, Pagefind's search results page will produce links to the nearest
   // section heading containing body text where the search string matched.
   //
   // Many of VisIt's documentation pages contain such long body text that the
   // user may have to read a lot or search quite a bit forward before finding
   // the relevant section of the body text where the search string matched.
   //
   // To address this and improve the user experience in other ways, we override
   // Pagefind's processResult() method to produce text fragment URLs,
   // https://en.wikipedia.org/wiki/URI_fragment. Some tests were performed on
   // Safari, Chrome and Firefox browsers to confirm that all support text fragments
   // though Firefox is by far the most adaptable.
   //
   // These URLs have the form...
   //
   //   https://somepage.htm#:~:text=matching%20initial%20text,matching%20end%20text
   //
   // When they work, text fragment URLs are perfect for this purpose because they
   // link directly to body text where the search string was matched. In addition,
   // they automatically highlight the matching text in the paage. But, in order
   // to work correctly, the text fragment URL must be a) properly encoded and 
   // b) match the rendered HTML in the body text in the page VERBATIM.
   //
   // Verbatim matching can be defeated by line breaks, paragraph breaks, auto-
   // numbering, &Xxx;-chars, tabbed or collapsible sections etc. in the
   // rendered HTML.
   //
   // The JavaScript logic below takes the `plain_excerpt` member of the `result`
   // object that Pagefind delivers to processResult and attempts to find the first
   // several words and last several words that do not cross a punctuation
   // boundary and uses these, after proper encoding, to produce the...
   //
   //  :~:text=first%20words,last%20words
   //
   // URL fragment. If it cannot find a suitably long (4) sequence of words at
   // beginning and end of `plain_excerpt` it falls back to default behavior of
   // using section headings.
   //
   // As a consequence of how we do business, the URLs that are emitted for search
   // results here contain BOTH the section heading id followed by the text fragment
   // if one is produced. Although we should really use just one or the other and not
   // both together, this seems to work fine in all browsers and removes the need to
   // parse out of the original URL Pagefind created just the id portion.
   //

   function firstLastWordsNoCrossPunctuation(str, n = 4) {
     const words = str.trim().split(" ").filter(Boolean);

     let i;
     let first = [];
     for (i = 0; i < words.length && first.length < n; i++) {
       if (/[.?:;!,]/.test(words[i])) { first = []; continue; }
       first.push(words[i]);
     }

     let last = [];
     for (let j = words.length - 1; j >= 0 && j > i && last.length < n; j--) {
       if (/[.?:;!,]/.test(words[j])) { last = []; continue; }
       last.unshift(words[j]);
     }

     if (first.length == n && last.length == n) {
       return { first: first.join(" "), last: last.join(" ") }
     }
     else {
       return { first: "", last: "" }
     }
   }

   // Handle some encoding necessary for text frag URLs (determined by trial-and-error)
   function encodeStr(str) {
      return str
            .replaceAll("&lt;", "<")
            .replaceAll("&gt;", ">")
            .replaceAll("&amp;", "&")
            .replaceAll(" ", "%20")
            .replaceAll(",", "%2C")
            .replaceAll("-", "%2D")
            .replaceAll("&", "%26");
   }

   // Try to make text frag URL...if it fails, fall back to default behavior
   function makeTextFragUrlFromExcerpt(excerptStr) {
     const parts = firstLastWordsNoCrossPunctuation(excerptStr);
     if (parts.first != "" && parts.last != "") {
       return "#:~:text="
              + encodeStr(parts.first)
              + "," 
              + encodeStr(parts.last);
     }
     else {
       return "";
     }
   }

   window.addEventListener('DOMContentLoaded', function () {
     new PagefindUI({
       element: "#pagefind-search",
       showSubResults: true,
       showImages: false,
       processResult: function(result) {
         console.log(result);
         if (result.plain_excerpt) {
             result.url = result.url + makeTextFragUrlFromExcerpt(result.plain_excerpt);
         }
         result.sub_results = result.sub_results.map(function(sub) {
           if (sub.plain_excerpt) {
             sub.url = sub.url + makeTextFragUrlFromExcerpt(sub.plain_excerpt);
           }
           return sub;
         });
         return result;
       }
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
