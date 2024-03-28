# jcc instructions

Created: February 29, 2024 2:10 PM
Last Edited Time: February 29, 2024 4:55 PM
Created By: wang juli
Last Edited By: wang juli

## jcc: compile an integer

```nasm
        .global main
main:
        mov $42, %rax
        ret
```

Here will return 42, then run make test will OK.

## jcc: Add + and - operation

use add and sub assembly instruction to realize + and - operation

- jcc: Add a tokenizer

```c
typedef struct Token Token;
struct Token {
    TokenKind kind; // Token kind
    Token *next;    // Next token
    int val;        // If kind is TK_NUM, its value
    char *loc;      // Token location
    int len;        // Token length
};
```

use a link to store a string e.g “12 + 34 - 5”

gdb --args ./jcc "12 + 34 - 5”

a toeknizer will like

```c
(gdb) p *tok
$1 = {kind = TK_NUM, next = 0x5555555592d0, val = 12, loc = 0x7fffffffe628 "12 + 34 - 5", len = 2}
(gdb) p *tok->next
$2 = {kind = TK_PUNCT, next = 0x555555559300, val = 0, loc = 0x7fffffffe62b "+ 34 - 5", len = 1}
(gdb) p *tok->next->next
$5 = {kind = TK_NUM, next = 0x555555559330, val = 34, loc = 0x7fffffffe62d "34 - 5", len = 2}
(gdb) p *tok->next->next->next
$6 = {kind = TK_PUNCT, next = 0x555555559360, val = 0, loc = 0x7fffffffe630 "- 5", len = 1}
(gdb) p *tok->next->next->next->next
$7 = {kind = TK_NUM, next = 0x555555559390, val = 5, loc = 0x7fffffffe632 "5", len = 1}
(gdb) p *tok->next->next->next->next->next
$8 = {kind = TK_EOF, next = 0x0, val = 0, loc = 0x7fffffffe633 "", len = 0}
```

## jcc: Add *, / and ()

use rest to update the rest of the token
```
    static Node *expr(Token **rest, Token *tok) {
        ...
        *rest = tok;
        ...
    }
    Node *node = expr(&tok, tok);

    if (tok->kind != TK_EOF)
        error_tok(tok, "extra token");
```

```c
typedef struct Node Node;
struct Node {
    NodeKind kind; // Node kind
    Node *lhs;     // Left-hand side
    Node *rhs;     // Right-hand side
    int val;       // Used if kind == ND_NUM
};
```

the last right-hand always store the kind of ND_NUM and the last right-had also store the kind of ND_NUM, others store the Punctuators e.g ND_ADD ND_SUB ND_MUL ND_DIV

**expr** function will do this functionality

```
a.  (3+5)/2                     b. 5*(9-6)                              c. 3*4/2
       /                              *                                      /
     /     \                       /     \                                /    \
    +       2                     5       -                              *      2
 /     \                               /     \                         /    \    
3       5                             9       6                       3      4
```

a expr → mul→primary→expr→mul→primary  left node store (3+5) , expr →mul→primary(&tok, tok->next)right node store 2 

b expr → mul→primary left node store 5 ,  expr → mul→primary→primary→expr→expr→mul→primary(&tok, tok->next)right node store 2

c expr→mul→primary first left node store 3,  expr→mul→primary→primary first leftnode 4. then 3*4 as a  right node, expr→mul→primary→primary second left node

**gen_expr** is a recursive function, and the exit is when the node→kind is ND_NUM

### 3*4/2 assambly code

```nasm
        .globl main
main:
        mov $2, %rax
        push %rax
        mov $4, %rax
        push %rax
        mov $3, %rax
        pop %rdi
        imul %rdi, %rax
        pop %rdi
        cqo
        idiv %rdi
        ret
```

.globl main: This directive declares the main symbol as globally accessible, indicating that it is the entry point of the program.

main:: This label marks the beginning of the main function.

mov $2, %rax: This instruction moves the immediate value 2 into the %rax register.

push %rax: This instruction pushes the value of %rax onto the stack.

mov $4, %rax: This instruction moves the immediate value 4 into the %rax register, overwriting the previous value.

push %rax: This instruction pushes the new value of %rax (which is 4) onto the stack.

mov $3, %rax: This instruction moves the immediate value 3 into the %rax register, overwriting the previous value.

pop %rdi: This instruction pops the top value from the stack and stores it in the %rdi register. The popped value is 4, which was pushed onto the stack earlier.

imul %rdi, %rax: This instruction multiplies the value in the %rax register (which is 3) by the value in the %rdi register (which is 4), storing the result in %rax. After this instruction, %rax will contain 12.

pop %rdi: This instruction pops the top value from the stack (which is 2) and stores it in the %rdi register.

cqo: This instruction sign-extends the value in %rax into the %rdx:%rax register pair. Since %rax contains 12, %rdx will be set to 0 and %rax will remain unchanged.

idiv %rdi: This instruction divides the %rdx:%rax register pair (treated as a single 128-bit value) by the value in the %rdi register (which is 2). The quotient is stored in %rax and the remainder in %rdx. After this instruction, %rax will contain 6.

ret: This instruction returns from the function.

## jcc: Add unary plus and minus

unary minus prior to minus and unary plus prior to plus
in unary judge minus first 'if (equal(tok, "-"))' expr also has 'if (equal(tok, "-"))', but run after unary. plus the same.
we can see the rule from the assembly code
```
root@ubuntu2004:~/virt/jcc# ./jcc '-10'
    .globl main
main:
    mov $10, %rax
    neg %rax
    ret
root@ubuntu2004:~/virt/jcc# ./jcc '1--10'
    .globl main
main:
    mov $10, %rax
    neg %rax
    push %rax
    mov $1, %rax
    pop %rdi
    sub %rdi, %rax
    ret
```

