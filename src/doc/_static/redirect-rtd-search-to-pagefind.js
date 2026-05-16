(function () {
  function findSearchForm() {
    return (
      document.querySelector("form[action$='search.html']") ||
      document.querySelector("form[action*='search']") ||
      document.querySelector(".wy-side-nav-search form")
    );
  }

  function siteRootFromPath() {
    // Local examples:
    //   /_build/index.html
    //   /_build/using_visit/foo.html
    //
    // RTD examples:
    //   /en/latest/index.html
    //   /en/v3.4.2/using_visit/foo.html

    const path = window.location.pathname;

    const buildIndex = path.indexOf("/_build/");
    if (buildIndex !== -1) {
      return path.slice(0, buildIndex + "/_build/".length);
    }

    const parts = path.split("/").filter(Boolean);
    if (parts.length >= 2 && parts[0] === "en") {
      return "/en/" + parts[1] + "/";
    }

    return "/";
  }

  document.addEventListener("DOMContentLoaded", function () {
    const form = findSearchForm();
    if (!form) return;

    form.addEventListener("submit", function (event) {
      const input =
        form.querySelector("input[name='q']") ||
        form.querySelector("input[type='search']") ||
        form.querySelector("input[type='text']");

      if (!input || !input.value.trim()) return;

      event.preventDefault();

      const root = siteRootFromPath();
      const q = encodeURIComponent(input.value.trim());

      window.location.href = root + "pagefind-search.html?q=" + q;
    });
  });
})();
