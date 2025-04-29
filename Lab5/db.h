// db.h
#pragma once

#include <stdio.h>
#include <mysql/mysql.h>
#include <stdlib.h>

typedef struct db db_t;

typedef int (*db_init_func_t)(db_t *self);
typedef int (*db_connect_func_t)(db_t *self);
typedef int (*db_query_func_t)(db_t *self, const char *query);
typedef int (*db_store_result_func_t)(db_t *self);
typedef void (*db_close_func_t)(db_t *self);
typedef void (*db_output_func_t)(db_t *self, char *buffer);

int db_init(db_t *self);
int db_connect(db_t *self);
int db_query(db_t *self, const char *query);
int db_store_result(db_t *self);
void db_close(db_t *self);
void db_output(db_t *self, char *buffer);

#define INIT_DB_STRUCT() { \
    .conn = NULL, \
    .res = NULL, \
    .init = db_init, \
    .connect = db_connect, \
    .query = db_query, \
    .store_result = db_store_result, \
    .close = db_close, \
    .output = db_output, \
}

#define CREATE_DB_STRUCT(name) \
	struct db name = INIT_DB_STRUCT()

typedef struct db
{
    MYSQL* conn;
    MYSQL_RES *res;
    db_init_func_t init;
    db_connect_func_t connect;
    db_query_func_t query;
    db_store_result_func_t store_result;
    db_close_func_t close;
    db_output_func_t output;
}db_t;

