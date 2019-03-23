USE_OPENMP=0
#MAX_DIGITS=100

CFLAGS := -O3 -march=native
LDLIBS := -lgmp

ifeq ($(USE_OPENMP), 1)
	CFLAGS += -DUSE_OPENMP -fopenmp
	LDLIBS += -lpthread
endif

ifdef MAX_DIGITS
	CFLAGS += -DMAX_DIGITS=$(MAX_DIGITS)
endif

all: persistence

persistence:

clean:
	rm -f persistence
