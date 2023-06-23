all: check

check: check-default check-skylake check-avx512

check-default:
	$(CXX) -std=gnu++23 -c -Wall -Wextra -O2 -g0 tests.h

check-skylake:
	$(CXX) -std=gnu++23 -march=skylake -c -Wall -Wextra -O2 -g0 tests.h

check-avx512:
	$(CXX) -std=gnu++23 -march=skylake-avx512 -c -Wall -Wextra -O2 -g0 tests.h

help:
	@echo "... check"
	@echo "... check-default"
	@echo "... check-skylake"
	@echo "... check-avx512"
