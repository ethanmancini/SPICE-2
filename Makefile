CC = gcc
CFLAGS = -g -O0 -Wno-error
FC = gfortran
FFLAGS = -g -O0 -finteger-4-integer-8 -std=legacy

OBJS = spice.o unix.o

all: spice

spice: $(OBJS)
	$(FC) $(OBJS) -o spice

%.o: %.f
	$(FC) -c $(FFLAGS) $*.f -o $*.o 

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f $(OBJS) spice

