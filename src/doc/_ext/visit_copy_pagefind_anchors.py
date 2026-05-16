from bs4 import BeautifulSoup

def copy_section_ids_to_headings(app, pagename, templatename, context, doctree):
    body = context.get("body")
    if not body:
        return

    soup = BeautifulSoup(body, "html.parser")
    changed = False

    for section in soup.find_all("section", id=True):
        heading = section.find(["h1", "h2", "h3", "h4", "h5", "h6"], recursive=False)
        if heading and not heading.get("id"):
            heading["id"] = section["id"]
            changed = True

    if changed:
        context["body"] = str(soup)

def setup(app):
    app.connect("html-page-context", copy_section_ids_to_headings)
    return {
        "version": "1.0",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
