NAME		=	Ben_AFK
FLAGS		= 	-std=c++2a -Wall -Werror -Wextra
COMPILER	=	g++

DIR_INC		=	./inc/
DIR_SRC		=	./src/
DIR_OBJ		= 	./obj/

HEAD_MD		=	client.h \

SRC_MD		=	client.cpp \

INC_PATH 	= 	$(addprefix $(DIR_INC), $(HEAD_MD))

OBJ 		= 	$(addprefix $(DIR_OBJ), $(SRC_MD:.cpp=.o))
INC 		= 	$(addprefix -I, $(DIR_INC))

DIR_LIB		=	../lib_rsa/
LIB_RSA		=	$(addprefix $(DIR_LIB), lib_rsa.a)
LIB_INC		=	$(addprefix $(DIR_LIB), inc/)

.PHONY: all obj $(NAME) clean fclean re $(LIB_RSA)

all: obj $(NAME)
	@cp -f $(NAME) ..

obj:
	@mkdir -p $(DIR_OBJ)

$(NAME): $(LIB_RSA) $(OBJ)
	@$(COMPILER) -o $(NAME) $(OBJ) -lstdc++fs

$(LIB_RSA):
	@make -C $(DIR_LIB)

$(DIR_OBJ)%.o: $(DIR_SRC)%.cpp $(INC_PATH)
	@$(COMPILER) $(FLAGS) $(INC) -c -o $@ $<

clean:
	@make -C $(DIR_LIB) clean
	@rm -rf $(DIR_OBJ)

fclean: clean
	@make -C $(DIR_LIB) fclean
	@rm -f $(NAME)
	@rm -f ../$(NAME)

re: fclean all
