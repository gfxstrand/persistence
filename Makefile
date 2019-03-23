USE_OPENMP=0

CFLAGS := -O3 -march=native
LDLIBS := -lgmp

ifeq ($(USE_OPENMP), 1)
	CFLAGS += -DUSE_OPENMP -fopenmp
	LDLIBS += -lpthread
endif

all: persistence

persistence:

clean:
	rm -f persistence
