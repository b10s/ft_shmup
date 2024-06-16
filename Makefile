NAME := ft_shmup

HDRS := ft_shmup.h
SRCS := main.c
OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

CC := cc
CFLAGS := -Wall -Wextra -Werror -MMD -MP
LDFLAGS := -lncurses

RM := rm -f

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

debug: CFLAGS += -fsanitize=address,undefined -g
debug: re

run: all
	./$(NAME)

.PHONY: all clean fclean re debug run 

-include $(DEPS)
