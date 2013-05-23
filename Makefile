all:
	cd writing && $(MAKE) all
	cd cpp_implementation && $(MAKE)

animation: anim.gif
	gifsicle -O3 anim.gif -o animation.gif

anim.gif: anim.dat
	gnuplot < anim.dat

anim.dat: animate.py
	python animate.py > anim.dat
