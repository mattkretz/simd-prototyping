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

fortests := for t in $(tests); do
fortestarchs := for a in $(testarchs); do
fortesttypes := for type in $(testtypes); do
fortestwidths := for w in $(testwidths); do

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

helptargets:=

# argument: arch
define pch_template
obj/$(1).h: tests/unittest.h tests/*.cpp
	@echo "Generate $$@"
	@grep -h '^ *# *include ' $$^|grep -v unittest.h|sort -u > $$@

obj/$(1).depend: obj/$(1).h
	@echo "Update $(1) dependencies"
	@$$(CXX) $$(CXXFLAGS) -march=$(1) -MM -MT "obj/$(1).h.gch" $$< > $$@

include obj/$(1).depend

obj/$(1).h.gch: obj/$(1).h
	@echo "Build pre-compiled header for $(1)"
	@$$(CXX) $$(CXXFLAGS) -march=$(1) -c $$< -o $$@

endef

$(foreach arch,$(testarchs),\
	$(eval $(call pch_template,$(arch))))

# arguments: test, arch
define exe_template
obj/$(1).$(2)/%.exe: tests/$(1).cpp obj/$(2).h.gch tests/unittest.h
	@echo "Build $$(@:obj/%.exe=check/%)"
	@mkdir -p $$(dir $$@)
	@$$(CXX) $$(CXXFLAGS) -march=$(2) -D UNITTEST_TYPE="$$(call gettype,$$*)" -D UNITTEST_WIDTH=$$(call getwidth,$$*) -include obj/$(2).h -c -o $$(@:.exe=.o) $$<
	@echo " Link $$(@:obj/%.exe=check/%)"
	@$$(CXX) $$(CXXFLAGS) -march=$(2) -o $$@ $$(@:.exe=.o)
	@rm $$(@:.exe=.o)

endef

$(foreach arch,$(testarchs),\
	$(foreach t,$(tests),\
	$(eval $(call exe_template,$(t),$(arch)))))

define simple_check_template
check-$(1): $(2)

helptargets+=check-$(1)

endef

define check_template
check-$(1):
	@$$(MAKE) --no-print-directory $$(shell $(2) | shuf)

helptargets+=check-$(1)

endef

$(foreach t,$(tests),\
	$(eval $(call check_template,$(t),$(fortestarchs) $(fortestwidths) $(fortesttypes) \
	  echo "check/$(t).$$$$a/$$$$type.$$$$w";done;done;done)) \
	)

$(foreach type,$(testtypes),\
	$(eval $(call check_template,$(type),$(fortests) $(fortestarchs) $(fortestwidths) \
	  echo "check/$$$$t.$$$$a/$(type).$$$$w";done;done;done)) \
	)

$(foreach arch,$(testarchs),\
	$(eval $(call simple_check_template,constexpr-$(arch),obj/constexpr.$(arch).s))\
	$(eval $(call check_template,$(arch),$(fortesttypes) $(fortests) \
	  echo "check-$$$$t.$(arch).$$$$type";done;done;\
	  echo "check-constexpr-$(arch)")) \
	)

$(foreach type,$(testtypes),\
	$(foreach w,$(testwidths),\
	  $(eval $(call check_template,$(type).$(w),$(fortests) $(fortestarchs) \
	    echo "check/$$$$t.$$$$a/$(type).$(w)";done;done))) \
	)

$(foreach t,$(tests),\
	$(foreach type,$(testtypes),\
	  $(eval $(call check_template,$(t).$(type),$(fortestarchs) $(fortestwidths) \
	    echo "check/$(t).$$$$a/$(type).$$$$w";done;done))) \
	$(foreach type,$(testtypes),\
	  $(foreach w,$(testwidths),\
	    $(eval $(call check_template,$(t).$(type).$(w),$(fortestarchs) \
	      echo "check/$(t).$$$$a/$(type).$(w)";done)))) \
	$(foreach arch,$(testarchs),\
	  $(foreach type,$(testtypes),\
	    $(eval $(call check_template,$(t).$(arch).$(type),$(fortestwidths) \
	      echo "check/$(t).$(arch)/$(type).$$$$w";done)))) \
	)

constexpr_checks ::= $(foreach arch,$(testarchs),obj/constexpr.$(arch).s)

.PHONY: check-constexpr
check-constexpr: $(shell shuf -e -- $(constexpr_checks))

check_targets := obj/check.targets

$(check_targets): $(wildcard tests/*.cpp) Makefile
	$(file >$@)
	$(foreach t,$(tests),$(foreach w,$(testwidths),$(foreach y,$(testtypes),$(foreach a,$(testarchs),\
		$(file >>$@,check/$t.$a/$y.$w)))))

obj/Makefile.check: $(check_targets) Makefile
	$(file >$@,.NOTINTERMEDIATE: $$(subst check,obj,$$(patsubst %,%.exe,$$(shell cat $(check_targets)))))
	$(file >>$@,.PHONY: check)
	$(file >>$@,check: $$(shell shuf $(check_targets)) check-constexpr check-simd_cat check-reduce)

include obj/Makefile.check

obj/codegen.depend: codegen/*.c++
	@echo "Update codegen dependencies"
	@mkdir -p obj
	@$(CXX) $(CXXFLAGS) -MM -MT obj/codegen.simd_cat.s codegen/simd_cat.c++ > $@
	@$(CXX) $(CXXFLAGS) -MM -MT obj/codegen.reduce.s codegen/reduce.c++ >> $@

include obj/codegen.depend

$(shell $(CXX) $(CXXFLAGS) -MM -MT "obj/constexpr.%.s" constexpr_tests.c++|tr -d '\\')
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

check/%: obj/%.exe
	@mkdir -p $(dir $@)
	@{ \
		echo "================================================================";\
		echo "  Run check/$*"; \
		echo "================================================================";\
		obj/$*.exe; \
	} | tee $@
	@tail -n1 $@ | grep -q '^Failed tests: 0$$'

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

helptxt := obj/help.txt

$(helptxt): Makefile $(check_targets)
	$(file >$@)
	$(foreach t,$(helptargets),$(file >>$@,... $(t)))
	@sed 's/^/... /' $(check_targets) >>$@

.PHONY: help
help: $(helptxt)
	@echo "... check"
	@echo "... check-reduce"
	@echo "... check-simd_cat"
	@echo "... check-constexpr"
	@cat $(helptxt)

.PHONY: clean
clean:
	rm -f obj/*.o obj/*.exe obj/*.s obj/*.depend $(check_targets) obj/Makefile.* $(helptxt)
	rm -rf obj/*.*/
	rm -rf check/*.*/

