IMGUI_DIR = imgui
SRC_DIR = src

SOURCES = $(SRC_DIR)/main_gui.cpp $(SRC_DIR)/Cards.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/imgui_demo.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

OBJS = $(SOURCES:.cpp=.o)

CXX = g++
CXXFLAGS = -std=c++17 -O2 -I$(SRC_DIR) -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += $(shell sdl2-config --cflags)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    GL_LIBS = -framework OpenGL
else
    GL_LIBS = -lGL
endif

LIBS = $(shell sdl2-config --libs) $(GL_LIBS) -ldl -lpthread

TARGET = flashcards_gui

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean