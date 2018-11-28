include ../Makefile.in

OBJS =  \
       FEModel.o \
       soillayer.o \
       siteLayering.o \
       outcropMotion.o

archive: $(OBJS)
	ar rv $(SRTlib) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f *.a

