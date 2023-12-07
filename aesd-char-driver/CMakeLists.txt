cmake_minimum_required(VERSION 3.5)

project(aesd-char-griver LANGUAGES C)

add_library(MyLib STATIC
  aesd-circular-buffer.c
  aesd-circular-buffer.h
)
