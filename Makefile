all: check

CXXFLAGS=-std=gnu++23 -Wall -Wextra -O2 -g0 -fconcepts-diagnostics-depth=3

Makefile.depend: Makefile constexpr_tests.c++ codegen/*.c++ *.h
	@echo "Updating dependencies"
	@mkdir -p obj
	@rm -f $@
	@$(CXX) -std=gnu++23 -MM -MT obj/tests.core2.s constexpr_tests.c++ >> $@
	@$(CXX) -std=gnu++23 -MM -MT obj/tests.skylake.s constexpr_tests.c++ >> $@
	@$(CXX) -std=gnu++23 -MM -MT obj/tests.skylake-avx512.s constexpr_tests.c++ >> $@
	@$(CXX) -std=gnu++23 -MM -MT obj/codegen.simd_cat.s codegen/simd_cat.c++ >> $@
	@$(CXX) -std=gnu++23 -MM -MT obj/codegen.reduce.s codegen/reduce.c++ >> $@

check: Makefile.depend \
  check-core2 check-skylake check-skylake-avx512 \
  check-simd_cat \
  check-reduce

check-core2: obj/tests.core2.s
check-skylake: obj/tests.skylake.s
check-skylake-avx512: obj/tests.skylake-avx512.s

obj/tests.%.s:
	@echo "Building constexpr tests for $*"
	@$(CXX) $(CXXFLAGS) -march=$* -S -o $@ constexpr_tests.c++

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

include Makefile.depend

help:
	@echo "... check"
	@echo "... check-core2"
	@echo "... check-skylake"
	@echo "... check-skylake-avx512"
	@echo "... check-reduce"
	@echo "... check-simd_cat"
