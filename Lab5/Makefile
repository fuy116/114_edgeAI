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

run-web:
	sudo ./web-server/bin/web-server

run-db:
	sudo ./db-server/bin/db-server

run-pred:
	cd elevator_ppl_counting && python main.py

run-gui:
	./gui-monitor/bin/monitor

stop:
	sudo pkill -f web-server	

check-port:
	sudo netstat -tulnp

