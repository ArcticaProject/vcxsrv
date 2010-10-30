@echo off
"bison++" -d -Ssrc/bison.cc -Hsrc/bison.h -h%1/mhmakeparser.h -o%1/mhmakeparser.cpp src\mhmakeParser.y
python addstdafxh.py %1\mhmakeparser.cpp

