#ifndef LEXER_H
#define LEXER_H

typedef enum TOKEN_TYPE
{
  KEYWORD_TOKEN_TYPE,
  SYMBOL_TOKEN_TYPE,
  INT_CONST_TOKEN_TYPE,
  STRING_CONST_TOKEN_TYPE,
  IDENTIFIER_TOKEN_TYPE,
  INVALID_TOKEN_TYPE
} TOKEN_TYPE;

#define TOKEN_MAX_LEN 256

typedef struct Token
{
  TOKEN_TYPE type;
  char token[TOKEN_MAX_LEN + 1];
  int line;
  int column;
} Token;

typedef struct LexCtx LexCtx;

// Gets the string representation of the type of token
const char *token_type_str(TOKEN_TYPE token_type);

// Scans a file and performs lexical analysis
void advance(LexCtx *ctx);

// Returns current scanned token
Token get_token(LexCtx *ctx);

// Initializes a lexer for a input file
LexCtx *init_lexer(const char *filename);

// Frees a lexer and clean resources
void fini_lexer(LexCtx *ctx);

#endif