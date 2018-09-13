CC         = gcc
CFLAGS     = -O3

TARGET = membw_useavx membw

all: $(TARGET)

clean:
	rm -f $(TARGET)

#
# This section of the Makefile is for compiling the binaries
#
							
membw_useavx: membw.c 
	$(CC) $(CFLAGS) $^ -mavx2 -DUSE_AVX -o $@
							
membw: membw.c 
	$(CC) $(CFLAGS) $^ -mavx2 -o $@
