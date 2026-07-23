#include "serpentc.h"
#include "lexer.h"
#include "builtins.h"
#include "../drivers/vbe.h"
#include "../graphics/console.h"

void serpentc_init(void) {
    /* Initialize SerpentC Heap Arena marker at 0x01000000 */
    uint32_t* serpent_arena = (uint32_t*)0x01000000;
    serpent_arena[0] = 0x53455250; // "SERP" Magic marker
    builtins_init();
}

static uint32_t parse_expression(lexer_t* lex, token_t* tok) {
    if (tok->type == TOKEN_NUMBER) {
        uint32_t val = tok->value;
        *tok = lexer_next(lex);
        return val;
    } else if (tok->type == TOKEN_IDENTIFIER) {
        uint32_t val = 0;
        if (!symbol_get(tok->start, tok->length, &val)) {
            val = 0;
        }
        *tok = lexer_next(lex);
        return val;
    }
    return 0;
}

void serpentc_eval(const char* script) {
    if (!script) return;

    lexer_t lex;
    lexer_init(&lex, script);

    token_t tok = lexer_next(&lex);

    while (tok.type != TOKEN_EOF && tok.type != TOKEN_ERROR) {
        if (tok.type == TOKEN_IDENTIFIER) {
            token_t name_tok = tok;
            tok = lexer_next(&lex);

            /* Check assignment: IDENTIFIER = expression ; */
            if (tok.type == TOKEN_ASSIGN) {
                tok = lexer_next(&lex);
                uint32_t val = parse_expression(&lex, &tok);
                symbol_set(name_tok.start, name_tok.length, val);
                console_print("[SerpentC] Variable assigned.");
                if (tok.type == TOKEN_SEMICOLON) {
                    tok = lexer_next(&lex);
                }
            }
            /* Check call: IDENTIFIER ( arg1, arg2, ... ) ; */
            else if (tok.type == TOKEN_LPAREN) {
                tok = lexer_next(&lex);
                uint32_t args[8];
                size_t arg_count = 0;

                while (tok.type != TOKEN_RPAREN && tok.type != TOKEN_EOF && tok.type != TOKEN_ERROR) {
                    uint32_t arg_val = parse_expression(&lex, &tok);
                    if (arg_count < 8) {
                        args[arg_count++] = arg_val;
                    }
                    if (tok.type == TOKEN_COMMA) {
                        tok = lexer_next(&lex);
                    } else if (tok.type != TOKEN_RPAREN) {
                        break;
                    }
                }

                if (tok.type == TOKEN_RPAREN) {
                    tok = lexer_next(&lex);
                }

                uint32_t result = 0;
                if (dispatch_builtin(name_tok.start, name_tok.length, args, arg_count, &result)) {
                    console_print("[SerpentC] Command executed.");
                } else {
                    console_print("[SerpentC] Error: unknown function.");
                }

                if (tok.type == TOKEN_SEMICOLON) {
                    tok = lexer_next(&lex);
                }
            } else {
                /* Skip unmapped token */
                tok = lexer_next(&lex);
            }
        } else {
            tok = lexer_next(&lex);
        }
    }
}
