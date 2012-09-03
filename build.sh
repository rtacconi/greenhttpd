#!/bin/bash
cc -c servlet.c
cc -c http.c
gcc http.o servlet.o -o httpd -levent