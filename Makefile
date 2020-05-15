client:
	gcc -g *.c -Wall -Wextra -o verdesrl `mysql_config --cflags --include --libs`

#	regole di compilazione utilizzate in fase di sviluppo
#	eseguire con: > valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./<prog> <options>
#	
valgrind-test:
	gcc -g *.c -Wall -Wextra -ggdb3 -o debug `mysql_config --cflags --include --libs`

clean:
	-rm -f verdesrl debug 
