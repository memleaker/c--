#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <map>
#include <list>
#include <string>
#include <memory>

#include "files.h"
#include "argument.h"
#include "content.h"

namespace c89 {

enum TokenType
{
  TK_IDENT,   // Identifiers
  TK_SEPOR,   // Separators
  TK_OPEOR,   // Operators
  TK_STR,     // String literals
  TK_NUM,     // Numeric literals
  TK_CHAR,    // Character literals
};

// c89 32 KeyWords
enum KeyWords
{
    K_IF, K_ELSE,
    K_SWITCH, K_CASE, K_DEFAULT,

    K_FOR, K_WHILE, K_DO,
    K_BREAK, K_CONTINUE, K_GOTO,

    K_AUTO, K_STATIC, K_CONST, K_EXTERN,
    K_REGISTER, K_VOLATILE,

    K_STRUCT, K_UNION, K_ENUM, K_TYPEDEF,

    K_INT, K_CHAR, K_SHORT, K_LONG, K_FLOAT, K_DOUBLE,
    K_SIGNED, K_UNSIGNED,
    
    K_VOID, K_SIZEOF, K_RETURN,
};

enum Operators
{
    /* arithmtic operator: +, -, *, /, % */
    O_PLUS, O_SUB, O_MUL, O_DIV, O_COMP,

    /* incrementing decrementing: ++, -- */
    O_INC, O_DEC,

    /* logical operator: &&, ||, ! */
    O_AND, O_OR, O_NOT,

    /* comparison operator: ==, !=, <, <=, >, >= */
    O_EQUAL, O_NOTEQUAL, O_LOWER, O_LOWEQUAL, O_GREATER, O_GREAEQUAL,

    /* assign operator: =, +=, -=, *=, /=, %=, <<=, >>=, &=, |=, ^=, ~= */
    O_ASSIGN, O_PLUSASSIGN, O_SUBASSIGN, O_MULASSIGN, O_DIVASSIGN, O_COMPASSIGN, 
    O_SHLASSIGN, O_RHLASSIGN,
    O_ANDASSIGN, O_ORASSIGN, O_XORASSIGN, O_NEGASSIGN,

    /* bit shifting: <<, >> */
    O_SHL, O_RHL,

    /* bitwise logical operators: &, |, ^, ~ */
    O_BTIAND, O_BITOR, O_BITXOR, O_BITNEG,

    O_SIZEOF,

    /* Arrow operator: -> */
    O_ARROW,

    /* Conditional Operator: ? */
    O_COND,

    O_UNKNOWN,
};

enum Separators
{
    /* , . : ; ( ) [ ] { } */
    S_COMMA, S_DOT, S_COLON, S_EMICLON,
    S_LPARET, S_RPARET,      /* parentheses */
    S_LSQBRCKT, S_RSQBRCKT,  /* square bracket */
    S_LCUBRCKT, S_RCUBRCKT,  /* curly bracket */
    S_UNKNOWN,
};

enum NumType
{
    N_INT, N_LONG, N_LONGLONG,
    N_UINT, N_ULONG, N_ULONGLONG,
    N_FLOAT, N_DOUBLE, N_LDOUBLE, N_UNKNOWN,
};

// 注释不应该是token，在预处理时就去掉了,
// 也不需要处理反斜杠，也是在预处理时就处理好了
class Token
{
public:
    TokenType type;

    /* Operators */
    Operators operate = O_UNKNOWN;

    /* Separators */
    Separators separator = S_UNKNOWN;

    /* Number */
    NumType numtype = N_UNKNOWN;
    union {
        /* number constant must be greater than 0 */
        uint64_t ullong_literal;
        long double ldouble_literal;
    } number_literal;

    /* Character */
    int char_literal;

    /* String */
    std::string string_literal;
    
    /* Identifier */
    std::string ident_name;

    /* Token Location */
    unsigned int row;
    unsigned int column;
    std::string filename;
};

class Tokenizer {
public:
    void TokenizeFiles(const CcArg& arg, const Files& files);
    void Tokenize(const CcArg& arg, const std::string& file, const std::string& content);

private:
    void TokenIdent(Content& c);
    void TokenPunct(Content& c);
    void TokenPlus(Content& c);
    void TokenSub(Content& c);
    void TokenChar(Content& c);
    void TokenNum(Content& c);
    NumType SuffixType(Content& start, Content& end);
    void TokenInteger(Content& start, Content& suffix, Content& end, int base);
    void TokenFloat(Content& start,  Content& suffix, Content& end);
    void TokenString(Content& c);
    void EscapeChar(Content& c, int *character);
    void OctalChar(Content& c, int *octal);
    void HexChar(Content& c, int *hex);
    int HexchToint(char ch);
    void TokenMul(Content& c);
    void TokenDiv(Content& c);
    void TokenComp(Content& c);
    void TokenNot(Content& c);
    void TokenLower(Content& c);
    void TokenGreater(Content& c);
    void TokenEqual(Content& c);
    void TokenAnd(Content& c);
    void TokenOr(Content& c);
    void TokenXor(Content& c);
    void TokenNeg(Content& c);
    void TokenCond(Content& c);
    void TokenComma(Content& c);
    void TokenDot(Content& c);
    void TokenColon(Content& c);
    void TokenEmiColon(Content& c);
    void TokenLParet(Content& c);
    void TokenRParet(Content& c);
    void TokenLSqbrckt(Content& c);
    void TokenRSqbrckt(Content& c);
    void TokenLCubrckt(Content& c);
    void TokenRCubrckt(Content& c);

public:
    std::list<std::shared_ptr<Token>> tokenlist;
};

}

#endif