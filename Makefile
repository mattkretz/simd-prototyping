all: check

CXXFLAGS+=-std=gnu++23 -Wall -Wextra -Wno-psabi -O2 -g0 -fconcepts-diagnostics-depth=3
#-D_GLIBCXX_DEBUG_UB=1

testarchs=athlon64 \
	  nocona \
	  core2 \
	  westmere \
	  ivybridge \
	  bdver4 \
	  skylake \
	  znver4 \
	  skylake-avx512

tests=$(patsubst tests/%.cpp,%,$(wildcard tests/*.cpp))

testtypes="signed-char" \
	  "unsigned-char" \
	  "signed-short" \
	  "unsigned-short" \
	  "signed-int" \
	  "unsigned-int" \
	  "signed-long" \
	  "unsigned-long" \
	  "signed-long-long" \
	  "unsigned-long-long" \
	  "float" \
	  "double" \
	  "std--float16_t" \
	  "std--float32_t" \
	  "std--float64_t" \
	  "char" \
	  "char8_t" \
	  "wchar_t" \
	  "char16_t" \
	  "char32_t" \
	  "std--byte"

getwidth = $(subst .,,$(suffix $(1)))
gettype = $(subst -, ,$(subst --,::,$(subst .,,$(basename $(notdir $(1))))))
getarch = $(subst .,,$(suffix $(subst /,,$(dir $(1)))))
gettest = $(basename $(subst /,,$(dir $(1))))

debug:
	@echo "$(testarchs)"
	@echo "$(tests)"
	@for i in $(testtypes); do echo "- $$i"; done
	@echo "width=$(call getwidth,shift_left.core2/signed-char.34)"
	@echo "type=$(call gettype,shift_left.core2/signed-char.34)"
	@echo "arch=$(call getarch,shift_left.core2/signed-char.34)"
	@echo "test=$(call gettest,shift_left.core2/signed-char.34)"
	@echo "type=$(call gettype,shift_left.core2/std--float32_t.34)"

.NOTINTERMEDIATE: obj/%.exe

Makefile.depend: Makefile constexpr_tests.c++ \
  $(wildcard codegen/*.c++) $(wildcard *.h)
	@echo "Updating dependencies"
	@mkdir -p obj
	@echo > $@
	@constexprtargets=""; \
	helptargets=""; \
	for t in $(tests); do \
	  for arch in $(testarchs); do \
	    mkdir -p obj/$$t.$$arch; \
	    runtargets=""; \
	    for types in $(testtypes); do \
	      for width in $$(seq 1 67); do \
	        runtargets="obj/$$t.$$arch/$$types.$$width.o $$runtargets"; \
	        echo "check-$$arch.$$types.$$width: check/$$t.$$arch/$$types.$$width\n"; \
	        echo "check-$$t.$$arch.$$types: check/$$t.$$arch/$$types.$$width\n"; \
	        echo "check-$$t.$$types.$$width: check/$$t.$$arch/$$types.$$width\n"; \
	        echo "check-$$types.$$width: check/$$t.$$arch/$$types.$$width\n"; \
	      done; \
	      echo "check-$$t.$$arch: check-$$t.$$arch.$$types\n"; \
	      echo "check-$$t.$$types: check-$$t.$$arch.$$types\n"; \
	    done; \
	    helptargets="check-$$t.$$arch $$helptargets"; \
	    echo "check-$$t: check-$$t.$$arch\n"; \
	    $(CXX) -std=gnu++23 -MM -MT "$$runtargets" tests/$$t.cpp; \
	    echo; \
	  done; \
	  for types in $(testtypes); do \
	    helptargets="check-$$t.$$types $$helptargets"; \
	  done; \
	done >> $@; \
	for types in $(testtypes); do \
	  for width in $$(seq 1 67); do \
	    helptargets="check-$$types.$$width $$helptargets"; \
	  done; \
	done; \
	for arch in $(testarchs); do \
	  echo ".PHONY: check-$$arch-run check-$$arch-constexpr\n"; \
	  echo "check: check-$$arch\n"; \
	  echo "check-$$arch: check-$$arch-run \n"; \
	  for types in $(testtypes); do \
	    for width in $$(seq 1 67); do \
	      echo "check-$$arch-run: check-$$arch.$$types.$$width\n"; \
	      helptargets="check-$$arch.$$types.$$width $$helptargets"; \
	      echo ".PHONY: check-$$arch.$$types.$$width\n"; \
	    done; \
	  done; \
	  constexprtargets="obj/constexpr.$$arch.s $$constexprtargets"; \
	  echo "check-$$arch-constexpr: obj/constexpr.$$arch.s\n"; \
	  echo "check-$$arch: check-$$arch-constexpr\n"; \
	  echo "help: help-check-$$arch\n\nhelp-check-$$arch:\n\t@echo \"... check-$$arch\"\n\t@echo \"... check-$$arch-constexpr\"\n\t@echo \"... check-$$arch-run\""; \
	done >> $@; \
	$(CXX) -std=gnu++23 -MM -MT "$$constexprtargets" constexpr_tests.c++ >> $@; \
	echo "help: help-individual-targets\n\nhelp-individual-targets:" >> $@; \
	for ht in $$helptargets; do \
		echo "\t@echo \"... $$ht\""; \
	done >> $@
	@$(CXX) -std=gnu++23 -MM -MT obj/codegen.simd_cat.s codegen/simd_cat.c++ >> $@
	@$(CXX) -std=gnu++23 -MM -MT obj/codegen.reduce.s codegen/reduce.c++ >> $@

check: Makefile.depend \
  check-simd_cat \
  check-reduce

obj/constexpr.%.s:
	@echo "Building constexpr tests for $*"
	@$(CXX) $(CXXFLAGS) -march=$* -S -o $@ constexpr_tests.c++

obj/%.o:
	@echo "Building runtime tests for $*"
	@$(CXX) $(CXXFLAGS) -march=$(call getarch,$*) \
		-D UNITTEST_TYPE="$(call gettype,$*)" \
		-D UNITTEST_WIDTH=$(call getwidth,$*) \
		-c -o $@ tests/$(call gettest,$*).cpp
	@echo "Building runtime tests for $* done"

REPORTFLAGS=-fmem-report -ftime-report -Q

obj/%.report:
	@echo "Building build time reports for $*"
	@$(CXX) $(CXXFLAGS) $(REPORTFLAGS) -march=$(call getarch,$*) \
		-D UNITTEST_TYPE="$(call gettype,$*)" \
		-D UNITTEST_WIDTH=$(call getwidth,$*) \
		-c -o $@ tests/$(call gettest,$*).cpp
	@echo "Building build time reports for $* done"

obj/%.exe: obj/%.o
	@echo "Linking runtime tests for $*"
	@$(CXX) $(CXXFLAGS) -march=$(call getarch,$*) -o $@ $<
	@echo "Linking runtime tests for $* done"

check/%: obj/%.exe
	@echo "Executing runtime tests for $*"
	obj/$*.exe

check-simd_cat: obj/codegen.simd_cat.s
	@echo "Testing for expected instructions in $<"
	@grep -A3 '^f0(' $< | grep "vinserti128	ymm0, ymm0, xmm1, 1"
	@grep -A3 '^f1(' $< | grep "vinsertf128	ymm0, ymm0, xmm1, 1"
	@grep -A3 '^f2(' $< | grep "vmovlhps	xmm0, xmm0, xmm1"

check-reduce: obj/codegen.reduce.s
	@echo "Testing for expected instructions in $<"
	@grep -A1 '^f0(' $< | tail -n1 | grep --color=auto "vphaddd"
	@grep -A2 '^f0(' $< | tail -n1 | grep --color=auto "vphaddd"
	@grep -A3 '^f0(' $< | tail -n1 | grep --color=auto "vmovd	eax"
	@grep -A4 '^f0(' $< | tail -n1 | grep --color=auto "ret"
	@grep -A1 '^f1(' $< | tail -n1 | grep --color=auto "vhaddps"
	@grep -A2 '^f1(' $< | tail -n1 | grep --color=auto "vhaddps"
	@grep -A3 '^f1(' $< | tail -n1 | grep --color=auto "ret"
	@grep -A1 '^f2(' $< | tail -n1 | grep --color=auto "vhaddpd"
	@grep -A2 '^f2(' $< | tail -n1 | grep --color=auto "ret"
	@grep -A1 '^f3(' $< | tail -n1 | grep --color=auto "vphaddw"
	@grep -A2 '^f3(' $< | tail -n1 | grep --color=auto "vphaddw"
	@grep -A3 '^f3(' $< | tail -n1 | grep --color=auto "vphaddw"
	@grep -A4 '^f3(' $< | tail -n1 | grep --color=auto "vpextrw	eax"
	@grep -A5 '^f3(' $< | tail -n1 | grep --color=auto "ret"
	@grep -A2 '^f4(' $< | tail -n2 | grep --color=auto "vphaddw"
	@grep -A3 '^f4(' $< | tail -n2 | grep --color=auto "vphaddw"
	@grep -A4 '^f4(' $< | tail -n2 | grep --color=auto "vpextrw	eax"
	@grep -A5 '^f4(' $< | tail -n2 | grep --color=auto "ret"

obj/codegen.%.s: codegen/%.c++
	@echo "Building $@"
	@$(CXX) $(CXXFLAGS) -masm=intel -march=skylake -S -o $@ $<
	@cat $@ | grep -v '^\s*\.' | c++filt > $@.tmp
	@mv $@.tmp $@

help: Makefile.depend
	@echo "... check"
	@echo "... check-reduce"
	@echo "... check-simd_cat"

include Makefile.depend

clean:
	rm -f obj/*.o obj/*.exe obj/*.s

.PHONY: check clean help check-reduce check-simd_cat
