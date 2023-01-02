C_FLAGS=-g

objs:
	mkdir $@

objs/html.o: src/html.c objs
	$(CC) $(C_FLAGS) -c $< -o $@

objs/main.o: src/main.c objs
	$(CC) $(C_FLAGS) -c $< -o $@

weatherd: objs/main.o objs/html.o
	$(CC) $(C_FLAGS) $^ -o $@

.PHONY: install
install:
	mkdir -p /var/weatherman
