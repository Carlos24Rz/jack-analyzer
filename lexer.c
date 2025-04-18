#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

typedef struct FileCtx
{
  FILE *file;
  int line;
  int column;
} FileCtx;

struct LexCtx
{
  Token current_token;
  FileCtx file_ctx;
};

// checks if the string is a valid jack keyword
bool is_keyword(char *str)
{
  const char *jack_keywords[] = {
    "class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return"
  };
  int i = 0;

  for (i = 0; i < (sizeof(jack_keywords)/sizeof(jack_keywords[0])); i++)
  {
    if (strcmp(str, jack_keywords[i]) == 0)
      return true;
  }

  return false;
}

// checks if the character is a valid jack symbol
bool is_symbol(char c)
{
  const char jack_symbols[] = {'{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', '>', '=', '~'};
  int i = 0;

  for (i = 0; i < (sizeof(jack_symbols)/sizeof(jack_symbols[0])); i++)
  {
    if (c == jack_symbols[i])
      return true;
  }

  return false;
}

// Gets the string representation of the type of token
const char *token_type_str(TOKEN_TYPE token_type)
{
  switch (token_type)
  {
    case KEYWORD_TOKEN_TYPE:
      return "keyword";
    case SYMBOL_TOKEN_TYPE:
      return "symbol";
    case INT_CONST_TOKEN_TYPE:
      return "integer";
    case STRING_CONST_TOKEN_TYPE:
      return "string";
    case IDENTIFIER_TOKEN_TYPE:
      return "identifier";
    default:
      return "unknown";
  }
}

// Print the captured token information
void print_token(Token *token)
{
  printf("[%s] '%s' at line %d, column %d\n", token_type_str(token->type), token->token, token->line, token->column);
}

void init_token(Token *token, TOKEN_TYPE token_type, char *token_str, int line, int column)
{
  token->type = token_type;
  strcpy(token->token, token_str);
  token->line = line;
  token->column = column;
}

// Reads a character and updates current file location
char read_char(FileCtx *ctx)
{
  char c = fgetc(ctx->file);

  if (c == '\n')
  {
    ctx->line += 1;
    ctx->column = 0;
  }
  else if (c != EOF)
  {
    ctx->column += 1;
  }

  return c;
}

void unread_char(FileCtx *ctx, char c)
{
  char pushed_c = ungetc(c, ctx->file);

  if (pushed_c == '\n')
  {
    // previous column value cannot be restored
    ctx->line -= 1;
  }
  else if (pushed_c != EOF)
  {
    ctx->column -= 1;
  }
}

// Returns current scanned token
Token get_token(LexCtx *ctx)
{
  return ctx->current_token;
}

