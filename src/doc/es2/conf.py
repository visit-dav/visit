# Sphinx i18n (gettext) configuration for Spanish translations.
#
# Usage examples (from src/doc):
#   sphinx-build -c es2 -b gettext . /tmp/visit-manual-gettext
#   sphinx-build -c es2 -D language=es -b html . /tmp/visit-manual-es
#
# This file intentionally reuses the main configuration in ../conf.py and only
# overrides the i18n-related settings.

from __future__ import annotations

import os

_DOC_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
_BASE_CONF = os.path.join(_DOC_ROOT, "conf.py")
with open(_BASE_CONF, "r", encoding="utf-8") as _fp:
    exec(compile(_fp.read(), _BASE_CONF, "exec"), globals())

# es2 builds a minimal “test” Sphinx project under src/doc/es2/source to avoid
# requiring optional extensions used in the full manual build.
root_doc = "index"
master_doc = "index"

# Keep the es2 config self-contained: avoid third-party extensions/themes that
# may not be installed in minimal environments.
extensions = ["sphinx.ext.mathjax"]
html_theme = "alabaster"

# Point to the main manual's shared templates/static assets.
templates_path = [os.path.join(_DOC_ROOT, "_templates")]
html_static_path = [os.path.join(_DOC_ROOT, "_static")]

# i18n / gettext settings
language = "es"
gettext_compact = False
locale_dirs = [os.path.abspath(os.path.join(os.path.dirname(__file__), "locale"))]
