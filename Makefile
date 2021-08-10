MODULE = socnet$(shell python3-config --extension-suffix)

all: $(MODULE)

$(MODULE):
	(cd src; make)

sdev:
	python3 setup.py sdist

clean:
	rm -fr $(MODULE) var socnet.egg-info dist build
	(cd src; make clean)