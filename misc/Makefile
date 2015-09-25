##
## Makefile for Makefile in /home/aubess_t/work/oxo
## 
## Made by au
## Login   <aubess_t@epitech.net>
## 
## Started on  Fri Jan 24 19:41:22 2014 au
## Last update Sat Mar  8 12:56:17 2014 au
##

SRC_LOADING	= loading.c
SRC_USLEEP	= my_usleep.c

OBJ_LOADING	= $(SRC_LOADING:.c=.o)
OBJ_USLEEP	= $(SRC_USLEEP:.c=.o)

NAME_LOADING	= loading
NAME_USLEEP	= my_usleep

LFLAGS		= -l_my_printf -L lib/

all: $(NAME_LOADING) $(NAME_USLEEP)

$(NAME_LOADING): $(OBJ_LOADING)
	cc -o $(NAME_LOADING) $(SRC_LOADING) $(LFLAGS)

$(NAME_USLEEP): $(OBJ_USLEEP)
	cc -o $(NAME_USLEEP) $(SRC_USLEEP)

clean:
	rm -rf $(OBJ_LOADING)
	rm -rf $(OBJ_USLEEP)

fclean: clean
	rm -rf $(NAME_LOADING)
	rm -rf $(NAME_USLEEP)

re: fclean all

.PHONY: all clean fclean re
