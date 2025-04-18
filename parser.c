#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

// Current identation level in parser
static int identation_level = 0;

// Print idententation. Each identation level is made of 2 spaces.
void print_identation()
{
  int i = 0;
  for (i = 0; i < identation_level; i++)
  {
    printf("  ");
  }
}

// Prints a open or close xml tag
void print_xml_tag(const char *tag, bool open, bool newline)
{
  print_identation();

  if (open)
  {
    printf("<%s>", tag);
  } else
  {
    printf("</%s>", tag);
  }

  if (newline)
  {
    printf("\n");
  }
}

// Prints a open xml tag. To be used only by non terminal symbols
void print_xml_open_tag(const char *tag, bool newline)
{
  print_xml_tag(tag, true, newline);
  identation_level++;
}

// Prints a closed xml tag. To be used only by non terminal symbols
void print_xml_close_tag(const char *tag, bool newline)
{
  identation_level--;
  print_xml_tag(tag, false, newline);
}

// Prints a terminal token to xml.
void print_xml_token(Token *token)
{
  const char *token_label = token_type_str(token->type);
  
  print_identation();

  printf("<%s>%s</%s>\n", token_label, token->token, token_label);
}

// Checks if a token matches a given type and (optionally) a string value.
// If token_str is NULL, then only the token type is checked.
bool check_token_matches(Token *token, TOKEN_TYPE token_type, const char *token_str)
{
  if (token->type != token_type)
  {
    return false;
  }
  else if (token_str == NULL)
  {
    return true;
  }

  return strcmp(token->token, token_str) == 0;
}

// Check if the token is one of the tokens that represent a type
bool check_type(Token *token)
{
 return check_token_matches(token, KEYWORD_TOKEN_TYPE, "int") || check_token_matches(token, KEYWORD_TOKEN_TYPE, "char") || check_token_matches(token, KEYWORD_TOKEN_TYPE, "boolean") || check_token_matches(token, IDENTIFIER_TOKEN_TYPE, NULL);
}

// Check if the token is one of the tokens that represent the beggining of a expression
bool check_expression(Token *token)
{
 return check_token_matches(token, INT_CONST_TOKEN_TYPE, NULL) || check_token_matches(token, STRING_CONST_TOKEN_TYPE, NULL) || check_token_matches(token, KEYWORD_TOKEN_TYPE, "true") || check_token_matches(token, KEYWORD_TOKEN_TYPE, "false") || check_token_matches(token, KEYWORD_TOKEN_TYPE, "null") || check_token_matches(token, IDENTIFIER_TOKEN_TYPE, "this") || check_token_matches(token, IDENTIFIER_TOKEN_TYPE, NULL) || check_token_matches(token, SYMBOL_TOKEN_TYPE, "(") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "-") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "~");
}

// Check if the token is one of the tokens that represent a logical-arithmetical operation
bool check_op(Token *token)
{
 return check_token_matches(token, SYMBOL_TOKEN_TYPE, "+") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "-") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "*") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "/") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "&") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "|") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "<") || check_token_matches(token, SYMBOL_TOKEN_TYPE, ">") || check_token_matches(token, SYMBOL_TOKEN_TYPE, "=");
}

// Validates and consumes token.
// If token is NULL, only type of token is validated
void compile(LexCtx *lex_ctx, TOKEN_TYPE token_type, const char* token)
{
  Token current_token = get_token(lex_ctx);

  if (!check_token_matches(&current_token, token_type, token))
  {
    fprintf(stderr, "Syntax error at line %d, column %d. Expected %s, got: %s\n", current_token.line, current_token.column, token, current_token.token);
    exit(1);
  }

  print_xml_token(&current_token);

  // Advance lexer to next token
  advance(lex_ctx);
}

// Validates and consumes token based only on the token type
void compile_type(LexCtx *lex_ctx, TOKEN_TYPE token_type)
{
  compile(lex_ctx, token_type, NULL);
}

// 
void handle_type(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "int") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "char") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "boolean"))
  {
    compile_type(lex_ctx, KEYWORD_TOKEN_TYPE);
  }
  else if (check_token_matches(&current_token, IDENTIFIER_TOKEN_TYPE, NULL))
  {
    compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);
  }
  else
  {
    fprintf(stderr, "Syntax error at line %d, column %d. Expected \"int\", \"char\", \"boolean\", or an identifier, got: %s", current_token.line, current_token.column, current_token.token);
    exit(1);
  }
}

