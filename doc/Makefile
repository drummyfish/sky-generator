# makefile pro preklad LaTeX verze Bc. prace
# (c) 2008 Michal Bidlo
# E-mail: bidlom AT fit vutbr cz
#===========================================
# asi budete chtit prejmenovat:
CO = pgr
all: $(CO).pdf

pdf: $(CO).pdf

$(CO).ps: $(CO).dvi
	dvips $(CO)

$(CO).pdf: clean
	pdflatex --shell-escape $(CO)
	bibtex $(CO)
	pdflatex --shell-escape $(CO)
	pdflatex --shell-escape $(CO)

$(CO).dvi: $(CO).tex $(CO).bib
	latex $(CO)
	bibtex $(CO)
	latex $(CO)
	latex $(CO)
clean:
	rm -f *.dvi *.log $(CO).blg $(CO).bbl $(CO).toc *.aux $(CO).out $(CO).lof
	rm -f $(CO).pdf
	rm -f *~

pack:
	tar czvf bp-xjmeno.tar.gz *.tex *.bib *.bst ./fig/* ./cls/* Makefile Changelog
