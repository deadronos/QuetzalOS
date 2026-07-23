#ifndef QUETZAL_SERPENTC_LEXER_H
#define QUETZAL_SERPENTC_LEXER_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_LPAREN,   // '('
    TOKEN_RPAREN,   // ')'
    TOKEN_COMMA,    // ','
    TOKEN_ASSIGN,   // '='
    TOKEN_SEMICOLON,// ';'
    TOKEN_ERROR
} token_type_t;

typedef struct {
    token_type_t type;
    const char* start;
    size_t length;
    uint32_t value; // Computed numerical value if TOKEN_NUMBER
} token_t;

typedef struct {
    const char* src;
    size_t pos;
} lexer_t;

void lexer_init(lexer_t* lex, const char* src);
token_t lexer_next(lexer_t* lex);

#endif /* QUETZAL_SERPENTC_LEXER_H */
