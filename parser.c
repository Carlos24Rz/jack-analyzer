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

  printf("<%s>", token_label);

  // Encode  <, >, " and & to valid xml representation
  if (strcmp(token->token, "<") == 0)
  {
    printf("&lt;");
  }
  else if (strcmp(token->token, ">") == 0)
  {
    printf("&gt;");
  }
  else if (strcmp(token->token, "\"") == 0)
  {
    printf("&quot;");
  }
  else if (strcmp(token->token, "&") == 0)
  {
    printf("&amp;");
  }
  else
  {
    printf("%s", token->token);
  }

  printf("</%s>\n", token_label);
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

void handle_syntax_error(Token *token, const char *expected_msg)
{
  if (token->type != INVALID_TOKEN_TYPE)
  {
    fprintf(stderr, "Syntax error at line %d, column %d. Expected %s, got: %s\n", token->line, token->column, token->token, expected_msg);
  }
}

#define CHECK_COMPILE_RETURN(ret) do { if (!(ret)) { return false; } } while (0)

// Validates and consumes token.
// If token is NULL, only type of token is validated
bool compile(LexCtx *lex_ctx, TOKEN_TYPE token_type, const char* token)
{
  Token current_token = get_token(lex_ctx);

  if (!check_token_matches(&current_token, token_type, token))
  {
    handle_syntax_error(&current_token, token);
    return false;
  }

  print_xml_token(&current_token);

  // Advance lexer to next token
  advance(lex_ctx);

  return true;
}

// Validates and consumes token based only on the token type
bool compile_type(LexCtx *lex_ctx, TOKEN_TYPE token_type)
{
  return compile(lex_ctx, token_type, NULL);
}

// consumes a type 
bool handle_type(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "int") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "char") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "boolean"))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, KEYWORD_TOKEN_TYPE));
  }
  else if (check_token_matches(&current_token, IDENTIFIER_TOKEN_TYPE, NULL))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));
  }
  else
  {
    handle_syntax_error(&current_token, "\"int\", \"char\", \"boolean\", or an identifier");
    return false;
  }

  return true;
}

// Compiles a class
bool compileClass(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("class", true);

  CHECK_COMPILE_RETURN(compile(lex_ctx, KEYWORD_TOKEN_TYPE, "class"));

  CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{"));

  // Lookup
  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "field") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "static"))
  {
    CHECK_COMPILE_RETURN(compileClassVarDec(lex_ctx));

    current_token = get_token(lex_ctx);
  }
  
  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "constructor") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "function") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "method"))
  {
    CHECK_COMPILE_RETURN(compileSubroutine(lex_ctx));

    current_token = get_token(lex_ctx);
  }

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}"));

  print_xml_close_tag("class", true);

  return true;
}

bool compileClassVarDec(LexCtx *lex_ctx)
{
  print_xml_open_tag("classVarDec", true);

  // Lookup
  Token current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "field") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "static"))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, KEYWORD_TOKEN_TYPE));
  }
  else
  {
    handle_syntax_error(&current_token, "\"class\" or \"string\"");
    return false;
  }

  CHECK_COMPILE_RETURN(handle_type(lex_ctx));

  CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ","));

    CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

    current_token = get_token(lex_ctx);
  }

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("classVarDec", true);

  return true;
}

bool compileSubroutine(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  print_xml_open_tag("subroutineDec", true);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "constructor") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "function") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "method"))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, KEYWORD_TOKEN_TYPE));
  }
  else
  {
    handle_syntax_error(&current_token, "\"constructor\", \"function\" or \"method\"");
    return false;
  }

  current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "void"))
  {
   CHECK_COMPILE_RETURN(compile_type(lex_ctx, KEYWORD_TOKEN_TYPE));
  }
  else
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));
  }

  CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "("));

  current_token = get_token(lex_ctx);

  CHECK_COMPILE_RETURN(compileParameterList(lex_ctx));

  CHECK_COMPILE_RETURN(compile(lex_ctx , SYMBOL_TOKEN_TYPE, ")"));

  CHECK_COMPILE_RETURN(compileSubroutineBody(lex_ctx));

  print_xml_close_tag("subroutineDec", true);

  return true;
}