## jcc:  Split main.c into multiple small files
### tokenizer to generate tokenizer
e.g 12 + 34 - 5
```
								   /--->		                       	/--->                               /--->                            /--->                           /---> 
kind = TK_NUM, 					  /		kind = TK_PUNCT,			   /     kind = TK_NUM,				   /	 kind = TK_PUNCT,			/	  kind = TK_NUM,		    /	  kind = TK_NUM,
next = 0x5555555592d0, ----------/	    next = 0x555555559300, -------/	     next = 0x555555559330,	------/		 next = 0x555555559360,	---/	  next = 0x555555559390,---/      next = 0,
val = 12, 								val = 0, 							 val = 34,							 val = 0,						  val = 5,					      val = 0,
loc = 0x7fffffffe628 "12 + 34 - 5", 	loc = 0x7fffffffe62b "+ 34 - 5",	 loc = 0x7fffffffe62d "34 - 5",	     loc = 0x7fffffffe630 "- 5", 	  loc = 0x7fffffffe632 "5",	      loc = 0x7fffffffe633 "",
len = 2									len = 1								 len = 2							 len = 1						  len = 1						  len = 0
```

### parse.c for parse the tokenizer, follow the link rule
https://en.cppreference.com/w/c/language/operator_precedence

### codegen.c to generate the assambly code 
e.g 12 + 34 - 5
```
  .globl main
main:
    mov $5, %rax
    push %rax
    mov $34, %rax
    push %rax
    mov $12, %rax
    pop %rdi
    add %rdi, %rax
    pop %rdi
    sub %rdi, %rax
  ret
```

## jcc: Accept multiple statements separated by semicolons
### parse
in new_unary will store the node the parent left hand
```
static Node *expr_stmt(Token **rest, Token *tok) {
    Node *node = new_unary(ND_EXPR_STMT, expr(&tok, tok));
    *rest = skip(tok, ";");
    return node;
}
```
```
static Node *new_unary(NodeKind kind, Node *expr) {
  Node *node = new_node(kind);
  node->lhs = expr;
  return node;
}
```
### codegen
generate the code from parent left hand node
```
static void gen_stmt(Node *node){
    if(node->kind == ND_EXPR_STMT)
        gen_expr(node->lhs);
    return;
}
```

## jcc: Support single-letter local variables
### tokenize
store 'a' ... 'z' into tokenize
```
if ('a' <= *p && *p <= 'z') {
    cur = cur->next = new_token(TK_IDENT, p, p + 1);
    p ++;
    continue;
}
```
### parse
single-letter local variables follow "；" statement 
stmt->expr_stmt->expr->assign

letter node store in the left hand, value node store in the right hand
```
static Node *assign(Token **rest, Token *tok){
    Node *node = equality(&tok, tok);
    if (equal(tok, "="))
        node = new_binary(ND_ASSIGN, node, assign(&tok, tok->next));
    *rest = tok;
    return node;
}
```
### codegen
variable store in the stack
push variable one by one in stack before assign a value to varibable
pop varibable one by ine in statck while assign a value to varibable
##### assembly code for a=b=3; a+b;
```
.globl main
main:
    push %rbp               ; Save the value of the base pointer (rbp) onto the stack
    mov %rsp, %rbp          ; Set the base pointer (rbp) to the current value of the stack pointer (rsp)
    sub $208, %rsp          ; Allocate 208 bytes of space on the stack for local variables

    lea -8(%rbp), %rax      ; Load effective address of -8(%rbp) into %rax
    push %rax               ; Push the address onto the stack
    lea -16(%rbp), %rax     ; Load effective address of -16(%rbp) into %rax
    push %rax               ; Push the address onto the stack

    mov $3, %rax            ; Move the immediate value 3 into %rax
    pop %rdi                ; Pop the top of the stack into %rdi (first argument)
    mov %rax, (%rdi)        ; Move the value in %rax (3) into the memory location pointed to by %rdi

    pop %rdi                ; Pop the top of the stack into %rdi (second argument)
    mov %rax, (%rdi)        ; Move the value in %rax (3) into the memory location pointed to by %rdi

    lea -16(%rbp), %rax     ; Load effective address of -16(%rbp) into %rax
    mov (%rax), %rax        ; Move the value stored at the address pointed to by %rax into %rax
    push %rax               ; Push the value onto the stack

    lea -8(%rbp), %rax      ; Load effective address of -8(%rbp) into %rax
    mov (%rax), %rax        ; Move the value stored at the address pointed to by %rax into %rax
    pop %rdi                ; Pop the top of the stack into %rdi
    add %rdi, %rax          ; Add the value in %rdi to %rax
    mov %rbp, %rsp          ; Restore the stack pointer to the base pointer (clean up stack)
    pop %rbp                ; Restore the base pointer
    ret                     ; Return from the function
```
## jcc: Support multi-letter local variables
Used Obj list link to record variable include name and offset
```
typedef struct Obj Obj;
struct Obj {
    Obj *next;
    char *name;     // Variable name
    int offset;     // Offset from register 'RBP'
};
```
In Function, body records AST node list, locals records variable list.
stack_size records heap space  that the variable occupied
```
typedef struct Fuction Fuction;
struct Fuction {
    Node *body;
    Obj *locals;
    int stack_size;
};
```

# jcc: Add { ... } body
In node, body records block{...}
sub block also use body to present the block
```
// AST node type
struct Node {
    NodeKind kind;  // Node kind
    Node *next;     // Next node
    Node *lhs;      // Left-hand side
    Node *rhs;      // Right-hand side
    Node *body;     // block {...}
    Obj *var;       // Used if kind == ND_VAR
    int val;        // Used if kind == ND_NUM
};
```
