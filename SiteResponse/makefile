include ../Makefile.in

OBJS =  \
       soillayer.o \
       siteLayering.o \
       outcropMotion.o \
       Mesher.o \
       EffectiveFEModel.o 

archive: $(OBJS)
	ar rv $(SRTlib) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f *.a

