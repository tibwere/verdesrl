all:
	gcc -g *.c -Wall -Wextra -o verdesrl `mysql_config --cflags --include --libs`
clean:
	-rm -f verdesrl 
