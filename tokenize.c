#include "jcc.h"

// Input string
static char *current_input;

// Reports an error and exit.
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);    
}

// Reports an error location and exit
static void verror_at(char *loc, char *fmt, va_list ap) {
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(tok->loc, fmt, ap);
}

// Consumes the current token if it matches 'op'
// op[tok->len] == '\0' to ensure that op only has one character
bool equal(Token *tok, char *op){
    return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

// Ensure that the current token is 's'
Token *skip(Token *tok, char *s){
    if (!equal(tok, s)) error_tok(tok, "expected '%s'", s);
    return tok->next;
}

// Create a new token
static Token *new_token(TokenKind kind, char *start, char *end) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = start;
    tok->len = end - start;
    return tok;
}

// https://en.cppreference.com/w/c/language/operator_precedence
static bool startswith(char *p, char *q) {
    return strncmp(p, q, strlen(q)) == 0;
}

// Return True if c is a valid as the first character of an identifier
static bool is_ident1(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

// Return True if c ia valid as a non-first character of an identifier
static bool is_ident2(char c) {
    return is_ident1(c) || ('0' <= c && c <= '9');
}

// Read a punctuator token from p and returns its length
static int read_punct(char *p) {
    if (startswith(p, "==") || startswith(p, "!=") ||
       startswith(p, "<=") || startswith(p, ">="))
       return 2;

    return ispunct(*p) ? 1 : 0;
}

static bool is_keyword(Token *tok) {
    static char *kw[] = {"return", "if", "else"};
    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
        if(equal(tok, kw[i]))
            return true;
    
    return false;
}

static void convert_keyword(Token *tok) {
    for(Token *cur = tok; cur; cur = cur->next) {
        if(is_keyword(cur))
            cur->kind = TK_KEYWORD;
    }
}

// Tokenize 'current_input' and returns new tokens.
Token *tokenize(char *p) {
    current_input = p;
    Token head = {};
    Token *cur = &head;
    while (*p) {
        // Skip whitespace characters
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Numeric literal
        if (isdigit(*p)) {
            // cur(cur->next) point to next cur and create a new tokoen for cur
            // cur->next = new_token(TK_NUM, p, p);
            // cur = cur->next;
            cur = cur->next = new_token(TK_NUM, p, p);
            char *q = p;
            cur->val = strtol(q, &p, 10);
            cur->len = p - q;
            continue;
        }

        // Identifier
        if (is_ident1(*p)) {
            char *start = p;
            do {
                p++;
            }while(is_ident2(*p));
            cur = cur->next = new_token(TK_IDENT, start, p);
            continue;
        }

        // Punctuators
        int punct_len = read_punct(p);
        if (punct_len) {
            cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
            p += cur->len;
            continue;
        }

        // Error
        error_at(p,"unexpected charater: %c", *p);
    }

    cur = cur->next = new_token(TK_EOF, p, p);
    convert_keyword(head.next);
    return head.next;
}