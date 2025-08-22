cmake_minimum_required(VERSION 3.2.0)

AUX_SOURCE_DIRECTORY(. SRC_LIB)

#生成链接库
ADD_LIBRARY(NetFunc ${SRC_LIB})