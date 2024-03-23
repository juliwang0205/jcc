#ifndef __JCC_H_
#define __JCC_H_

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;

//
// tokenize.c
//

// Token
typedef enum {
    TK_IDENT, // Identifiers e.g a=3 a is a identifier
    TK_PUNCT, // Punctuators
    TK_NUM,   // Numeric literals
    TK_EOF,   // End-of-file
}TokenKind;

//Token type
typedef struct Token Token;
struct Token {
    TokenKind kind; // Token kind
    Token *next;    // Next token
    int val;        // If kind is TK_NUM, its value
    char *loc;      // Token location
    int len;        // Token length
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);

//
// Parser
//

// Local variable
typedef struct Obj Obj;
struct Obj {
    Obj *next;
    char *name;     // Variable name
    int offset;     // Offset from register 'RBP' 
};

// Function
typedef struct Fuction Fuction;
struct Fuction {
    Node *body;
    Obj *locals;
    int stack_size;
};

// AST node
typedef enum {
    ND_ADD,         // +
    ND_SUB,         // -
    ND_MUL,         // *
    ND_DIV,         // /
    ND_NEG,         // unary -
    ND_EQ,          // ==
    ND_NE,          // !=
    ND_LT,          // <
    ND_LE,          // <=
    ND_ASSIGN,      // =
    ND_EXPR_STMT,   // ;
    ND_VAR,         // Variable
    ND_NUM,         // Integer
} NodeKind;

// AST node type
struct Node {
    NodeKind kind;  // Node kind
    Node *next;     // Next node
    Node *lhs;      // Left-hand side
    Node *rhs;      // Right-hand side
    Obj *var;       // Used if kind == ND_VAR
    int val;        // Used if kind == ND_NUM
};

Fuction *parse(Token *tok);

//
// codegen.c
//

void codegen(Fuction *prog);

#endif