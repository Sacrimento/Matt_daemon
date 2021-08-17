all: server client

server:
	@make -C server
	@cp server/matt_daemon .

client:
	@make -C client
	@cp client/Ben_AFK .

clean:
	@make -C server clean
	@make -C client clean

fclean:
	@make -C server fclean
	@make -C client fclean

re:
	@make -C server re
	@make -C client re

.PHONY = all server client clean fclean re
