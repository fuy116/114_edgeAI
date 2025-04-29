all:
	gcc -O3 -lmysqlclient db-server.c db.c -o db-server && \
	gcc -O3 -lmysqlclient web-server.c db.c -o web-server

run-server:
	sudo ./web-server &
	sudo ./db-server &

check-port:
	sudo netstat -tulnp
