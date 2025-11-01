# コンパイラとフラグ
CXX      := clang++-22
CXXFLAGS := @compile_flags.txt -O2 -MMD -MP

static_test:
	$(CXX) $(CXXFLAGS) -c ./tests/col/command/command_static_test.cpp -o ./build/col/command/command_static_test.o
	$(CXX) $(CXXFLAGS) -c ./tests/col/command/concepts_static_test.cpp -o ./build/col/command/concepts_static_test.o
	$(CXX) $(CXXFLAGS) -c ./tests/col/control_flow_static_test.cpp -o ./build/col/control_flow_static_test.o
	$(CXX) $(CXXFLAGS) -c ./tests/col/optional_static_test.cpp -o ./build/col/optional_static_test.o

example:
	$(CXX) $(CXXFLAGS) -c ./examples/col/command/main.cpp -o ./build/col/command/main.o
	$(CXX) $(CXXFLAGS) ./build/col/command/main.o -o ./build/col/command.out

clean:
	rm -rf ./build/col/*

.PHONY: examples clean