// Compiles a class
void compileClass(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("class", true);

  compile(lex_ctx, KEYWORD_TOKEN_TYPE, "class");

  compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{");

  // Lookup
  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "field") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "static"))
  {
    compileClassVarDec(lex_ctx);

    current_token = get_token(lex_ctx);
  }

  
  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "constructor") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "function") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "method"))
  {
    compileSubroutine(lex_ctx);

    current_token = get_token(lex_ctx);
  }

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}");

  print_xml_close_tag("class", true);
}

void compileClassVarDec(LexCtx *lex_ctx)
{
  print_xml_open_tag("classVarDec", true);

  // Lookup
  Token current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "field") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "static"))
  {
    compile_type(lex_ctx, KEYWORD_TOKEN_TYPE);
  }
  else
  {
    fprintf(stderr, "Syntax error at line %d, column %d. Expected \"class\" or \"string\", got: %s", current_token.line, current_token.column, current_token.token);
    exit(1);
  }

  handle_type(lex_ctx);

  compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    compile(lex_ctx, SYMBOL_TOKEN_TYPE, ",");

    compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

    current_token = get_token(lex_ctx);
  }

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";");

  print_xml_close_tag("classVarDec", true);
}

void compileSubroutine(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  print_xml_open_tag("subroutineDec", true);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "constructor") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "function") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "method"))
  {
    compile_type(lex_ctx, KEYWORD_TOKEN_TYPE);
  }
  else
  {
    fprintf(stderr, "Syntax error at line %d, column %d. Expected \"constructor\", \"function\" or \"method\", got: %s", current_token.line, current_token.column, current_token.token);
    exit(1);
  }

  current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "void"))
  {
    compile_type(lex_ctx, KEYWORD_TOKEN_TYPE);
  }
  else
  {
    compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);
  }

  compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "(");

  current_token = get_token(lex_ctx);

  compileParameterList(lex_ctx);

  compile(lex_ctx , SYMBOL_TOKEN_TYPE, ")");

  compileSubroutineBody(lex_ctx);

  print_xml_close_tag("subroutineDec", true);
}

void compileParameterList(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  print_xml_open_tag("parameterList", true);

  if (!check_type(&current_token))
  {
    print_xml_close_tag("parameterList", true);
    return;
  }

  handle_type(lex_ctx);

  compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);

    handle_type(lex_ctx);

    compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

    current_token = get_token(lex_ctx);
  }

  print_xml_close_tag("parameterList", true);
}

void compileSubroutineBody(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("subroutineBody", true);

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{");

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "var"))
  {
    compileVarDec(lex_ctx);

    current_token = get_token(lex_ctx);
  }

  compileStatements(lex_ctx);

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}");

  print_xml_close_tag("subroutineBody", true);
}

void compileVarDec(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("varDec", true);

  compile(lex_ctx, KEYWORD_TOKEN_TYPE, "var");

  handle_type(lex_ctx);

  compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
    compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

    current_token = get_token(lex_ctx);
  }

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";");

  print_xml_close_tag("varDec", true);
}

void compileStatements(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("statements", true);

  while (true)
  {
    current_token = get_token(lex_ctx);

    if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "let"))
    {
      compileLet(lex_ctx);
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "if"))
    {
      compileIf(lex_ctx);
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "while"))
    {
      compileWhile(lex_ctx);
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "do"))
    {
      compileDo(lex_ctx);
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "return"))
    {
      compileReturn(lex_ctx);
    }
    else
    {
      break;
    }
  }

  print_xml_close_tag("statements", true);
}

void compileLet(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("letStatement", true);

  compile(lex_ctx, KEYWORD_TOKEN_TYPE, "let");

  compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

  current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "["))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
    compileExpression(lex_ctx);
    compile(lex_ctx, SYMBOL_TOKEN_TYPE, "]");
  }

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "=");

  compileExpression(lex_ctx);

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";");

  print_xml_close_tag("letStatement", true);
}

