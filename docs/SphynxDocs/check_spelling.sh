#!/bin/sh

# Make a temporary dir for the adjusted conf.py file
tempfoo=$(basename $0)
TMPDIR=$(mktemp -d -t ${tempfoo}) || exit 1

#
# Copy conf.py and sed-replace the extensions variable
#
cat conf.py | sed \
    -e "s/^extensions[[:space:]]*=[[:space:]]*\[\(.*\)\]/extensions = [\1, 'sphinxcontrib.spelling']/" \
    -e "s/^    app.add_directive('spelling', SpellingDirective)//" \
    > ${TMPDIR}/conf.py

#
# Run Sphinx with spell-check builder
#
sphinx-build -c ${TMPDIR} -b spelling . _spelling
rm -rf ${TMPDIR}
