#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

struct Parser
{
  LexCtx *lexer;
  int identation_level;
};

// Print idententation. Each identation level is made of 2 spaces.
void print_identation(int identation_level, FILE *out)
{
  int i = 0;
  for (i = 0; i < identation_level; i++)
  {
    fprintf(out, "  ");
  }
}

// Prints a open or close xml tag
void print_xml_tag(const char *tag, bool open, bool newline, int identation_level, FILE *out)
{
  print_identation(identation_level, out);

  if (open)
  {
    fprintf(out, "<%s>", tag);
  } else
  {
    fprintf(out,"</%s>", tag);
  }

  if (newline)
  {
    fprintf(out,"\n");
  }
}

// Prints a open xml tag. To be used only by non terminal symbols
void print_xml_open_tag(const char *tag, bool newline, int *identation_level, FILE *out)
{
  print_xml_tag(tag, true, newline, *identation_level, out);
  *identation_level += 1;
}

// Prints a closed xml tag. To be used only by non terminal symbols
void print_xml_close_tag(const char *tag, bool newline, int *identation_level, FILE *out)
{
  *identation_level -= 1;
  print_xml_tag(tag, false, newline, *identation_level, out);
}

// Prints a terminal token to xml.
void print_xml_token(Token *token, int *identation_level, FILE *out)
{
  const char *token_label = token_type_str(token->type);
  
  print_identation(*identation_level, out);

  fprintf(out, "<%s>", token_label);

  // Encode  <, >, " and & to valid xml representation
  if (strcmp(token->token, "<") == 0)
  {
    fprintf(out, "&lt;");
  }
  else if (strcmp(token->token, ">") == 0)
  {
    fprintf(out, "&gt;");
  }
  else if (strcmp(token->token, "\"") == 0)
  {
    fprintf(out, "&quot;");
  }
  else if (strcmp(token->token, "&") == 0)
  {
    fprintf(out, "&amp;");
  }
  else
  {
    fprintf(out, "%s", token->token);
  }

  fprintf(out, "</%s>\n", token_label);
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
    fprintf(stderr, "Syntax error at line %d, column %d. Expected %s, got: %s\n", token->line, token->column, expected_msg, token->token);
  }
}

#define CHECK_COMPILE_RETURN(ret) do { if (!(ret)) { return false; } } while (0)

// Validates and consumes token.
// If token is NULL, only type of token is validated
bool compile(Parser *parser, FILE *out, TOKEN_TYPE token_type, const char* token)
{
  Token current_token = get_token(parser->lexer);

  if (!check_token_matches(&current_token, token_type, token))
  {
    handle_syntax_error(&current_token, token);
    return false;
  }

  print_xml_token(&current_token, &parser->identation_level, out);

  // Advance lexer to next token
  advance(parser->lexer);

  return true;
}

// Validates and consumes token based only on the token type
bool compile_type(Parser *parser, FILE *out, TOKEN_TYPE token_type)
{
  return compile(parser, out, token_type, NULL);
}

// consumes a type 
bool handle_type(Parser* parser, FILE *out)
{
  Token current_token = get_token(parser->lexer);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "int") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "char") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "boolean"))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, KEYWORD_TOKEN_TYPE));
  }
  else if (check_token_matches(&current_token, IDENTIFIER_TOKEN_TYPE, NULL))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));
  }
  else
  {
    handle_syntax_error(&current_token, "\"int\", \"char\", \"boolean\", or an identifier");
    return false;
  }

  return true;
}

// Compiles a class
bool compileClass(Parser* parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("class", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compile(parser, out, KEYWORD_TOKEN_TYPE, "class"));

  CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "{"));

  // Lookup
  current_token = get_token(parser->lexer);

  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "field") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "static"))
  {
    CHECK_COMPILE_RETURN(compileClassVarDec(parser, out));

    current_token = get_token(parser->lexer);
  }
  
  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "constructor") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "function") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "method"))
  {
    CHECK_COMPILE_RETURN(compileSubroutine(parser, out));

    current_token = get_token(parser->lexer);
  }

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "}"));

  print_xml_close_tag("class", true, &parser->identation_level, out);

  return true;
}

bool compileClassVarDec(Parser *parser, FILE *out)
{
  print_xml_open_tag("classVarDec", true, &parser->identation_level, out);

  // Lookup
  Token current_token = get_token(parser->lexer);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "field") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "static"))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, KEYWORD_TOKEN_TYPE));
  }
  else
  {
    handle_syntax_error(&current_token, "\"class\" or \"string\"");
    return false;
  }

  CHECK_COMPILE_RETURN(handle_type(parser, out));

  CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(parser->lexer);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ","));

    CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

    current_token = get_token(parser->lexer);
  }

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("classVarDec", true, &parser->identation_level, out);

  return true;
}

