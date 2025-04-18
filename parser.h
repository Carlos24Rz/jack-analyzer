#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

/**
 * The following are the available grammar rules for the jack programming language.
 * They consume the required tokens by the indicated rule. Recursive by nature.
 */

// Compiles a class. Should be the first call to the parser
void compileClass(LexCtx *lex_ctx);

void compileClassVarDec(LexCtx *lex_ctx);
void compileSubroutine(LexCtx *lex_ctx);
void compileParameterList(LexCtx *lex_ctx);
void compileSubroutineBody(LexCtx *lex_ctx);
void compileVarDec(LexCtx *lex_ctx);
void compileStatements(LexCtx *lex_ctx);
void compileLet(LexCtx *lex_ctx);
void compileIf(LexCtx *lex_ctx);
void compileWhile(LexCtx *lex_ctx);
void compileDo(LexCtx *lex_ctx);
void compileReturn(LexCtx *lex_ctx);
void compileExpression(LexCtx *lex_ctx);
void compileTerm(LexCtx *lex_ctx);
int compileExpressionList(LexCtx *lex_ctx);

#endif