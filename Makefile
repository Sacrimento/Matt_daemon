all: srv clt

srv:
	@make -C server

clt:
	@make -C client

clean:
	@make -C server clean
	@make -C client clean

fclean:
	@make -C server fclean
	@make -C client fclean

re:
	@make -C server re
	@make -C client re

.PHONY = all srv clt clean fclean re
