thesis_paper.pdf: probplot.png digraphs thesis_paper.tex
	pdflatex --shell-escape thesis_paper.tex
	pdflatex --shell-escape thesis_paper.tex #twice for \tableofcontents

all: thesis_paper.pdf presentation.pdf

presentation.pdf: probplot.png presentation.tex digraphs
	pdflatex --shell-escape presentation.tex
	pdflatex --shell-escape presentation.tex

digraphs: *.dot
	touch digraphs #this is an empty file, used to prevent this section from running unneccesarily
	dot 6ndag.dot -Tpng -o 6ndag.png
	neato 6ndigraph-cyc.dot -Tpng -o 6ndigraph-cyc.png
	neato 6ndigraph.dot -Tpng -o 6ndigraph.png
	neato 6ngraph.dot -Tpng -o 6ngraph.png
	neato 6ngraph-red.dot -Tpng -o 6ngraph-red.png
	dot alg2ex1.dot -Tpdf -o alg2ex1.pdf
	dot alg2ex2.dot -Tpdf -o alg2ex2.pdf
	dot alg2ex3.dot -Tpdf -o alg2ex3.pdf
	neato alg_ex1.dot -Tpdf -o alg_ex1.pdf
	neato alg_ex2.dot -Tpdf -o alg_ex2.pdf
	neato alg_ex3.dot -Tpdf -o alg_ex3.pdf

probplot.png: probability.dat
	gnuplot probplot.gpl


