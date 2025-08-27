# コンパイラとフラグ
CXX      := clang++-19
CXXFLAGS := @compile_flags.txt -O2 -MMD -MP

command:
	$(CXX) $(CXXFLAGS) -c ./tests/col/command_static_test.cpp -o ./build/col/command_static_test.o
	$(CXX) $(CXXFLAGS) -c ./tests/col/command_test.cpp -o ./build/col/command_test.o
	$(CXX) $(CXXFLAGS) ./tests/col/command_test.cpp -o ./build/col/command_test.out

clean:
	rm -rf ./build/col/*