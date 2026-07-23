#include "lexer.h"

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static int is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

void lexer_init(lexer_t* lex, const char* src) {
    lex->src = src ? src : "";
    lex->pos = 0;
}

token_t lexer_next(lexer_t* lex) {
    token_t tok;
    tok.type = TOKEN_EOF;
    tok.start = NULL;
    tok.length = 0;
    tok.value = 0;

    /* Skip whitespace */
    while (lex->src[lex->pos] != '\0') {
        char c = lex->src[lex->pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            lex->pos++;
        } else {
            break;
        }
    }

    if (lex->src[lex->pos] == '\0') {
        tok.type = TOKEN_EOF;
        tok.start = &lex->src[lex->pos];
        return tok;
    }

    const char* start = &lex->src[lex->pos];
    char c = lex->src[lex->pos];

    /* Identifiers */
    if (is_alpha(c)) {
        size_t len = 0;
        while (is_alnum(lex->src[lex->pos])) {
            len++;
            lex->pos++;
        }
        tok.type = TOKEN_IDENTIFIER;
        tok.start = start;
        tok.length = len;
        return tok;
    }

    /* Numbers (Decimal or Hex) */
    if (is_digit(c)) {
        uint32_t val = 0;
        size_t len = 0;

        /* Hexadecimal number prefix 0x or 0X */
        if (c == '0' && (lex->src[lex->pos + 1] == 'x' || lex->src[lex->pos + 1] == 'X')) {
            lex->pos += 2;
            len += 2;
            while (is_hex_digit(lex->src[lex->pos])) {
                char hc = lex->src[lex->pos];
                uint32_t d = 0;
                if (hc >= '0' && hc <= '9') d = hc - '0';
                else if (hc >= 'a' && hc <= 'f') d = 10 + (hc - 'a');
                else if (hc >= 'A' && hc <= 'F') d = 10 + (hc - 'A');

                val = (val << 4) | d;
                len++;
                lex->pos++;
            }
        } else {
            /* Decimal number */
            while (is_digit(lex->src[lex->pos])) {
                val = (val * 10) + (lex->src[lex->pos] - '0');
                len++;
                lex->pos++;
            }
        }

        tok.type = TOKEN_NUMBER;
        tok.start = start;
        tok.length = len;
        tok.value = val;
        return tok;
    }

    /* Single character tokens */
    lex->pos++;
    tok.start = start;
    tok.length = 1;

    switch (c) {
        case '(': tok.type = TOKEN_LPAREN; break;
        case ')': tok.type = TOKEN_RPAREN; break;
        case ',': tok.type = TOKEN_COMMA; break;
        case '=': tok.type = TOKEN_ASSIGN; break;
        case ';': tok.type = TOKEN_SEMICOLON; break;
        default:  tok.type = TOKEN_ERROR; break;
    }

    return tok;
}
