# =============================================
# Makefile — 3D Maze Quiz Challenge
# =============================================
# Cara pakai:
#   make        -> build executable 'maze_quiz'
#   make clean  -> hapus semua file hasil build
#   make run    -> build lalu langsung jalankan
# =============================================

ifeq ($(OS),Windows_NT)
CXX      = C:\msys64\mingw64\bin\g++.exe
LIBS     = -L./lib -lglfw3 -lfreeglut -lopengl32 -lglu32 -lgdi32 -lwinmm
RM       = del /Q
RUN      = .\$(TARGET)
TARGET   = maze_quiz.exe
else
CXX      = g++
LIBS     = -lGL -lGLU -lglut -lglfw -lm
RM       = rm -f
RUN      = ./$(TARGET)
TARGET   = maze_quiz
endif
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I./include -DFREEGLUT_STATIC

# Semua file .cpp di direktori src
SRCS = main.cpp globals.cpp maze.cpp quiz.cpp player.cpp renderer.cpp hud.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "Build selesai: ./$(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: all
	$(RUN)

clean:
	-$(RM) $(OBJS) $(TARGET)

.PHONY: all run clean