bool compileSubroutine(Parser *parser, FILE *out)
{
  Token current_token = get_token(parser->lexer);

  print_xml_open_tag("subroutineDec", true, &parser->identation_level, out);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "constructor") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "function") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "method"))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, KEYWORD_TOKEN_TYPE));
  }
  else
  {
    handle_syntax_error(&current_token, "\"constructor\", \"function\" or \"method\"");
    return false;
  }

  current_token = get_token(parser->lexer);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "void"))
  {
   CHECK_COMPILE_RETURN(compile_type(parser, out, KEYWORD_TOKEN_TYPE));
  }
  else
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));
  }

  CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "("));

  current_token = get_token(parser->lexer);

  CHECK_COMPILE_RETURN(compileParameterList(parser, out));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ")"));

  CHECK_COMPILE_RETURN(compileSubroutineBody(parser, out));

  print_xml_close_tag("subroutineDec", true, &parser->identation_level, out);

  return true;
}

bool compileParameterList(Parser *parser, FILE *out)
{
  Token current_token = get_token(parser->lexer);

  print_xml_open_tag("parameterList", true, &parser->identation_level, out);

  if (!check_type(&current_token))
  {
    print_xml_close_tag("parameterList", true, &parser->identation_level, out);
    return true;
  }

  CHECK_COMPILE_RETURN(handle_type(parser, out));

  CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(parser->lexer);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));

    CHECK_COMPILE_RETURN(handle_type(parser, out));

    CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

    current_token = get_token(parser->lexer);
  }

  print_xml_close_tag("parameterList", true, &parser->identation_level, out);

  return true;
}

bool compileSubroutineBody(Parser *parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("subroutineBody", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "{"));

  current_token = get_token(parser->lexer);

  while (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "var"))
  {
    CHECK_COMPILE_RETURN(compileVarDec(parser, out));

    current_token = get_token(parser->lexer);
  }

  CHECK_COMPILE_RETURN(compileStatements(parser, out));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "}"));

  print_xml_close_tag("subroutineBody", true, &parser->identation_level, out);

  return true;
}

bool compileVarDec(Parser *parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("varDec", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compile(parser, out, KEYWORD_TOKEN_TYPE, "var"));

  CHECK_COMPILE_RETURN(handle_type(parser, out));

  CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(parser->lexer);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

    current_token = get_token(parser->lexer);
  }

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("varDec", true, &parser->identation_level, out);

  return true;
}

bool compileStatements(Parser *parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("statements", true, &parser->identation_level, out);

  while (true)
  {
    current_token = get_token(parser->lexer);

    if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "let"))
    {
      CHECK_COMPILE_RETURN(compileLet(parser, out));
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "if"))
    {
      CHECK_COMPILE_RETURN(compileIf(parser, out));
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "while"))
    {
      CHECK_COMPILE_RETURN(compileWhile(parser, out));
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "do"))
    {
      CHECK_COMPILE_RETURN(compileDo(parser, out));
    }
    else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "return"))
    {
      CHECK_COMPILE_RETURN(compileReturn(parser, out));
    }
    else
    {
      break;
    }
  }

  print_xml_close_tag("statements", true, &parser->identation_level, out);

  return true;
}

bool compileLet(Parser *parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("letStatement", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compile(parser, out, KEYWORD_TOKEN_TYPE, "let"));

  CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(parser->lexer);

  if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "["))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compileExpression(parser, out));
    CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "]"));
  }

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "="));

  CHECK_COMPILE_RETURN(compileExpression(parser, out));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("letStatement", true, &parser->identation_level, out);

  return true;
}

bool compileIf(Parser *parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("ifStatement", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compile(parser, out, KEYWORD_TOKEN_TYPE, "if"));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "("));
  CHECK_COMPILE_RETURN(compileExpression(parser, out));
  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ")"));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "{"));
  CHECK_COMPILE_RETURN(compileStatements(parser, out));
  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "}"));

  current_token = get_token(parser->lexer);

  if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "else"))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, KEYWORD_TOKEN_TYPE));

    CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "{"));
    CHECK_COMPILE_RETURN(compileStatements(parser, out));
    CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "}"));
  }

  print_xml_close_tag("ifStatement", true, &parser->identation_level, out);

  return true;
}

bool compileWhile(Parser *parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("whileStatement", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compile(parser, out, KEYWORD_TOKEN_TYPE, "while"));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "("));
  CHECK_COMPILE_RETURN(compileExpression(parser, out));
  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ")"));

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "{"));
  CHECK_COMPILE_RETURN(compileStatements(parser, out));
  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "}"));

  print_xml_close_tag("whileStatement", true, &parser->identation_level, out);

  return true;
}

