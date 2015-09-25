##
## Makefile for Makefile in /home/aubess_t/work/oxo
## 
## Made by au
## Login   <aubess_t@epitech.net>
## 
## Started on  Fri Nov 24 19:41:22 2014 au
## Last update Sun Mar 23 22:50:32 2014 au
##

SRC		= src/wolf.c

OBJ		= $(SRC:.c=.o)

NAME		= bin/wolf3D

LFLAGS		= -lSDL -lSDLmain -lSDL_image -lSDL_ttf -lSDL_mixer -lm

MISC		= ./misc

LOADING		= @$(MISC)/loading

USLEEP		= @$(MISC)/my_usleep

GREEN		= \033[1;32m
CYAN		= \033[0;36m
CYAN_BRIGHT	= \033[1;36m
WHITE		= \033[0m
RED		= \033[1;31m
GREY		= \033[1;37m
HIDDEN		= \033[8m



## #####################################################################
##
## BIN'S RULES
##

all: $(NAME)

$(NAME): $(OBJ)
	@echo  "\n"
	@echo  "$(RED) >\tCOMPILATION FINISHED\t<\n$(WHITE)"

	@$(USLEEP)

	@echo 	""
	@echo  "$(RED) >\tARCHIVES' CONTENT TABLES BUILDING & BINARIES' LINKAGE\t<\n"

	@$(LOADING)

	@echo  "$(CYAN)"

	@cc -o 	$(NAME) $(SRC) $(LFLAGS)

	@echo  "\n"
	@echo  "$(GREEN)\t >\tLIBS & BINARIES' UPDATE : FINISHED\t<\n\n"
	@echo  "$(GREY) > Projet développé par :\n"

	@cat 	$(MISC)/login

	@echo  "$(GREY) Copyrigth ©\n"
	@echo  "$(WHITE)"

clean:
	$(USLEEP)
	@echo "$(RED)\n >\t.o & TEMPORARY FILES DELETED\n$(WHITE)"
	@rm -rf $(OBJ)

fclean: clean
	@echo ""
	@rm -rf $(NAME)
	@$(USLEEP)
	@echo "$(RED) >\tBINARIES DELETED\n$(WHITE)"
	@$(USLEEP)
	@echo "$(CYAN)"

clear:
	clear

re:	clear fclean all



## #####################################################################
##
## MISC' RULES
##

$(MISC):
	@echo "$(GREY)\n\n\n\t\t\t\t > MAKE TOOLS <\n$(WHITE)"
	@$(USLEEP)
	@(make -C $(MISC) re)



## #####################################################################
##
## BONUS RULES
##

coffee:
	@echo "$(CYAN_BRIGHT)"
	@$(MISC)/$@/$@.sh
	@echo "$(WHITE)"

boobs:
	@clear
	@echo "$(CYAN_BRIGHT)"
	@cat $(MISC)/$@
	@echo "$(WHITE)"

## #####################################################################
##
## .PHONY
##

.PHONY: all clean fclean re misc coffee boobs
