FILE = man
SOURCE = $(FILE).tex 
OBJ = $(FILE).dvi

$(OBJ): makefile $(SOURCE) 
	latex $(SOURCE)

pdf::
	dvipdf $(FILE).dvi

clean::
	rm -f $(OBJ) $(FILE).aux $(FILE).bbl $(FILE).log $(FILE).pdf
