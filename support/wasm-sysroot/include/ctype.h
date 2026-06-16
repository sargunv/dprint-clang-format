#ifndef DPRINT_CLANG_FORMAT_WASM_CTYPE_H
#define DPRINT_CLANG_FORMAT_WASM_CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

int isalnum(int value);
int isalpha(int value);
int isdigit(int value);
int islower(int value);
int isspace(int value);
int isupper(int value);
int isxdigit(int value);
int tolower(int value);
int toupper(int value);

#ifdef __cplusplus
}
#endif

#endif
