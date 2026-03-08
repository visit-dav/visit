# Spanish docs (Sphinx gettext/i18n) — experimental

This directory contains an **experimental** Spanish translation workflow using Sphinx’s
gettext/i18n support (`.pot`/`.po` catalogs). Unlike `src/doc/es/`, this approach does
**not** create translated `.rst` source files.

## Build message templates (`.pot`)

From `src/doc/es2`:

```bash
python3 -m sphinx -c . -b gettext source /tmp/visit-es2-gettext
```

## Edit translations (`.po`)

Translations live under:

`src/doc/es2/locale/es/LC_MESSAGES/`

## Build Spanish HTML

From `src/doc/es2`:

```bash
python3 -m sphinx -c . -D language=es -b html source /tmp/visit-es2-html
```