bool compileParameterList(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  print_xml_open_tag("parameterList", true);

  if (!check_type(&current_token))
  {
    print_xml_close_tag("parameterList", true);
    return true;
  }

  CHECK_COMPILE_RETURN(handle_type(lex_ctx));

  CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));

    CHECK_COMPILE_RETURN(handle_type(lex_ctx));

    CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

    current_token = get_token(lex_ctx);
  }

  print_xml_close_tag("parameterList", true);

  return true;
}

bool compileSubroutineBody(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("subroutineBody", true);

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{"));

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "var"))
  {
    CHECK_COMPILE_RETURN(compileVarDec(lex_ctx));

    current_token = get_token(lex_ctx);
  }

  CHECK_COMPILE_RETURN(compileStatements(lex_ctx));

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}"));

  print_xml_close_tag("subroutineBody", true);

  return true;
}

bool compileVarDec(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("varDec", true);

  CHECK_COMPILE_RETURN(compile(lex_ctx, KEYWORD_TOKEN_TYPE, "var"));

  CHECK_COMPILE_RETURN(handle_type(lex_ctx));

  CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

    current_token = get_token(lex_ctx);
  }

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("varDec", true);

  return true;
}

bool compileStatements(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("statements", true);

  while (true)
  {
    current_token = get_token(lex_ctx);

    if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "let"))
    {
      CHECK_COMPILE_RETURN(compileLet(lex_ctx));
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "if"))
    {
      CHECK_COMPILE_RETURN(compileIf(lex_ctx));
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "while"))
    {
      CHECK_COMPILE_RETURN(compileWhile(lex_ctx));
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "do"))
    {
      CHECK_COMPILE_RETURN(compileDo(lex_ctx));
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "return"))
    {
      CHECK_COMPILE_RETURN(compileReturn(lex_ctx));
    }
    else
    {
      break;
    }
  }

  print_xml_close_tag("statements", true);

  return true;
}

bool compileLet(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("letStatement", true);

  CHECK_COMPILE_RETURN(compile(lex_ctx, KEYWORD_TOKEN_TYPE, "let"));

  CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "["))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compileExpression(lex_ctx));
    CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "]"));
  }

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "="));

  CHECK_COMPILE_RETURN(compileExpression(lex_ctx));

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("letStatement", true);

  return true;
}

bool compileIf(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("ifStatement", true);

  CHECK_COMPILE_RETURN(compile(lex_ctx, KEYWORD_TOKEN_TYPE, "if"));

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "("));
  CHECK_COMPILE_RETURN(compileExpression(lex_ctx));
  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")"));

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{"));
  CHECK_COMPILE_RETURN(compileStatements(lex_ctx));
  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}"));

  current_token = get_token(lex_ctx);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "else"))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, KEYWORD_TOKEN_TYPE));

    CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{"));
    CHECK_COMPILE_RETURN(compileStatements(lex_ctx));
    CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}"));
  }

  print_xml_close_tag("ifStatement", true);

  return true;
}

bool compileWhile(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("whileStatement", true);

  CHECK_COMPILE_RETURN(compile(lex_ctx, KEYWORD_TOKEN_TYPE, "while"));

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "("));
  CHECK_COMPILE_RETURN(compileExpression(lex_ctx));
  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")"));

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "{"));
  CHECK_COMPILE_RETURN(compileStatements(lex_ctx));
  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "}"));

  print_xml_close_tag("whileStatement", true);

  return true;
}

bool compileDo(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("doStatement", true);

  CHECK_COMPILE_RETURN(compile(lex_ctx, KEYWORD_TOKEN_TYPE, "do"));
  CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(lex_ctx);

  // This is duplicated in compileTerm, it would require to rewrite the lexer to handle token lookahead
  if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
    // Expression list returns -1 when it fails instead of false
    if (compileExpressionList(lex_ctx) == -1)
    {
      return false;
    }
    CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")"));
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "."))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "("));
    // Expression list returns -1 when it fails instead of false
    if (compileExpressionList(lex_ctx) == -1)
    {
      return false;
    }
    CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")"));
  }
  else
  {
    handle_syntax_error(&current_token, "\"(\", or \".\"");
    return false;
  }

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("doStatement", true);

  return true;
}

