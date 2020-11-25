# This file is part of hwzip from https://www.hanshq.net/zip.html
# It is put in the public domain; see the LICENSE file for details.

# Note: Before using this Makefile, you probably want to tweak some flags
# below. In particular, you may not want to use -Weverything.
#
# Don't forget to pass OPT=1 for optimized builds.


CC = clang
LD = $(CC)

WFLAGS = -Weverything -Wno-missing-prototypes -Wno-missing-noreturn \
         -Wno-overlength-strings -Wno-language-extension-token -Wno-padded \
         -Wno-c99-extensions

CFLAGS  = -std=gnu90 -MMD $(WFLAGS)
LDFLAGS =

ifeq ($(WERROR),1)
WFLAGS += -Werror
endif

ifeq ($(OPT),1)
CFLAGS += -O3 -DNDEBUG -march=native -Wno-unused-macros
else
CFLAGS += -O0 -g
endif

ifeq ($(ASAN),1)
CFLAGS  += -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all
LDFLAGS += -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all
endif

ifeq ($(VALGRIND),1)
Vgrnd=valgrind --quiet --error-exitcode=1 --leak-check=full
else
Vgrnd=
endif

ifeq ($(COVERAGE),1)
CFLAGS  += -fprofile-instr-generate -fcoverage-mapping
LDFLAGS += -fprofile-instr-generate
GenCov = llvm-profdata merge default.profraw -o default.profdata && \
         llvm-cov show -format html -instr-profile default.profdata \
         -o cov -show-line-counts-or-regions
GenCovCheck = $(GenCov) tests
GenCovBench = $(GenCov) benchmarks
else
GenCovCheck =
GenCovBench =
endif


# Source files shared between targets.
CORESRC  = crc32.c
CORESRC += deflate.c
CORESRC += huffman.c
CORESRC += lz77.c
CORESRC += tables.c
CORESRC += zip.c

# Source files for the 'hwzip' target.
HWZIPSRC  = $(CORESRC)
HWZIPSRC += hwzip.c

# Source files for the 'tests' target.
TESTSRC  = $(CORESRC)
TESTSRC += bits_test.c
TESTSRC += bitstream_test.c
TESTSRC += crc32_test.c
TESTSRC += deflate_test.c
TESTSRC += hamlet.c
TESTSRC += huffman_test.c
TESTSRC += inflate_test.c
TESTSRC += lz77_test.c
TESTSRC += test_utils.c
TESTSRC += tests.c
TESTSRC += zip_test.c
TESTSRC += zlib_utils.c

# Source files for the 'benchmarks' target.
BENCHSRC  = $(CORESRC)
BENCHSRC += benchmarks.c
BENCHSRC += deflate_bench.c
BENCHSRC += hamlet.c
BENCHSRC += huffman_bench.c
BENCHSRC += inflate_bench.c
BENCHSRC += lz77_bench.c
BENCHSRC += zlib_bench.c
BENCHSRC += zlib_utils.c


ALLSRC = $(HWZIPSRC) $(TESTSRC) $(BENCHSRC) generate_tables.c

all : hwzip tests benchmarks

check : tests
	$(Vgrnd) ./tests
	$(GenCovCheck)

bench : benchmarks
	$(Vgrnd) ./benchmarks
	$(GenCovBench)

%.o : %.c .flags
	$(CC) -c $< -o $@ $(CFLAGS)

tables.c : generate_tables.c .flags
	$(CC) -c $< -o generate_tables.o $(CFLAGS)
	$(LD) -o generate_tables generate_tables.o $(LDFLAGS)
	./generate_tables > $@

hamlet.c : hamlet.txt
	echo '#include "hamlet.h"' > $@
	echo 'const uint8_t hamlet[] = {' >> $@
	xxd -i < $< >> $@
	echo '};' >> $@

hwzip : $(HWZIPSRC:.c=.o)
	$(LD) -o $@ $^ $(LDFLAGS)

tests : $(TESTSRC:.c=.o)
	$(LD) -o $@ $^ $(LDFLAGS) -lz

benchmarks : $(BENCHSRC:.c=.o)
	$(LD) -o $@ $^ $(LDFLAGS) -lz

clean :
	rm -f $(ALLSRC:.c=.o) $(ALLSRC:.c=.d)
	rm -f hwzip tests benchmarks deflate_fuzzer inflate_fuzzer zip_fuzzer
	rm -f generate_tables hamlet.c
	rm -f default.profdata default.profdata default.profraw
	rm -rf cov
	rm -f .flags

.flags : dummy
	@{ \
          T=`mktemp`; \
          echo $(CC) $(CFLAGS) $(LD) $(LDFLAGS) > $$T; \
          cmp -s $$T $@ || mv -f $$T $@; \
        }

-include $(ALLSRC:.c=.d)

.PHONY : all check clean dummy
