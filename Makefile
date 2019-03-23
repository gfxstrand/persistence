# Copyright © 2019 Jason Ekstrand
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
