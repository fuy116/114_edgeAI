SUBDIRS := db-server web-server

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

run-web:
	cd web-server && sudo ./bin/web-server

run-db:
	sudo ./db-server/bin/db-server

stop:
	sudo pkill -f web-server	

check-port:
	sudo netstat -tulnp

