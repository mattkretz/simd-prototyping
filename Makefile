all: check

check:
	$(CXX) -std=gnu++23 -c -Wall -Wextra -O2 -g0 tests.h

help:
	@echo "... check"
