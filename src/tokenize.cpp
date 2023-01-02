#include <ostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <cctype>
#include <cstring>
#include <cstdlib>

#include "log.h"
#include "tokenize.h"

namespace c89 {

static const std::map<std::string, KeyWords> keywords = {
    {"if", K_IF}, {"else", K_ELSE}, 
    {"switch", K_SWITCH}, {"case", K_CASE}, {"default", K_DEFAULT},
    {"for", K_FOR}, {"while", K_WHILE}, {"do", K_DO},
    {"break", K_BREAK}, {"continue", K_CONTINUE}, {"goto", K_GOTO},
    {"auto", K_AUTO}, {"static", K_STATIC}, {"const", K_CONST}, {"extern", K_EXTERN},
    {"register", K_REGISTER}, {"volatile", K_VOLATILE}, 
    {"struct", K_STRUCT}, {"union", K_UNION}, {"enum", K_ENUM}, {"typedef", K_TYPEDEF},
    {"int", K_INT}, {"char", K_CHAR}, {"short", K_SHORT}, {"long", K_LONG}, {"float", K_FLOAT},
    {"double", K_DOUBLE}, {"signed", K_SIGNED}, {"unsigned", K_UNSIGNED},
    {"void", K_VOID}, {"sizeof", K_SIZEOF}, {"return", K_RETURN},
};

static const std::map<Operators, std::string> operators = {
    {O_PLUS, "+"}, {O_SUB, "-"}, {O_MUL, "*"}, {O_DIV, "/"},
    {O_INC, "++"}, {O_DEC, "--"},
    {O_AND, "&&"}, {O_OR, "||"}, {O_NOT, "!"},
    {O_EQUAL, "=="}, {O_NOTEQUAL, "!="}, {O_LOWER, "<"}, {O_LOWEQUAL, "<="},
    {O_GREATER, ">"}, {O_GREAEQUAL, ">="}, 
    {O_ASSIGN, "="}, {O_PLUSASSIGN, "+="}, {O_SUBASSIGN, "-="}, {O_MULASSIGN, "*="},
    {O_DIVASSIGN, "/="}, {O_COMPASSIGN, "%="}, {O_SHLASSIGN, "<<="}, {O_RHLASSIGN, ">>="},
    {O_ANDASSIGN, "&="}, {O_ORASSIGN, "|="}, {O_XORASSIGN, "^="}, {O_NEGASSIGN, "~="},
    {O_SHL, "<<"}, {O_RHL, ">>"}, {O_BTIAND, "&"}, {O_BITOR, "|"}, {O_BITXOR, "^"}, {O_BITNEG, "~"},
    {O_ARROW, "->"}, {O_COND, "?"},
};

static const std::map<Separators, std::string> separators = {
    {S_COMMA, ","}, {S_DOT, "."}, {S_COLON, ":"}, {S_EMICLON, ";"},
    {S_LPARET, "("}, {S_RPARET, ")"}, {S_LSQBRCKT, "["}, {S_RSQBRCKT, "]"},
    {S_LCUBRCKT, "{"}, {S_RCUBRCKT, "}"},
};

void Tokenizer::TokenizeFiles(const CcArg& arg, const Files& files)
{
    std::ifstream ifs;
    std::ostringstream stream;

    // 这里应只遍历tmpcfiles
    for (const auto& vec : {files.cfiles, files.tmpcfiles})
    {
        for (const auto& s : vec)
        {
            ifs.open(s, std::ios::in);
            if (!ifs.is_open()) {
                Error::Fatal("File "+s+" not found");
            }

            stream << ifs.rdbuf();

            Tokenize(arg, s, stream.str());

            stream.clear();
            ifs.close();
        }
    }

    // debug
    for (auto& t : tokenlist)
    {
        if (t->type == TK_IDENT) {
            std::cout << t->ident_name << std::endl;
        }
        else if (t->type == TK_STR) {
            std::cout << "\"" << t->string_literal << "\"" << std::endl;
        }
        else if (t->type == TK_CHAR) {
            std::cout << t->char_literal << std::endl;
        }
        else if (t->type == TK_OPEOR) {
            auto it = operators.find(t->operate);
            if (it == operators.end()) {
                Error::Fatal("internal error");
            }
            std::cout << it->second << std::endl;
        }
        else if (t->type == TK_SEPOR) {
            auto it = separators.find(t->separator);
            if (it == separators.end()) {
                Error::Fatal("internal error");
            }
            std::cout << it->second << std::endl;
        }
        else if (t->type == TK_NUM) {
            if ((int)t->numtype > N_ULONGLONG) {
                // cout 默认输出6位, setprecision 设置精度
                std::cout << std::setprecision(16) << t->number_literal.ldouble_literal << std::endl;
            } else {
                std::cout << t->number_literal.ullong_literal << std::endl;
            }
        }
     }
}

void Tokenizer::Tokenize(const CcArg& arg, const std::string& file, const std::string& content)
{
    Content c (file, content);

    while (*c)
    {
        // skip space
        if (isspace(*c) || iscntrl(*c) || isblank(*c)) {
            ++c;
            continue;
        }
        // identifier
        else if (isalpha(*c) || *c=='_') {
            TokenIdent(c);
        // number
        } else if (isdigit(*c) || (*c == '.' && isdigit(*(c+1)))) {
            TokenNum(c);
        // string
        } else if (*c == '"') {
            TokenString(c);
        // character
        } else if (*c == '\'') {
            TokenChar(c);
        // operators or separators
        } else if (ispunct(*c)) {
            TokenPunct(c);
        } else {
            Error::Fatal(c.Location() + "Unrecognized Character" + c.Currline());
        }
    }
}

/* Identifier */
void Tokenizer::TokenIdent(Content& c)
{
    const char* start {c.Str()};
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    while (*(++c))
    {
        /* |a-z| |A-Z| _ |0-9| */
        if (isalpha(*c) || *c == '_' || isdigit(*c)) {
            continue;
        }

        break;
    }

    /* new token */
    tok->type = TK_IDENT;
    tok->ident_name = std::string(start, c.Str());
    tokenlist.emplace_back(tok);
}

/* String */
void Tokenizer::TokenString(Content& c)
{
    Content start {c};
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    while (*++c)
    {
        /* find terminated character */
        if (*c == '\"') {

            /* skip \" */
            if (*(c-1) == '\\') {
                continue;
            }

            /* get string */
            tok->type = TK_STR;
            tok->string_literal = std::string(start+1, c.Str());
            tokenlist.emplace_back(tok);

            /* jump terminated character \" */
            ++c;
            return;
        }
    }

    Error::Fatal(start.Location() + "Missing terminating \" character" + start.Currline());
}

/*
 * number literal
 *
 * |type|    |example|
 * int       1, 0x1, 01
 * uint      1U, 0x1U, 01U
 * long      1L, 0x1L, 01L
 * ulong     1UL, 0x1UL, 01UL
 * float     1.1F
 * double    1.1, 1e10, 1.1e10 (默认为double, 科学计数法也是double)
 * ldouble   1.1L
 * 
 * C99标准增加了一种新的浮点型常量格式--用十六进制表示浮点型常量，即在十六进制数前加上0x或者0X前缀，用p和P分别代替e和E，用2的幂代替10的幂。
 * 例如：0xa.1fp10
 * c89 仅支持10进制浮点数常量
 */
void Tokenizer::TokenNum(Content& c)
{
    int loop {1};
    int base {10};
    Content start {c};
    NumType type {N_INT};

    /* get number base */
    if (*start == '0') {
        base = 8;
        if ((*(start+1) == 'x'||*(start+1)=='X') && isxdigit(*(start+2))) {
            base = 16;
            start += 2;
            c += 2;
        }
    }

    /* find suffix or end */
    while (*c)
    {
        /* float, 由于e作为科学计数法标识且是16进制字符，因此当base为16时，e不能被识别为科学计数法标识 */
        if (*c == '.' || ((*c == 'e' || *c == 'E') && base != 16)) {
            type = N_DOUBLE;
            ++c;
        } else if (base == 16 && isxdigit(*c)) {
            ++c;
        } else if (base == 8 && *c >= '0' && *c <= '7') {
            ++c;
        } else if (base == 10 && isdigit(*c)) {
            ++c;
        } else {
            break;
        }
    }

    /* find number end */
    Content suffix {c};
    while (*c && loop)
    {
        switch (*c)
        {
        case 'f': case 'F':
        case 'l': case 'L':
        case 'u': case 'U':
            ++c;
            break;
        /* termniated */
        default:
            loop = 0;
        }
    }

    if (type == N_INT) {
        TokenInteger(start, suffix, c, base);
    } else {
        TokenFloat(start, suffix, c);
    }
}

void Tokenizer::TokenInteger(Content& start, Content& suffix, Content& end, int base)
{
    int cnt {0};
    NumType sufftype {N_INT};
    uint64_t pow {1}, result{0};
    std::shared_ptr<Token> tok = std::make_shared<Token>();

    if (start == end) {
        return;
    }

    /* calculate */
    cnt = suffix - start;
    if (base == 10) {
        while (cnt-- > 0)
        {
            result += (pow * (*(start+cnt) - '0'));
            pow *= 10;
        }
    } else if (base == 8) {
        while (cnt-- > 0)
        {
            result += (pow * (*(start+cnt) - '0'));
            pow <<= 3;
        }
    } else {
        while (cnt-- > 0)
        {
            result += (pow * HexchToint(*(start+cnt)));
            pow <<= 4;
        }
    }

    /* get suffix type, note: SuffixType will change suffix */
    sufftype = SuffixType(suffix, end);

    /* check type */
    if ((int)sufftype > N_ULONGLONG)
    {
        Error::Fatal(end.Location() + "invalid suffix on int constant" +
                     end.Currline());
    }

    /* new token */
    tok->type = TK_NUM;
    tok->numtype = sufftype;
    tok->number_literal.ullong_literal = result;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenFloat(Content& start, Content& suffix, Content& end)
{
    NumType sufftype{N_INT};
    std::shared_ptr<Token> tok = std::make_shared<Token>();

    if (start == end) {
        return;
    }
    
    /* calculate before 'SuffixType', because SuffixType will change suffix */
    std::string num(start.Str(), suffix.Str());
    tok->number_literal.ldouble_literal = strtold(num.c_str(), NULL);

    /* get suffix type, note: SuffixType will change suffix */
    sufftype = SuffixType(suffix, end);

    /* check number type */
    switch (sufftype)
    {
    case N_FLOAT:
        break;
    case N_LONG:
        sufftype = N_LDOUBLE;
        break;
    case N_INT:
        sufftype = N_DOUBLE;
        break;
    default:
        Error::Fatal(end.Location() + "invalid suffix on float constant" + 
            end.Currline());
    }

    /* new token */
    tok->type = TK_NUM;
    tok->numtype = sufftype;
    tokenlist.emplace_back(tok);
}

/* U, UL, ULL, L, LU, LLU, LL, F, */
NumType Tokenizer::SuffixType(Content& start, Content& end)
{
    int longcnt {0};
    int ucnt {0};
    NumType type{N_INT};

    if (start == end) {
        return type;
    }

    switch (*start)
    {
    case 'f':
    case 'F':
        if (++start != end) {
            Error::Fatal(start.Location() + "invalid suffix on floating constant" + 
                            start.Currline());
        }
        return N_FLOAT;
    case 'u':
    case 'U':
        type = N_UINT;
        longcnt = 0;

        while (++start != end)
        {
            if (*start == 'L' || *start == 'l') {
                longcnt++;

                if (longcnt > 2) {
                    Error::Fatal(start.Location() + "invalid suffix on int constant" + 
                                 start.Currline());
                }
            } else {
                Error::Fatal(start.Location() + "invalid suffix on int constant" + 
                                start.Currline());
            }
        }

        return (NumType)((int)type + longcnt);
    case 'l':
    case 'L':
        type = N_INT;
        longcnt = 1;

        while (++start != end)
        {
            if (*start == 'L' || *start == 'l') {
                longcnt++;
                if (longcnt > 2) {
                    Error::Fatal(start.Location() + "invalid suffix on int constant" + 
                                 start.Currline());
                }
            } else if (*start == 'U' || *start == 'u') {
                ucnt++;
                type = N_UINT;

                if (ucnt > 1) {
                    Error::Fatal(start.Location() + "invalid suffix on int constant" + 
                                 start.Currline());
                }
            } else {
                Error::Fatal(start.Location() + "invalid suffix on int constant" + 
                            start.Currline());
            }
        }

        return (NumType)((int)type + longcnt);
        break;
    default:
        Error::Fatal(start.Location() + "invalid suffix" + '\'' + *start + '\'' +
                start.Currline());
    }

    return type;
}

/* 
 * 在C语言中字符常量'a'具有int类型，sizeof('a') 等于 sizeof(int)
 * 在C++中字符字面量'a'具有char类型，sizeof('a') 为1

 * C语言使用前缀L u U分別表示wchar_t char16_t char32_t等字符类型的字面量。例如: u'y'

 * 由于字符型字面量可能不属于C/C++的token的字符范围，就需要用反斜线\开始的转义(esacpe)序列來表示一个字符值：
 * 简单转义序列：\' \" \? \\ \a \b \f \n \r \t \v 共計11个字符
 * 八进制转义序列：如 \1 \12 \123等等，直至不是八进制数字为止，最多使用3位八进制数字
 * 十六进制转义序列：如 \x1abf4 ，可以使用任意多的十六进制数字，直至不是十六进制数字为止
 * 通用字符名（universe-character name）：\u后面必须跟4个十六进制数字（不足四位前面用0补齐），表示Unicode中在0至0xFFFF之內的码点（但不能表示0xD800到0xDFFF之內的碼点，Unicode标准规定这个范围内的码点保留，不表示字符）
 * 32位的通用字符名：\U后面必须跟8个十六进制数字（不足八位前面用零补齐），表示Unicode中所有可能的码点（除0xD800到0xDFFF之外）

 * C89 不支持通用字符名, 也不支持在字符常量中使用前缀 L, u, U
*/
void Tokenizer::TokenChar(Content& c)
{
    Content start {c};
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    /* get character from '' */
    if (*++c)
    {
        /* end */
        if (*c == '\'') {
            Error::Fatal(start.Location() + "empty character constant" +
                         start.Currline());
        }
        /* get escape character */
        else if (*c == '\\') {
            EscapeChar(c, &tok->char_literal);
            if (*c != '\'') {
                Error::Fatal(start.Location() + "Multi-character character constant" +
                             start.Currline());
            }

            /* jump terminated character */
            ++c;
        }
        /* normal character */
        else {
            if (*(c+1) != '\'') {
                Error::Fatal(start.Location() + "Multi-character character constant" +
                             start.Currline());
            }
            tok->char_literal = *c;
            c += 2;
        }

        tok->type = TK_CHAR;
        tokenlist.emplace_back(tok);
    }
}

void Tokenizer::EscapeChar(Content& c, int *character)
{
    switch (*++c)
    {
    case '\'':
        *character = '\'';
        ++c;
        break;
    case '\"':
        *character = '\"';
        ++c;
        break;
    case '?':
        *character = '\?';
        ++c;
        break;
    case '\\':
        *character = '\\';
        ++c;
        break;
    case 'a':
        *character = '\a';
        ++c;
        break;
    case 'b':
        *character = '\b';
        ++c;
        break;
    case 'f':
        *character = '\f';
        ++c;
        break;
    case 'n':
        *character = '\n';
        ++c;
        break;
    case 'r':
        *character = '\r';
        ++c;
        break;
    case 't':
        *character = '\t';
        ++c;
        break;
    case 'v':
        *character = '\v';
        ++c;
        break;
    case 'x':
        HexChar(c, character);
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        OctalChar(c, character);
        break;
    default:
        Error::Fatal(c.Location() + "unknown escape character \\" + *c + c.Currline());
    }
}

/* \23, \0, \777 */
void Tokenizer::OctalChar(Content& c, int *octal)
{
    int cnt {0};
    int result {0};
    int pow {1};
    const char* start {c.Str()};

    while (*c)
    {
        /* octal 0-7 */
        if (*c >= '0' && *c <= '7') {
            cnt++;
            ++c;

            /* max 3 character */
            if (cnt > 3) {
                Error::Fatal(c.Location() + "multi-character character constant" + c.Currline());
            }
        }
        /* terminated */
        else if (*c == '\'') {
            break;
        } else {
            Error::Fatal(c.Location() + "multi-character character constant" + c.Currline());
        }
    }

    /* octal to dec */
    while (cnt-- > 0)
    {
        result += ((pow) * (int)(*(start+cnt) - '0'));
        pow <<= 3;
    }

    *octal = result;
}

/* \xff, \x1234, \xAB */
void Tokenizer::HexChar(Content& c, int *hex)
{
    int cnt{0};
    int result {0};
    int pow {1};
    const char* start {c+1};

    /* if character is '\x' */
    if (*++c == '\'') {
        Error::Fatal(c.Location() + "used with no following hex digits" + c.Currline());
    }

    while (*c)
    {
        /* 0-9, a-f, A-F */
        if (isxdigit(*c)) {
            cnt++;
            ++c;
        }
        /* terminated character */
        else if (*c == '\'') {
            break;
        } else {
            /* character is not hex character */
            Error::Fatal(c.Location() + "multi-character character constant" + c.Currline());
        }
    }

    /* hex to dec */
    while (cnt-- > 0)
    {
        result += ((pow) * HexchToint(*(start+cnt)));
        pow <<= 4;
    }

    *hex = result;
}

int Tokenizer::HexchToint(char ch)
{
    switch (ch)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return ch - '0';
        break;
    case 'a':
    case 'A':
        return 10;
        break;
    case 'b':
    case 'B':
        return 11;
        break;
    case 'c':
    case 'C':
        return 12;
        break;
    case 'd':
    case 'D':
        return 13;
        break;
    case 'e':
    case 'E':
        return 14;
        break;
    case 'f':
    case 'F':
        return 15;
        break;
    default:
        Error::Fatal("HexchToint internal error");
    }

    return 0;
}

/* Operators Or Separators */
void Tokenizer::TokenPunct(Content& c)
{
    switch (*c)
    {
    /* Operators */
    case '+':
        TokenPlus(c);
        break;
    case '-':
        TokenSub(c);
        break;
    case '*':
        TokenMul(c);
        break;
    case '/':
        TokenDiv(c);
        break;
    case '%':
        TokenComp(c);
        break;
    case '!':
        TokenNot(c);
        break;
    case '<':
        TokenLower(c);
        break;
    case '>':
        TokenGreater(c);
        break;
    case '=':
        TokenEqual(c);
        break;
    case '&':
        TokenAnd(c);
        break;
    case '|':
        TokenOr(c);
        break;
    case '^':
        TokenXor(c);
        break;
    case '~':
        TokenNeg(c);
        break;
    case '?':
        TokenCond(c);
        break;

    /* Separators */
    case ',':
        TokenComma(c);
        break;
    case '.':
        TokenDot(c);
        break;
    case ':':
        TokenColon(c);
        break;
    case ';':
        TokenEmiColon(c);
        break;
    case '(':
        TokenLParet(c);
        break;
    case ')':
        TokenRParet(c);
        break;
    case '[':
        TokenLSqbrckt(c);
        break;
    case ']':
        TokenRSqbrckt(c);
        break;
    case '{':
        TokenLCubrckt(c);
        break;
    case '}':
        TokenRCubrckt(c);
        break;
    default:
        Error::Fatal(c.Location() + "unknown punct: " + *c + c.Currline());
        break;
    }
}

void Tokenizer::TokenPlus(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_PLUSASSIGN;
        ++c;
        break;
    case '+':
        tok->operate = O_INC;
        ++c;
        break;
    default:
        tok->operate = O_PLUS;
        break;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenSub(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '>':
        tok->operate = O_ARROW;
        ++c;
        break;
    case '=':
        tok->operate = O_SUBASSIGN;
        ++c;
        break;
    case '-':
        tok->operate = O_DEC;
        ++c;
        break;
    default:
        tok->operate = O_SUB;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenMul(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_MULASSIGN;
        ++c;
        break;
    default:
        tok->operate = O_MUL;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenDiv(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_DIVASSIGN;
        ++c;
        break;
    default:
        tok->operate = O_DIV;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenComp(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_COMPASSIGN;
        ++c;
        break;
    default:
        tok->operate = O_COMP;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenNot(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_NOTEQUAL;
        ++c;
        break;
    default:
        tok->operate = O_NOT;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenLower(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_LOWEQUAL;
        ++c;
        break;
    case '<':
        if (*++c == '=') {
            tok->operate = O_SHLASSIGN;
            ++c;
        } else {
            tok->operate = O_SHL;
        }
        break;
    default:
        tok->operate = O_LOWER;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenGreater(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_GREAEQUAL;
        ++c;
        break;
    case '>':
        if (*++c == '=') {
            tok->operate = O_RHLASSIGN;
            ++c;
        } else {
            tok->operate = O_RHL;
        }
        break;
    default:
        tok->operate = O_GREATER;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenEqual(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_EQUAL;
        ++c;
        break;
    default:
        tok->operate = O_ASSIGN;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenAnd(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_ANDASSIGN;
        ++c;
        break;
    case '&':
        tok->operate = O_AND;
        ++c;
        break;
    default:
        tok->operate = O_BTIAND;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenOr(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_ORASSIGN;
        ++c;
        break;
    case '|':
        tok->operate = O_OR;
        ++c;
        break;
    default:
        tok->operate = O_BITOR;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenXor(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_XORASSIGN;
        ++c;
        break;
    default:
        tok->operate = O_BITXOR;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenNeg(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    switch (*(++c))
    {
    case '=':
        tok->operate = O_NEGASSIGN;
        ++c;
        break;
    default:
        tok->operate = O_BITNEG;
    }

    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenCond(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->operate = O_COND;
    tok->type = TK_OPEOR;
    tokenlist.emplace_back(tok);  
}

void Tokenizer::TokenComma(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_COMMA;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenDot(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_DOT;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenColon(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_COLON;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenEmiColon(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_EMICLON;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenLParet(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_LPARET;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenRParet(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_RPARET;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenLSqbrckt(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_LSQBRCKT;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenRSqbrckt(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_RSQBRCKT;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenLCubrckt(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_LCUBRCKT;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

void Tokenizer::TokenRCubrckt(Content& c)
{
    std::shared_ptr<Token> tok {std::make_shared<Token>()};

    ++c;
    tok->separator = S_RCUBRCKT;
    tok->type = TK_SEPOR;
    tokenlist.emplace_back(tok);
}

}