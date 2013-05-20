# -*- makefile -*-
#

PREFIX = /usr/local
VERSION = 0.5.5.1
CFLAGS=-O2 -Wall -g
INCLUDES=-I. -I../include 
TEST_PROGS= merge_dvb conv satscan cam_set cam_test quickscan cam_menu dump_TS
#CXX = g++-3.2
#CC  = gcc-3.2
