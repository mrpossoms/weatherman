C_FLAGS=-g

INC=-Igitman_sources/onion/src
LINK=gitman_sources/onion/build/src/onion/libonion_static.a

objs:
	mkdir $@

objs/html.o: src/html.c objs
	$(CC) $(INC) $(C_FLAGS) -c $< -o $@

objs/main.o: src/main.c objs
	$(CC) $(INC) $(C_FLAGS) -c $< -o $@

weatherd: objs/main.o objs/html.o
	$(CC) $(INC) $(C_FLAGS) $^ -o $@ $(LINK)

.PHONY: install
install:
	mkdir -p /var/weatherman
