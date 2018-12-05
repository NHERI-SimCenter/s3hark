include ./Makefile.in

# -------------------------





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

clean:
	rm -f $(FEMlib)
	rm -f $(SRTlib)
	(cd FEM; make clean)
	(cd SiteResponse; make clean)
	
tidy:
	rm -f $(source)/bin/siteresponse
	rm -f $(source)/lib/*.a
	make clean

install: siteResponse
	cp $(source)/bin/siteresponse $(HOME)/bin/.

.PHONY: siteResponse
