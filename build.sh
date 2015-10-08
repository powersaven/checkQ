#!/bin/sh -xe

TOP=$(cd $(dirname $0); pwd -P)

CCFLAG="-I${TOP} -I${TOP}/src -I${TOP}/src/mylib -g -D_GNU_SOURCE -D_POSIX_SOURCE -DHAVE_DEBUG -DREADABLE_DATA_LOG -std=c99 -Werror -Wall -c -o"

BUILD_FLAG="-fPIC -lm -lpthread -lxml2 -o"

CC="gcc ${CCFLAG}"

BUILD="gcc ${BUILD_FLAG}"

ARC="ar cru"

LIB="ranlib"

cd $TOP

lib/libmysql/link.sh

cd src

cd mylib

$CC frandom.o frandom.c
$CC log.o log.c
$CC hash.o hash.c
$CC my_mutex.o my_mutex.c
$CC mylib.o mylib.c
$CC sig_hdl.o sig_hdl.c
$CC vector.o vector.c
$CC sigsegv.o sigsegv.c

$ARC libcq.a frandom.o log.o hash.o my_mutex.o mylib.o sig_hdl.o vector.o sigsegv.o
$LIB libcq.a

cd t

$CC log_test.o log_test.c
$CC hash_test.o hash_test.c
$CC vector_test.o vector_test.c
$BUILD log_test log_test.o $TOP/src/mylib/libcq.a
$BUILD hash_test hash_test.o $TOP/src/mylib/libcq.a
$BUILD vector_test vector_test.o $TOP/src/mylib/libcq.a

cd ..
cd ..

cd server

$CC cq_server.o cq_server.c

# Build executables in server
cd t

$CC test_server.o test_server.c
$CC test_server_conn.o test_server_conn.c
$BUILD test_server test_server.o $TOP/src/server/cq_server.o $TOP/src/mylib/libcq.a $TOP/lib/libmysql/lib/libmysqlclient.a
$BUILD test_server_conn test_server_conn.o $TOP/src/server/cq_server.o $TOP/src/mylib/libcq.a $TOP/lib/libmysql/lib/libmysqlclient.a 

cd ..
cd ..

cd command
$CC cq_command.o cq_command.c

cd t

$CC test_command.o test_command.c
$BUILD test_command test_command.o $TOP/src/command/cq_command.o $TOP/src/mylib/libcq.a

cd ..
cd ..

cd table

$CC cq_table.o cq_table.c

cd t

$CC test_table.o test_table.c
$BUILD test_table test_table.o $TOP/src/table/cq_table.o $TOP/src/mylib/libcq.a

cd ..
cd ..

cd parser
$CC file_operation.o file_operation.c
$CC verify.o verify.c
$CC parser.o parser.c

$ARC libparser.a parser.o verify.o file_operation.o 
$LIB libparser.a 

cd t

$CC test_parser.o test_parser.c 
$BUILD test_parser test_parser.o $TOP/src/server/cq_server.o $TOP/src/command/cq_command.o $TOP/src/table/cq_table.o $TOP/src/parser/libparser.a $TOP/src/mylib/libcq.a $TOP/lib/libmysql/lib/libmysqlclient.a

cd ..
cd ..

cd cq_mysql
$CC cq_mysql_cmd.o cq_mysql_cmd.c
$CC cq_sql_gen.o cq_sql_gen.c

cd t

$CC test_mysql_cmd.o test_mysql_cmd.c 
#$BUILD test_mysql_cmd test_mysql_cmd.o $TOP/src/cq_mysql/cq_mysql_cmd.o $TOP/src/cq_mysql/cq_sql_gen.o $TOP/src/table/cq_table.o $TOP/src/server/cq_server.o $TOP/src/mylib/libcq.a $TOP/lib/libmysql/lib/libmysqlclient.a

cd ..
cd ..

cd engine
$CC cq_engine.o cq_engine.c
$CC cq_stats.o cq_stats.c

cd t
$CC test_engine.o test_engine.c
$BUILD test_engine test_engine.o $TOP/src/engine/cq_engine.o $TOP/src/engine/cq_stats.o $TOP/src/server/cq_server.o $TOP/src/command/cq_command.o $TOP/src/table/cq_table.o $TOP/src/cq_mysql/cq_mysql_cmd.o $TOP/src/cq_mysql/cq_sql_gen.o $TOP/src/mylib/libcq.a $TOP/lib/libmysql/lib/libmysqlclient.a -lpthread

cd ..
$CC checkQ.o checkQ.c

$BUILD $TOP/bin/checkQ checkQ.o $TOP/src/engine/cq_engine.o $TOP/src/engine/cq_stats.o $TOP/src/server/cq_server.o $TOP/src/command/cq_command.o $TOP/src/table/cq_table.o $TOP/src/cq_mysql/cq_mysql_cmd.o $TOP/src/cq_mysql/cq_sql_gen.o $TOP/src/parser/libparser.a $TOP/src/mylib/libcq.a $TOP/lib/libmysql/lib/libmysqlclient.a -lpthread


cd ..
cd ..
