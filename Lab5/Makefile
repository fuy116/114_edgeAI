SUBDIRS := db-server web-server

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

run: run-db run-pred
# run-web:
# 	cd web-server && sudo ./bin/web-server

run-db:
	sudo sh -c "./db-server/bin/db-server &"

run-pred:
	cd elevator_ppl_counting && python main.py

stop:
	sudo pkill -f web-server	

check-port:
	sudo netstat -tulnp

