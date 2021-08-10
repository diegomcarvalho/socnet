MODULE = socnet$(shell python3-config --extension-suffix)

all: $(MODULE)

$(MODULE):
	(cd src; make)

clean:
	rm -f $(MODULE)
	(cd src; make clean)