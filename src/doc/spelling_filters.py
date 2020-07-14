import re
from enchant.tokenize import Filter, tokenize, unit_tokenize

class list_tokenize(tokenize):
    def __init__(self, text):
        tokenize.__init__(self, '')
        self._words = text 
    def __next__(self):
        if not self._words:
            raise StopIteration()
        word = self._words.pop(0)
        return (word, 0)

    next = __next__ # for python 2 support


class VisItPythonSymbolFilter(Filter):
    """If a word looks like a VisIt Python symbol, split it into sub-tokens
       to check spelling of the sub-tokens"""

    def _split(self, word):
        return list_tokenize([ x for x in re.split("([A-Z]{1}[a-z0-9]*)?",word) if x != ''])