bool compileDo(Parser *parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("doStatement", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compile(parser, out, KEYWORD_TOKEN_TYPE, "do"));
  CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

  current_token = get_token(parser->lexer);

  // This is duplicated in compileTerm, it would require to rewrite the lexer to handle token lookahead
  if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
    // Expression list returns -1 when it fails instead of false
    if (compileExpressionList(parser, out) == -1)
    {
      return false;
    }
    CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ")"));
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "."))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "("));
    // Expression list returns -1 when it fails instead of false
    if (compileExpressionList(parser, out) == -1)
    {
      return false;
    }
    CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ")"));
  }
  else
  {
    handle_syntax_error(&current_token, "\"(\", or \".\"");
    return false;
  }

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("doStatement", true, &parser->identation_level, out);

  return true;
}

bool compileReturn(Parser *parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("returnStatement", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compile(parser, out, KEYWORD_TOKEN_TYPE, "return"));

  current_token = get_token(parser->lexer);

  if (check_expression(&current_token))
  {
    CHECK_COMPILE_RETURN(compileExpression(parser, out));
  }

  CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ";"));

  print_xml_close_tag("returnStatement", true, &parser->identation_level, out);

  return true;
}

bool compileExpression(Parser* parser, FILE *out)
{
  Token current_token;

  print_xml_open_tag("expression", true, &parser->identation_level, out);

  CHECK_COMPILE_RETURN(compileTerm(parser, out));

  current_token = get_token(parser->lexer);

  while (check_op(&current_token))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compileTerm(parser, out));

    current_token = get_token(parser->lexer);
  }

  print_xml_close_tag("expression", true, &parser->identation_level, out);

  return true;
}

bool compileTerm(Parser *parser, FILE *out)
{
  Token current_token = get_token(parser->lexer);

  print_xml_open_tag("term", true, &parser->identation_level, out);

  if (check_token_matches(&current_token, INT_CONST_TOKEN_TYPE, NULL))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, INT_CONST_TOKEN_TYPE));
  }
  else if (check_token_matches(&current_token, STRING_CONST_TOKEN_TYPE, NULL))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, STRING_CONST_TOKEN_TYPE));
  }
  else if (check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "true") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "false") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "null") || check_token_matches(&current_token, KEYWORD_TOKEN_TYPE, "this"))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, KEYWORD_TOKEN_TYPE));
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compileExpression(parser, out));
    CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ")"));
  }
  else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "-") || check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "~"))
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
    CHECK_COMPILE_RETURN(compileTerm(parser, out));
  }
  else
  {
    CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));

    current_token = get_token(parser->lexer);
   
    if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "["))
    {
      CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
      CHECK_COMPILE_RETURN(compileExpression(parser, out));
      CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "]"));
    }
    // subroutine call - // This is duplicated in compileDo, it would require to rewrite the lexer to handle token lookahead
    else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "("))
    {
      CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
      // Expression list returns -1 when it fails instead of false
      if (compileExpressionList(parser, out) == -1)
      {
        return false;
      }
      CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ")"));
    }
    else if (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, "."))
    {
      CHECK_COMPILE_RETURN(compile_type(parser, out, SYMBOL_TOKEN_TYPE));
      CHECK_COMPILE_RETURN(compile_type(parser, out, IDENTIFIER_TOKEN_TYPE));
      CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, "("));
      // Expression list returns -1 when it fails instead of false
      if (compileExpressionList(parser, out) == -1)
      {
        return false;
      }
      CHECK_COMPILE_RETURN(compile(parser, out, SYMBOL_TOKEN_TYPE, ")"));
    }
  }

  print_xml_close_tag("term", true, &parser->identation_level, out);

  return true;
}

int compileExpressionList(Parser *parser, FILE *out)
{
  Token current_token = get_token(parser->lexer);
  int num_expressions = 0;

  print_xml_open_tag("expressionList", true, &parser->identation_level, out);

  if (!check_expression(&current_token))
  {
    print_xml_close_tag("expressionList", true, &parser->identation_level, out);
    return 0;
  }

  if(!compileExpression(parser, out))
   return -1;

  current_token = get_token(parser->lexer);

  while (check_token_matches(&current_token, SYMBOL_TOKEN_TYPE, ","))
  {
    if(!(compile_type(parser, out, SYMBOL_TOKEN_TYPE) && compileExpression(parser, out)))
      return -1;
    num_expressions++;

    current_token = get_token(parser->lexer);
  }

  print_xml_close_tag("expressionList", true, &parser->identation_level, out);

  return num_expressions;
}

Parser *init_parser(const char *filename)
{
  Parser *parser = (Parser *)malloc(sizeof(Parser));

  if (parser == NULL)
    return NULL;

  parser->lexer = init_lexer(filename);

  if (parser->lexer == NULL)
  {
    free(parser);
    return NULL;
  }

  parser->identation_level = 0;

  return parser;
}

void fini_parser(Parser *parser)
{
  fini_lexer(parser->lexer);

  free(parser);
}