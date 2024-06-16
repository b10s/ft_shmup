NAME := ft_shmup

SRCS := main.cpp
OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

CC := cc
CFLAGS := -Wall -Wextra -Werror -MMD -MP
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -MMD -MP
LDFLAGS := -lncurses

RM := rm -f

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

debug: CFLAGS += -fsanitize=address,undefined -g
debug: CXXFLAGS += -fsanitize=address,undefined -g
debug: re

run: all
	./$(NAME)

.PHONY: all clean fclean re debug run

-include $(DEPS)
