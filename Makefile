GXX := g++
SRC_DIR := ./src
OBJ_DIR := ./obj
INC_DIR := -I./include
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
CXXFLAGS := -Wall -g

main.exe: $(OBJ_FILES)
	$(GXX) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(GXX) $(CPPFLAGS) $(CXXFLAGS) $(INC_DIR) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $@
	
clean:
	rm -f  $(OBJ_DIR)/%.o, main.exe
