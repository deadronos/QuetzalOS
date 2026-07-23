#ifndef QUETZAL_SERPENTC_BUILTINS_H
#define QUETZAL_SERPENTC_BUILTINS_H

#include <stdint.h>
#include <stddef.h>

#define SERPENTC_MAX_VARS 32
#define SERPENTC_MAX_NAME 32

typedef struct {
    char name[SERPENTC_MAX_NAME];
    uint32_t value;
    int is_set;
} symbol_t;

void builtins_init(void);
int symbol_set(const char* name, size_t len, uint32_t val);
int symbol_get(const char* name, size_t len, uint32_t* out_val);
int dispatch_builtin(const char* name, size_t len, const uint32_t* args, size_t arg_count, uint32_t* out_result);

#endif /* QUETZAL_SERPENTC_BUILTINS_H */
