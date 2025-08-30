# コンパイラとフラグ
CXX      := clang++-19
CXXFLAGS := @compile_flags.txt -O2 -MMD -MP

command:
	$(CXX) $(CXXFLAGS) -c ./tests/col/command_static_test.cpp -o ./build/col/command_static_test.o
	$(CXX) $(CXXFLAGS) -c ./tests/col/command_test.cpp -o ./build/col/command_test.o
	$(CXX) $(CXXFLAGS) ./build/col/command_test.o -o ./build/col/command_test.out

examples:
	$(CXX) $(CXXFLAGS) -c ./examples/col/main.cpp -o ./build/col/main.o
	$(CXX) $(CXXFLAGS) ./build/col/main.o -o ./build/col/main.out

clean:
	rm -rf ./build/col/*

.PHONY: examples clean