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