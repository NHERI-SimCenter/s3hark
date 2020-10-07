include ./Makefile.in

# -------------------------
s3hark: ./SiteResponse/s3hark.cpp $(FEMlib)
	make libs
	@$(CXX) $(CXXOPTFLAG) $(LINCLUDE) $(MINCLUDE) ./SiteResponse/s3hark.cpp $(s3harklib) $(FEMlib) $(FEMlib) $(NUMLIBS) -o $(source)/bin/s3hark
	echo "s3hark Compiled"

siteResponse: ./SiteResponse/Main.cpp $(FEMlib)
	make libs
	@$(CXX) $(CXXOPTFLAG) $(LINCLUDE) $(MINCLUDE) ./SiteResponse/Main.cpp $(SRTlib) $(FEMlib) $(NUMLIBS) -o $(source)/bin/siteresponse
	echo "FEMSRTCompiled"

fem:
	make tidy
	make siteResponse

$(FEMlib):
	@ make libs

archive: $(OBJS)
	ar rv $(FEMlib) $(OBJS)

libs:
	(mkdir -p bin)
	(mkdir -p lib)
	(cd FEM; make archive)
	(cd SiteResponse; make archive)
	(cd UI; make archive)

clean:
	rm -f $(FEMlib)
	rm -f $(SRTlib)
	(cd FEM; make clean)
	(cd SiteResponse; make clean)
	(cd UI; rm -rf *.o)
	(cd bin; rm -rf s3hark; rm -rf *.dSYM)
	
tidy:
	rm -f $(source)/bin/siteresponse
	rm -f $(source)/lib/*.a
	make clean

install: siteResponse
	cp $(source)/bin/siteresponse $(HOME)/bin/.

.PHONY: siteResponse
