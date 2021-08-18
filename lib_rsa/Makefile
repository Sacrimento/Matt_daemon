NAME 		= 	lib_rsa.a
FLAGS		= 	-std=c++2a -Wall -Werror -Wextra
COMPILER	=	g++

DIR_INC		=	./inc/
DIR_SRC		=	./src/
DIR_OBJ		= 	./obj/

HEAD_RSA	=	encryption.h \
				
SRC_RSA		=	encryption.cpp \

INC_PATH 	= 	$(addprefix $(DIR_INC), $(HEAD_RSA))

OBJ 		= 	$(addprefix $(DIR_OBJ), $(SRC_RSA:.cpp=.o))
INC 		= 	$(addprefix -I, $(DIR_INC))

all: obj $(NAME)	

obj:
	@mkdir -p $(DIR_OBJ)

$(NAME): $(OBJ)
	@ar rcs $(NAME) $(OBJ)

$(DIR_OBJ)%.o: $(DIR_SRC)%.cpp $(INC_PATH)
	@$(COMPILER) $(FLAGS) $(INC) -c -o $@ $< 

clean:
	@rm -rf $(DIR_OBJ)

fclean: clean 
	@rm -f $(NAME)

re: fclean all
