#ifndef AST_H
#define AST_H
#include "array.h"
#include "token.h"
typedef struct AST_STRUCT AST;

typedef enum
{
    AST_NUMBER,
    AST_STRING, /*to get value of this field access "string" field in AST struct*/

    AST_UNARY,
    AST_BINARY,
    AST_TERNARY,
    AST_SEQUENCE,
    AST_FUNCTION_CALL,
    AST_POSTFIX,
    AST_STMT,

    AST_ID,    /*to get value of this field access "string" field in AST struct*/
    AST_FALSE, /*to get value of this field access "string" field in AST struct*/
    AST_TRUE,  /*to get value of this field access "string" field in AST struct*/
    AST_NULL,  /*to get value of this field access "string" field in AST struct*/
} AST_Type;

typedef enum
{
    BINARY_PLUS,        // +
    BINARY_MINUS,       // -
    BINARY_MULTIPLY,    // *
    BINARY_DIVIDE,      // /
    BINARY_MOD,         // %
    BINARY_AND,         // &&
    BINARY_OR,          // ||
    BINARY_BITWISE_OR,  // |
    BINARY_BITWISE_AND, // &
    BINARY_BITWISE_XOR, // ^
    BINARY_LTE,         // <=
    BINARY_LT,          // <
    BINARY_GTE,         // >=
    BINARY_GT,          // >
    BINARY_LSH,         // <<
    BINARY_RSH,         // >>
    BINARY_EQUALS,      // ==
    BINARY_NOT_EQUALS,  // !=
    BINARY_ASSIGNMENT,  // =
} BinaryType;

typedef enum
{
    UNARY_MINUS,     // -
    UNARY_NOT,       // !
    UNARY_INCREMENT, // ++
    UNARY_DECREMENT, // --
} UnaryType;

typedef enum
{
    STMT_COMPOUND,
    STMT_IF,
    STMT_FOR,
    STMT_WHILE,
    STMT_PRINT,
    STMT_RETURN,
} StatmentType;

typedef enum
{
    POSTFIX_INCR, // ++
    POSTFIX_DECR, // --
} PostfixType;

typedef struct
{
    BinaryType type;
    AST *left;
    AST *right;
} BinaryExpr;

typedef struct
{
    UnaryType type;
    AST *value;
} UnaryExpr;

typedef struct
{
    define_array(statments, AST *);
} StatmentCompound;
typedef struct
{
    AST *condition;
    AST *then;
    AST *other_wise;
} StatmentIf;
typedef struct
{
    AST *expr;
} StatmentPrint;
typedef struct
{
    AST *expr;
} StatmentReturn;

typedef union
{
    StatmentCompound compound;
    StatmentIf _if;
    StatmentPrint print;
    StatmentReturn _return;
} Statment_As;

typedef struct
{
    StatmentType type;
    Statment_As as;
} Statment;

typedef struct
{
    AST *condition;
    AST *then;
    AST *other_wise;
} TernaryExpr;

typedef struct
{
    AST *callee;
    AST *args;
} FunctionCall;

typedef struct
{
    PostfixType type;
    AST *operand;
} PostfixExpr;

typedef struct
{
    define_array(exprs, AST *);
} SequenceExpr;

typedef union
{
    double number;
    char *string;
    UnaryExpr unaryExpr;
    BinaryExpr binaryExpr;
    TernaryExpr ternaryExpr;
    SequenceExpr sequenceExpr;
    FunctionCall functionCall;
    PostfixExpr postfixExpr;
    Statment statment;
} AST_As;

struct AST_STRUCT
{
    AST_Type type;
    AST_As as;
    Token token;
};
AST *init_ast(AST_Type type);
AST *init_primary_ast(AST_Type type, char *string);
AST *init_binary_ast(TokenType type);
AST *init_unary_ast(TokenType type);
AST *init_postfix_ast(TokenType type);
AST *init_stmt_ast(StatmentType type);
char *ast_type_to_str(AST_Type type);
size_t ast_push(AST *ast, AST *child);
char *ast_to_json(AST *ast);
void ast_print(AST *ast);
#endif