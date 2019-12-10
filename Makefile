CXX = g++
CFLAGS = -Wall -Wextra -std=c++17 -Ofast
OBJ = obj/main.o
LINK = 

obj/%.o: src/%.cpp
	mkdir -p obj
	$(CXX) $(CFLAGS) $< -c -o $@

sts: $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o $@ $(LINK)

clean:
	rm -rf obj sts
