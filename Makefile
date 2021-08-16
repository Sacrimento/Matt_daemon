NAME		=	matt_daemon
FLAGS		= 	-std=c++2a -Wall -Werror -Wextra
COMPILER	=	g++

DIR_INC		=	./inc/
DIR_SRC		=	./src/
DIR_OBJ		= 	./obj/

HEAD_MD	=		tcp_server.h \
				logger.h \
				conf.h \

SRC_MD		=	daemon.cpp \
				tcp_server.cpp \
				logger.cpp \
				conf.cpp \

INC_PATH 	= 	$(addprefix $(DIR_INC), $(HEAD_MD))

OBJ 		= 	$(addprefix $(DIR_OBJ), $(SRC_MD:.cpp=.o))
INC 		= 	$(addprefix -I, $(DIR_INC))

.PHONY: all obj $(NAME) clean fclean re kill

all: obj $(NAME)

obj:
	@mkdir -p $(DIR_OBJ)

$(NAME): $(OBJ)
	@$(COMPILER) -o $(NAME) $(OBJ) -lstdc++fs

$(DIR_OBJ)%.o: $(DIR_SRC)%.cpp $(INC_PATH)
	@$(COMPILER) $(FLAGS) $(INC) -c -o $@ $<

clean:
	@rm -rf $(DIR_OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all

kill:
	sudo kill $(pidof matt_daemon)