// Scans a file and performs lexical analysis
void advance(LexCtx *ctx)
{
  FileCtx *file_ctx = &ctx->file_ctx;
  char c;

  while ((c = read_char(file_ctx)) != EOF)
  {
    // ignore whitespace
    if (isspace(c))
    {
      continue;
    }
    // line comment or start of multiline comment
    else if (c == '/')
    {
      char next_c = read_char(file_ctx);

      // single comment - move to next line
      if (next_c == '/')
      {
        while ((c = read_char(file_ctx)) != '\n' && c != EOF) ;

        continue;
      }
      // multiline comment - move to end of multiline comment
      else if (next_c == '*')
      {
        char prev_c = 0;

        while ((c = read_char(file_ctx)) != EOF && !(prev_c == '*' && c == '/'))
        {
          prev_c = c;
        }

        if (c == EOF)
        {
          fprintf(stderr, "Incomplete comment at line %d, column %d\n", file_ctx->line, file_ctx->column);
          init_token(&ctx->current_token, INVALID_TOKEN_TYPE, "", file_ctx->line, file_ctx->column);
          return;
        }

        continue;
      }
      else
      {
        unread_char(file_ctx, next_c);
      }
    }

    // handle symbols
    if (is_symbol(c))
    {
      char c_str[2] = {c, '\0'};
      // print_token(SYMBOL_TOKEN_TYPE, c_str, ctx->line, ctx->column);
      init_token(&ctx->current_token, SYMBOL_TOKEN_TYPE, c_str, file_ctx->line, file_ctx->column);
      return;
    }

    // handle strings
    if (c == '"')
    {
      char prev = 0;
      char str[TOKEN_MAX_LEN + 1];
      int i = 0;

      while (((c = read_char(file_ctx)) != EOF && c != '"' && c != '\n') || (prev == '\\' && c == '"'))
      {
        prev = c;
        str[i++] = c;
      }

      str[i] = '\0';

      if (c == '"')
      {
        //print_token(STRING_CONST_TOKEN_TYPE, str, ctx->line, ctx->column - strlen(str));
        init_token(&ctx->current_token, STRING_CONST_TOKEN_TYPE, str, file_ctx->line, file_ctx->column - strlen(str));
        return;
      }
      else
      {
        fprintf(stderr, "Incomplete string at line %d, column %d\n", file_ctx->line, file_ctx->column - (int)strlen(str));
        init_token(&ctx->current_token, INVALID_TOKEN_TYPE, str, file_ctx->line, file_ctx->column - (int)strlen(str));
        return;
      }
    }

    // handle integers
    if (isdigit(c))
    {
      char str[TOKEN_MAX_LEN] = {c};
      int integer;
      int i = 1;

      while (isdigit((c = read_char(file_ctx))))
      {
        str[i++] = c;
      }

      str[i] = '\0';

      integer = atoi(str);

      if (integer >= 0 && integer <= 32767)
      {
        // print_token(INT_CONST_TOKEN_TYPE, str, ctx->line, ctx->column - strlen(str));
        init_token(&ctx->current_token, INT_CONST_TOKEN_TYPE, str, file_ctx->line, file_ctx->column - strlen(str));
        unread_char(file_ctx, c);
        return;
      }
      else
      {
        fprintf(stderr, "Out of range integer %d at line %d, column %d\n", integer, file_ctx->line, file_ctx->column - (int)strlen(str));
        init_token(&ctx->current_token, INVALID_TOKEN_TYPE, str, file_ctx->line, file_ctx->column - (int)strlen(str));
        return;
      }
    }

    // handle identifiers and keywords
    if (isalpha(c) || c == '_')
    {
      char str[TOKEN_MAX_LEN] = {c};
      int i = 1;

      while (isalnum((c = read_char(file_ctx))) || c == '_')
      {
        str[i++] = c;
      }

      str[i] = '\0';

      if (is_keyword(str))
      {
        //print_token(KEYWORD_TOKEN_TYPE, str, ctx->line, ctx->column - (int)strlen(str));
        init_token(&ctx->current_token, KEYWORD_TOKEN_TYPE, str, file_ctx->line, file_ctx->column - strlen(str));
        unread_char(file_ctx, c);
        return;
      }
      else
      {
        //print_token(IDENTIFIER_TOKEN_TYPE, str, ctx->line, ctx->column - (int)strlen(str));
        init_token(&ctx->current_token, IDENTIFIER_TOKEN_TYPE, str, file_ctx->line, file_ctx->column - strlen(str));
        unread_char(file_ctx, c);
        return;
      }
    }
    else
    {
      fprintf(stderr, "Unknown token at line %d, column %d\n", file_ctx->line, file_ctx->column);
      init_token(&ctx->current_token, INVALID_TOKEN_TYPE, "", file_ctx->line, file_ctx->column);
      return;
    }
  }

  init_token(&ctx->current_token, INVALID_TOKEN_TYPE, "", file_ctx->line, file_ctx->column);
}

LexCtx *init_lexer(const char *filename)
{
  LexCtx *ctx;

  ctx = (LexCtx *)malloc(sizeof(LexCtx));

  ctx->file_ctx.file = fopen(filename, "r");
  ctx->file_ctx.column = 0;
  ctx->file_ctx.line = 1;
  
  return ctx;
}

void fini_lexer(LexCtx *ctx)
{
  fclose(ctx->file_ctx.file);

  free(ctx);
}