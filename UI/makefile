include ../Makefile.in

OBJS =  \
       ../SiteResponse/EffectiveFEModel.o \
       ../SiteResponse/siteLayering.o \
       ../SiteResponse/soillayer.o \
       ../SiteResponse/outcropMotion.o \
       ../FEM/StandardStream.o \
	   ../FEM/FileStream.o \
	   ../FEM/OPS_Stream.o \
	   SiteResponse.o

archive: $(OBJS)
	ar rv $(s3harklib) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f *.a

