#! /bin/sh
flex --nounistd -oglsl-optimizer/src/glsl/glcpp/glcpp-lex.c glsl-optimizer/src/glsl/glcpp/glcpp-lex.l
flex --nounistd -oglsl-optimizer/src/glsl/glsl_lexer.cpp glsl-optimizer/src/glsl/glsl_lexer.ll
bison -v -o "glsl-optimizer/src/glsl/glcpp/glcpp-parse.c" -p "glcpp_parser_" --defines=glsl-optimizer/src/glsl/glcpp/glcpp-parse.h glsl-optimizer/src/glsl/glcpp/glcpp-parse.y
bison -v -o "glsl-optimizer/src/glsl/glsl_parser.cpp" -p "_mesa_glsl_" --defines=glsl-optimizer/src/glsl/glsl_parser.h glsl-optimizer/src/glsl/glsl_parser.yy