bool compileReturn(LexCtx *lex_ctx)
{
  Token current_token;

  print_xml_open_tag("returnStatement", true);

  CHECK_COMPILE_RETURN(compile(lex_ctx, KEYWORD_TOKEN_TYPE, "return"));

  current_token = get_token(lex_ctx);

  if (check_expression(&current_token))
  {
    CHECK_COMPILE_RETURN(compileExpression(lex_ctx));
  }

  CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("returnStatement", true);

  return true;
}

bool compileExpression(LexCtx* lex_ctx)
{
  Token current_token;

  print_xml_open_tag("expression", true);

  CHECK_COMPILE_RETURN(compileTerm(lex_ctx));

  current_token = get_token(lex_ctx);

  while (check_op(&current_token))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compileTerm(lex_ctx));

    current_token = get_token(lex_ctx);
  }

  print_xml_close_tag("expression", true);

  return true;
}

bool compileTerm(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);

  print_xml_open_tag("term", true);

  if (check_token_matches(&current_token, INT_CONST_TOKEN_TYPE, NULL))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, INT_CONST_TOKEN_TYPE));
  }
  else if (check_token_matches(&current_token, STRING_CONST_TOKEN_TYPE, NULL))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, STRING_CONST_TOKEN_TYPE));
  }
  else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "true") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "false") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "null") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "this"))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, KEYWORD_TOKEN_TYPE));
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compileExpression(lex_ctx));
    CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")"));
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "-") || check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "~"))
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compileTerm(lex_ctx));
  }
  else
  {
    CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));

    current_token = get_token(lex_ctx);
   
    if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "["))
    {
      CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
      CHECK_COMPILE_RETURN(compileExpression(lex_ctx));
      CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "]"));
    }
    // subroutine call - // This is duplicated in compileDo, it would require to rewrite the lexer to handle token lookahead
    else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
    {
      CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
      // Expression list returns -1 when it fails instead of false
      if (compileExpressionList(lex_ctx) == -1)
      {
        return false;
      }
      CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")"));
    }
    else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "."))
    {
      CHECK_COMPILE_RETURN(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE));
      CHECK_COMPILE_RETURN(compile_type(lex_ctx, IDENTIFIER_TOKEN_TYPE));
      CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, "("));
      // Expression list returns -1 when it fails instead of false
      if (compileExpressionList(lex_ctx) == -1)
      {
        return false;
      }
      CHECK_COMPILE_RETURN(compile(lex_ctx, SYMBOL_TOKEN_TYPE, ")"));
    }
  }

  print_xml_close_tag("term", true);

  return true;
}

int compileExpressionList(LexCtx *lex_ctx)
{
  Token current_token = get_token(lex_ctx);
  int num_expressions = 0;

  print_xml_open_tag("expressionList", true);

  if (!check_expression(&current_token))
  {
    print_xml_close_tag("expressionList", true);
    return 0;
  }

  if(!compileExpression(lex_ctx))
   return -1;

  current_token = get_token(lex_ctx);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    if(!(compile_type(lex_ctx, SYMBOL_TOKEN_TYPE) && compileExpression(lex_ctx)))
      return -1;
    num_expressions++;

    current_token = get_token(lex_ctx);
  }

  print_xml_close_tag("expressionList", true);

  return num_expressions;
}

int main(int argc, char *argv[])
{
  LexCtx *ctx;
  bool status;

  if (argc != 2)
  {
    fprintf(stderr, "Usage: ./lexer <filename>\n");
    return 1;
  }

  ctx = init_lexer(argv[1]);

  advance(ctx);

  status = compileClass(ctx);

  if (!status)
  {
    fprintf(stderr, "Failed to parse!\n");
    return 1;
  }

  fprintf(stderr, "Done!\n");

  return 0;

}