#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdio.h>
#include "lexer.h"

typedef struct Parser Parser;

/**
 * The following are the available grammar rules for the jack programming language.
 * They consume the required tokens by the indicated rule. Recursive by nature.
 */

// Compiles a class. Should be the first call to the parser
bool compileClass(Parser *parser, FILE *out_file);

bool compileClassVarDec(Parser *parser, FILE *out_file);
bool compileSubroutine(Parser *parser, FILE *out_file);
bool compileParameterList(Parser *parser, FILE *out_file);
bool compileSubroutineBody(Parser *parser, FILE *out_file);
bool compileVarDec(Parser *parser, FILE *out_file);
bool compileStatements(Parser *parser, FILE *out_file);
bool compileLet(Parser *parser, FILE *out_file);
bool compileIf(Parser *parser, FILE *out_file);
bool compileWhile(Parser *parser, FILE *out_file);
bool compileDo(Parser *parser, FILE *out_file);
bool compileReturn(Parser *parser, FILE *out_file);
bool compileExpression(Parser *parser, FILE *out_file);
bool compileTerm(Parser *parser, FILE *out_file);
int compileExpressionList(Parser *parse, FILE *out_file);

#endif