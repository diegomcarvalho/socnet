MODULE = socnet$(shell python3-config --extension-suffix)

all: $(MODULE)

$(MODULE):
	(cd src; make)

build: clean all

doc:
	doxygen Doxyfile

sdev:
	python3 setup.py sdist

clean:
	rm -fr $(MODULE) var socnet.egg-info dist build html latex
	(cd src; make clean)