CXX := c++ -g3 #-fsanitize=address
CXXFLAGS := -std=c++98 -Wall -Wextra #-Werror

NAME := webserv

SRC_DIR := src/
SRC := $(shell find src -name '*.cpp')

INC := inc/

OBJ_DIR := .obj/
OBJ := $(patsubst $(SRC_DIR)%.cpp, $(OBJ_DIR)%.o, $(SRC))

all: $(NAME)

bonus: all

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -I$(INC) $^ -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INC) -MMD -MP  -c $< -o $@ 

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

testParse:
	c++ -Wall -Wextra -std=c++98 -DPARSE_DEBUG -Iinc src/parse/*.cpp src/*.cpp  test/parse/main.cpp && ./a.out
	rm a.out

testVal:
	c++ -Wall -Wextra -std=c++98 -Iinc src/validate/*.cpp src/parse/*.cpp src/*.cpp  test/validate/main.cpp && ./a.out
	rm a.out

.PHONY: all clean fclean re bonus
