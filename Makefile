LIBS_DIR   = lib
INC_DIR    = include
CXX        = g++
CXXFLAGS   = -std=c++20
LDFLAGS    = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

default:
	$(CXX) main.cpp -o game $(CXXFLAGS) $(LDFLAGS) -L$(LIBS_DIR) -I$(INC_DIR)

game.exe:
	x86_64-w64-mingw32-g++ main.cpp -o game.exe $(CXXFLAGS) \
		-lraylib -lopengl32 -lgdi32 -lwinmm -static \
		-L$(LIBS_DIR) -I$(INC_DIR)