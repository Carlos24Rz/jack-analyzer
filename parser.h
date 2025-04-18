#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "lexer.h"

/**
 * The following are the available grammar rules for the jack programming language.
 * They consume the required tokens by the indicated rule. Recursive by nature.
 */

// Compiles a class. Should be the first call to the parser
bool compileClass(LexCtx *lex_ctx);

bool compileClassVarDec(LexCtx *lex_ctx);
bool compileSubroutine(LexCtx *lex_ctx);
bool compileParameterList(LexCtx *lex_ctx);
bool compileSubroutineBody(LexCtx *lex_ctx);
bool compileVarDec(LexCtx *lex_ctx);
bool compileStatements(LexCtx *lex_ctx);
bool compileLet(LexCtx *lex_ctx);
bool compileIf(LexCtx *lex_ctx);
bool compileWhile(LexCtx *lex_ctx);
bool compileDo(LexCtx *lex_ctx);
bool compileReturn(LexCtx *lex_ctx);
bool compileExpression(LexCtx *lex_ctx);
bool compileTerm(LexCtx *lex_ctx);
int compileExpressionList(LexCtx *lex_ctx);

#endif