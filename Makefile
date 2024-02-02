all: check

CXXFLAGS+=-std=gnu++23 -Wall -Wextra -Wno-psabi -O2 -g0 -fconcepts-diagnostics-depth=3
#-D_GLIBCXX_DEBUG_UB=1

testarchs ::= athlon64 \
	     nocona \
	     core2 \
	     westmere \
	     ivybridge \
	     bdver4 \
	     skylake \
	     znver4 \
	     skylake-avx512

tests ::= $(patsubst tests/%.cpp,%,$(wildcard tests/*.cpp))

testtypes ::= signed-char \
	     unsigned-char \
	     signed-short \
	     unsigned-short \
	     signed-int \
	     unsigned-int \
	     signed-long \
	     unsigned-long \
	     signed-long-long \
	     unsigned-long-long \
	     float \
	     double \
	     std::float16_t \
	     std::float32_t \
	     std::float64_t \
	     char \
	     char8_t \
	     wchar_t \
	     char16_t \
	     char32_t \
	     std::byte

testtypes ::= $(subst ::,--,$(testtypes))

testwidths ::= $(shell seq 1 67)

getwidth = $(subst .,,$(suffix $(1)))
gettype = $(subst -, ,$(subst --,::,$(subst .,,$(basename $(notdir $(1))))))
getarch = $(subst .,,$(suffix $(subst /,,$(dir $(1)))))
gettest = $(basename $(subst /,,$(dir $(1))))

.PHONY: debug
debug:
	@echo "$(testarchs)"
	@echo "$(tests)"
	@echo "$(testwidths)"
	@for i in $(testtypes); do echo "- $$i"; done
	@echo "width=$(call getwidth,shift_left.core2/signed-char.34)"
	@echo "type=$(call gettype,shift_left.core2/signed-char.34)"
	@echo "arch=$(call getarch,shift_left.core2/signed-char.34)"
	@echo "test=$(call gettest,shift_left.core2/signed-char.34)"
	@echo "type=$(call gettype,shift_left.core2/std--float32_t.34)"

help/%:
	@echo "... $*"

define simple_check_template
check-$(1): $(2)

help-check-targets: help/check-$(1)

endef

define check_template
check-$(1):
	@$$(MAKE) --no-print-directory $$(shell shuf -e -- $(2))

help-check-targets: help/check-$(1)

endef

$(foreach arch,$(testarchs),\
	$(foreach type,$(testtypes),\
	$(foreach w,$(testwidths),\
	$(eval $(call check_template,$(arch).$(type).$(w),\
	$(foreach t,$(tests),\
	check/$(t).$(arch)/$(type).$(w)))))))

$(foreach t,$(tests),\
	$(eval $(call check_template,$(t),\
	$(foreach arch,$(testarchs),\
	$(foreach w,$(testwidths),\
	$(foreach type,$(testtypes),\
	check/$(t).$(arch)/$(type).$(w)))))))

$(foreach t,$(tests),\
	$(foreach arch,$(testarchs),\
	$(foreach type,$(testtypes),\
	$(eval $(call check_template,$(t).$(arch).$(type),\
	$(foreach w,$(testwidths),\
	check/$(t).$(arch)/$(type).$(w)))))))

$(foreach t,$(tests),\
	$(foreach type,$(testtypes),\
	$(eval $(call check_template,$(t).$(type),\
	$(foreach w,$(testwidths),\
	$(foreach arch,$(testarchs),\
	check/$(t).$(arch)/$(type).$(w)))))))

$(foreach t,$(tests),\
	$(foreach type,$(testtypes),\
	$(foreach w,$(testwidths),\
	$(eval $(call check_template,$(t).$(type).$(w),\
	$(foreach arch,$(testarchs),\
	check/$(t).$(arch)/$(type).$(w)))))))

$(foreach type,$(testtypes),\
	$(eval $(call check_template,$(type),\
	$(foreach arch,$(testarchs),\
	$(foreach w,$(testwidths),\
	$(foreach t,$(tests),\
	check/$(t).$(arch)/$(type).$(w)))))))

$(foreach type,$(testtypes),\
	$(foreach w,$(testwidths),\
	$(eval $(call check_template,$(type).$(w),\
	$(foreach arch,$(testarchs),\
	$(foreach t,$(tests),\
	check/$(t).$(arch)/$(type).$(w)))))))

$(foreach arch,$(testarchs),\
	$(eval $(call simple_check_template,constexpr-$(arch),obj/constexpr.$(arch).s)))

$(foreach arch,$(testarchs),\
	$(eval $(call check_template,$(arch),check-constexpr-$(arch) \
	$(foreach type,$(testtypes),\
	$(foreach w,$(testwidths),\
	check-$(arch).$(type).$(w))))))

checks ::= $(foreach t,$(tests),\
	$(foreach w,$(testwidths),\
	$(foreach type,$(testtypes),\
	$(foreach arch,$(testarchs),check/$(t).$(arch)/$(type).$(w)))))

constexpr_checks ::= $(foreach arch,$(testarchs),obj/constexpr.$(arch).s)

.PHONY: check-constexpr
check-constexpr:
	@$(MAKE) --no-print-directory $(shell shuf -e -- $(constexpr_checks))

.PHONY: check
check: Makefile.depend
	@$(MAKE) --no-print-directory $(shell shuf -e -- $(checks) check-simd_cat check-reduce $(constexpr_checks))

# too many:
#help-check-targets: $(subst check,help/check,$(checks))

.NOTINTERMEDIATE: $(subst check,obj,$(checks:=.exe))

.PHONY: help-check-targets

Makefile.depend: Makefile constexpr_tests.c++ $(wildcard codegen/*.c++) $(wildcard *.h)
	@echo "Updating dependencies"
	@for t in $(tests); do \
	  for arch in $(testarchs); do \
	    $(CXX) -std=gnu++23 -march=$$arch -MM -MT "obj/$$t.$$arch/%.exe" tests/$$t.cpp; \
	  done; \
	done > $@
	@$(CXX) -std=gnu++23 -MM -MT "$(foreach arch,$(testarchs),obj/constexpr.$(arch).s)" constexpr_tests.c++ >> $@
	@$(CXX) -std=gnu++23 -MM -MT obj/codegen.simd_cat.s codegen/simd_cat.c++ >> $@
	@$(CXX) -std=gnu++23 -MM -MT obj/codegen.reduce.s codegen/reduce.c++ >> $@

obj/constexpr.%.s:
	@echo "Build constexpr tests for $*"
	@$(CXX) $(CXXFLAGS) -march=$* -S -o $@ constexpr_tests.c++

REPORTFLAGS=-fmem-report -ftime-report -Q

obj/%.report:
	@echo "Build time reports for $*"
	@$(CXX) $(CXXFLAGS) $(REPORTFLAGS) -march=$(call getarch,$*) \
		-D UNITTEST_TYPE="$(call gettype,$*)" \
		-D UNITTEST_WIDTH=$(call getwidth,$*) \
		-c -o obj/$*.o tests/$(call gettest,$*).cpp
	@echo "Build time reports for $* done"

obj/%.exe:
	@mkdir -p obj/$(call gettest,$*).$(call getarch,$*)
	@echo "Build check/$*"
	@$(CXX) $(CXXFLAGS) -march=$(call getarch,$*) \
		-D UNITTEST_TYPE="$(call gettype,$*)" \
		-D UNITTEST_WIDTH=$(call getwidth,$*) \
		-c -o obj/$*.o tests/$(call gettest,$*).cpp
	@echo " Link check/$*"
	@$(CXX) $(CXXFLAGS) -march=$(call getarch,$*) -o $@ obj/$*.o
	@rm obj/$*.o

check/%: obj/%.exe
	@mkdir -p $(dir $@)
	@{ \
		echo "================================================================";\
		echo "  Run check/$*"; \
		echo "================================================================";\
		obj/$*.exe; \
	} | tee $@

.PHONY: check-simd_cat
check-simd_cat: obj/codegen.simd_cat.s
	@echo "Testing for expected instructions in $<"
	@grep -A3 '^f0(' $< | grep "vinserti128	ymm0, ymm0, xmm1, 1"
	@grep -A3 '^f1(' $< | grep "vinsertf128	ymm0, ymm0, xmm1, 1"
	@grep -A3 '^f2(' $< | grep "vmovlhps	xmm0, xmm0, xmm1"

.PHONY: check-reduce
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

.PHONY: help
help: Makefile.depend help-check-targets
	@echo "... check"
	@echo "... check-reduce"
	@echo "... check-simd_cat"
	@echo "... check-constexpr"

include Makefile.depend

.PHONY: clean
clean:
	rm -f obj/*.o obj/*.exe obj/*.s
	@rm -f $(subst check,obj,$(checks:=.exe))
	@rm -f $(checks)

