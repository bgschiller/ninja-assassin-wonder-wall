presentation.pdf: probplot.png presentation.tex digraphs
	pdflatex --shell-escape presentation.tex

digraphs:
	dot 6ndag.dot -Tpng -o 6ndag.png
	neato 6ndigraph-cyc.dot -Tpng -o 6ndigraph-cyc.png
	neato 6ndigraph.dot -Tpng -o 6ndigraph.png
	neato 6ngraph.dot -Tpng -o 6ngraph.png
	dot alg2ex1.dot -Tpdf -o alg2ex1.pdf
	dot alg2ex2.dot -Tpdf -o alg2ex2.pdf
	dot alg2ex3.dot -Tpdf -o alg2ex3.pdf
	neato alg_ex1.dot -Tpdf -o alg_ex1.pdf
	neato alg_ex2.dot -Tpdf -o alg_ex2.pdf
	neato alg_ex3.dot -Tpdf -o alg_ex3.pdf

probplot.png: probability.dat
	gnuplot probplot.gpl


