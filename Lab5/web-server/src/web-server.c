// simple_web_server.c
#include "db.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080

char json_content[200];

void read_html(const char *filename, char *buffer, size_t buffer_size) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("Open HTML file fail");
    exit(EXIT_FAILURE);
  }

  size_t n = fread(buffer, 1, buffer_size - 1, file);
  buffer[n] = '\0';
  fclose(file);
}

int main() {
  int server_fd, client_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[3000] = {0};
  char html_content[5000];

  CREATE_DB_STRUCT(my_db);
  my_db.init(&my_db);
  my_db.connect(&my_db);

  read_html("./web-server/static/index.html", html_content,
            sizeof(html_content));

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server is running on http://127.0.0.1:%d\n", PORT);

  char output_buffer[5000];
  char elevator_id[3];
  char inside_count[3];
  char outside_count[3];
  char date[20];
  char time[20];

  while (1) {
    my_db.query(&my_db,
                "SELECT * FROM people_counting ORDER BY time DESC LIMIT 1");
    my_db.store_result(&my_db);
    my_db.output(&my_db, output_buffer);
    sscanf(output_buffer, "%s %s %s %s %s", elevator_id, inside_count,
           outside_count, date, time);
    snprintf(json_content, sizeof(json_content),
             "[{\"elevator_id\":%s,\"inside_count\":%s,\"outside_count\":%s,"
             "\"time\":\"%s-%s\"}]",
             elevator_id, inside_count, outside_count, date, time);
    // printf("%s", json_content);

    client_fd =
        accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client_fd < 0) {
      perror("accept failed");
      continue;
    }

    memset(buffer, 0, sizeof(buffer));
    read(client_fd, buffer, sizeof(buffer));
    printf("Received request:\n%s\n", buffer);

    if (strncmp(buffer, "GET /api/people-counting", 24) == 0) {
      const char *response_header = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: application/json\r\n\r\n";
      write(client_fd, response_header, strlen(response_header));
      write(client_fd, json_content, strlen(json_content));
    } else {
      const char *response_header = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/html\r\n\r\n";
      write(client_fd, response_header, strlen(response_header));
      write(client_fd, html_content, strlen(html_content));
    }

    close(client_fd);
  }

  close(server_fd);
  return 0;
}