void compileIf(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("ifStatement", true);

  compile(lex_ctx, KEYWORD_TOKEN_TYPE, "if");

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "(");
  compileExpression(lex_ctx);
  compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")");

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{");
  compileStatements(lex_ctx);
  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}");

  current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "else"))
  {
    compile_type(lex_ctx, KEYWORD_TOKEN_TYPE);

    compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{");
    compileStatements(lex_ctx);
    compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}");
  }

  print_xml_close_tag("ifStatement", true);
}

void compileWhile(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("whileStatement", true);

  compile(lex_ctx, KEYWORD_TOKEN_TYPE, "while");

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "(");
  compileExpression(lex_ctx);
  compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")");

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{");
  compileStatements(lex_ctx);
  compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}");

  print_xml_close_tag("whileStatement", true);
}

void compileDo(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("doStatement", true);

  compile(lex_ctx, KEYWORD_TOKEN_TYPE, "do");
  compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

  current_token = get_token(lex_ctx);

  // This is duplicated in compileTerm, it would require to rewrite the lexer to handle token lookahead
  if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
    compileExpressionList(lex_ctx);
    compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")");
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "."))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
    compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);
    compile(lex_ctx, SYMBOL_TOKEN_TYPE, "(");
    compileExpressionList(lex_ctx);
    compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")");
  }
  else
  {
    fprintf(stderr, "Syntax error at line %d, column %d. Expected \"(\", or \".\", got: %s", current_token.line, current_token.column, current_token.token);
    exit(1);
  }

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";");

  print_xml_close_tag("doStatement", true);
}

void compileReturn(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("returnStatement", true);

  compile(lex_ctx, KEYWORD_TOKEN_TYPE, "return");

  current_token = get_token(lex_ctx);

  if (check_expression(&current_token))
  {
    compileExpression(lex_ctx);
  }

  compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";");

  print_xml_close_tag("returnStatement", true);
}

void compileExpression(LexCtx* lex_ctx)
{
  Token current_token;

  print_xml_open_tag("expression", true);

  compileTerm(lex_ctx);

  current_token = get_token(lex_ctx);

  while (check_op(&current_token))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
    compileTerm(lex_ctx);

    current_token = get_token(lex_ctx);
  }

  print_xml_close_tag("expression", true);
}

void compileTerm(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  print_xml_open_tag("term", true);

  if (check_token_matches(&current_token, INT_CONST_TOKEN_TYPE, NULL))
  {
    compile_type(lex_ctx, INT_CONST_TOKEN_TYPE);
  }
  else if (check_token_matches(&current_token, STRING_CONST_TOKEN_TYPE, NULL))
  {
    compile_type(lex_ctx, STRING_CONST_TOKEN_TYPE);
  }
  else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "true") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "false") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "null") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "this"))
  {
    compile_type(lex_ctx, KEYWORD_TOKEN_TYPE);
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
    compileExpression(lex_ctx);
    compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")");
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "-") || check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "~"))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
    compileTerm(lex_ctx);
  }
  else
  {
    compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);

    current_token = get_token(lex_ctx);
   
    if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "["))
    {
      compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
      compileExpression(lex_ctx);
      compile(lex_ctx, SYMBOL_TOKEN_TYPE, "]");
    }
    // subroutine call - // This is duplicated in compileDo, it would require to rewrite the lexer to handle token lookahead
    else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
    {
      compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
      compileExpressionList(lex_ctx);
      compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")");
    }
    else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "."))
    {
      compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
      compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE);
      compile(lex_ctx, SYMBOL_TOKEN_TYPE, "(");
      compileExpressionList(lex_ctx);
      compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")");
    }
  }

  print_xml_close_tag("term", true);
}

int compileExpressionList(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  print_xml_open_tag("expressionList", true);

  if (!check_expression(&current_token))
  {
    print_xml_close_tag("expressionList", true);
    return 0;
  }

  compileExpression(lex_ctx);

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    compile_type(lex_ctx, SYMBOL_TOKEN_TYPE);
    compileExpression(lex_ctx);

    current_token = get_token(lex_ctx);
  }

  print_xml_close_tag("expressionList", true);

  return 0;
}

int main(int argc, char *argv[])
{
  LexCtx *ctx;

  if (argc != 2)
  {
    fprintf(stderr, "Usage: ./lexer <filename>\n");
    return 1;
  }

  ctx = init_lexer(argv[1]);

  advance(ctx);

  compileClass(ctx);

  return 0;

}