
SHELL = /bin/sh

all: decomp_kernels.pdf Hn.pdf Hnw.pdf Hnv.pdf


decomp_kernels.pdf: clean decomp_kernels.tex refs.bib
    pdflatex decomp_kernels
    bibtex decomp_kernels
    pdflatex decomp_kernels
    pdflatex decomp_kernels

Hn.pdf: clean Hn.tex
    pdflatex Hn
    pdflatex Hn
    pdflatex Hn

Hnw.pdf: clean Hnw.tex
    pdflatex Hnw
    pdflatex Hnw
    pdflatex Hnw


Hnv.pdf: clean Hnv.tex
    pdflatex Hnv
    pdflatex Hnv
    pdflatex Hnv

clean:
    if (rm -f *.log *.aux *.bbl *.blg *.toc *.dvi *.ps *.brf *.lbl *pdf) then :; fi

