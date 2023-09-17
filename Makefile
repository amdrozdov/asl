all:
	mkdir -p build && cd build && cmake .. && cmake --build .
run: all
	./build/asl info -f samples/sample.wav
clean:
	rm -f *.wav
lint:
	cpplint `find ./src -name \*.h -or -name \*.cpp`
check: all
	./build/asl --verbose slice -f samples/sample.wav -s 1 2 -e 2 3 -o sl_one.wav sl_two.wav
