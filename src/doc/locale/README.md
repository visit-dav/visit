# Sphinx translations (`locale/`)

This folder holds Sphinx gettext catalogs for non-English translations of the
VisIt user manual.

Layout (example for Spanish):

`src/doc/locale/es/LC_MESSAGES/<docname>.po`

Sphinx loads compiled catalogs (`.mo`) at build time. This repo commits the
compiled `.mo` files alongside the `.po` sources for translated pages.

Read the Docs can be configured to build separate translation projects per
language; the Sphinx config (`src/doc/conf.py`) reads `READTHEDOCS_LANGUAGE`
and loads the corresponding catalogs from `locale/`.
