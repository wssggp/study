#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

// gcc -D_FILE_OFFSET_BITS=64 `mysql_config --cflags` -o mysql_test mysql_test.c `mysql_config --libs`

int main() {

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char server[] = "127.0.0.1";//192.168.2.107
    char user[] = "guopeng";
    char password[] = "Wssggp@52013";
    char database[] = "lingsheng-test";

    conn = mysql_init(NULL);
    if (mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        printf("%s\n", mysql_error(conn));
    } 

    char sql[128] = {0};
    sprintf(sql, "insert into user_info(`name`,title,money)values('king', '', 3);");
    if (mysql_query(conn, sql)) {
        printf("%s\n", mysql_error(conn));
    }


    if (mysql_query(conn, "select * from user_info")) {
        printf("%s\n", mysql_error(conn));
    }

    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {
        printf("%s\t", row[0]);
        printf("%s\t", row[1]);
        printf("%s\t", row[2]);
        printf("%s\n", row[3]);
    }

    mysql_free_result(res);
    mysql_close(conn);

    return 0;

}
