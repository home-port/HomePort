# Copyright 2013 Aalborg University. All rights reserved.
#  
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY Aalborg University ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Aalborg University OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
# 
# The views and conclusions contained in the software and
# documentation are those of the authors and should not be interpreted
# as representing official policies, either expressed.

# Settings
CFLAGS := -c -Wall -g -DDEBUG $(CFLAGS)
EXEC := homeport
MODULES := libWebserver libHTTP libREST

# Vars that will be populated
LIBS :=
SRC :=
TESTS :=

.PHONY : all
all: $(EXEC)

# Modules, as according to:
# Miller, P. (1998). Recursive make considered harmful.
# AUUGN Journal of AUUG Inc, 19(1), 14-25.
CFLAGS += $(patsubst %, -I%/src, $(MODULES)) $(patsubst %, -I%/include, $(MODULES))
include $(patsubst %, %/module.mk,$(MODULES))
OBJ := $(patsubst %.c,%.o, $(filter %.c,$(SRC)))
LDFLAGS := $(patsubst %, -l%, $(LIBS)) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

$(EXEC): $(OBJ) main.o
	$(CC) -o $(EXEC) $(OBJ) main.o $(LDFLAGS)

.PHONY : mrproper
mrproper:
	make clean
	make
 
.PHONY : run
run:
	./$(EXEC)
 
.PHONY : debug
debug:
	gdb -ex run ./$(EXEC)
 
.PHONY : memcheck
memcheck:
	valgrind ./$(EXEC)

$(TESTS): $(OBJ) $(patsubst %, %.o, $(TESTS))
	$(CC) -o $@ $(filter-out $(patsubst %_test, %.o, $@), $(OBJ)) $(patsubst %, %.o, $@) $(LDFLAGS)

.PHONY : test
test: $(TESTS)
	for t in $(TESTS); do \
      ./$$t; \
   done

.PHONY : clean
clean:
	$(RM) $(OBJ) main.o homeport
	$(RM) -r doc/*
	$(RM) $(TESTS) $(patsubst %, %.o, $(TESTS))

.PHONY : doc
doc:
	doxygen doxygen.conf

