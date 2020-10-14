# include local arch. particularities

include $(PVM_ROOT)/conf/$(PVM_ARCH).def

CFLAGS	=	-I$(PVM_ROOT)/include $(ARCHCFLAGS)

LIBS	=	-lpvm3 -lgpvm3 $(ARCHLIB) -lm
LFLAGS	=	-L$(PVM_ROOT)/lib/$(PVM_ARCH)
LDFLAGS	=	$(LFLAGS) $(LIBS)

# dependances go here

.c:
	$(CC) -Wall $(CFLAGS) -o $@ $< $(LDFLAGS)
	mv $@ $(HOME)/pvm3/bin/$(PVM_ARCH)
