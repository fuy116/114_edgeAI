#include "db.h"
#include "read_env.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int db_init(db_t *self) {
  self->conn = mysql_init(NULL);
  if (self->conn == NULL) {
    fprintf(stderr, "mysql_init() failed(): %s\n", mysql_error(self->conn));
    return EXIT_FAILURE;
  }
  load_env("./.env");
  return EXIT_SUCCESS;
}

int db_connect(db_t *self) {
  if (mysql_real_connect(self->conn, getenv("DB_IP"), getenv("DB_USR"),
                         getenv("DB_PWD"), getenv("DB_NAME"),
                         atoi(getenv("DB_PORT")), NULL, 0) == NULL) {
    fprintf(stderr, "mysql_real_connect() failed: %s\n",
            mysql_error(self->conn));
    mysql_close(self->conn);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int db_query(db_t *self, const char *query) {
  if (mysql_query(self->conn, query)) {
    fprintf(stderr, "%s failed. Error: %s\n", query, mysql_error(self->conn));
    mysql_close(self->conn);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int db_store_result(db_t *self) {
  self->res = mysql_store_result(self->conn);
  if (self->res == NULL) {
    fprintf(stderr, "mysql_store_result() failed. Error: %s\n",
            mysql_error(self->conn));
    mysql_close(self->conn);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void db_output(db_t *self, char *buffer) {
  MYSQL_ROW row;
  int num_fields = mysql_num_fields(self->res);
  buffer[0] = '\0'; // init

  while ((row = mysql_fetch_row(self->res))) {
    for (int i = 0; i < num_fields; i++) {
      const char *field = row[i] ? row[i] : "NULL";
      strcat(buffer, field);
      strcat(buffer, " ");
    }
    strcat(buffer, "\n");
  }
}

void db_close(db_t *self) {
  mysql_free_result(self->res);
  mysql_close(self->conn);
}
