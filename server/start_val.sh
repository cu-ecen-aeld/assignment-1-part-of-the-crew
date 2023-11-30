#!/bin/sh
# Author: NDunichev

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./aesdsocket > ./valgrind-log.txt
