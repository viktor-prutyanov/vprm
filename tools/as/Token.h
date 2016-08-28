enum class TokenType
{
    NoType,
    Instr,
    Reg,
    ToLabel,
    FromLabel,
    Num,
    Directive
};

enum class Instruction
{
    NoInstruction,
#define DEF_INSTR(name, str) \
    name,
#include "def_instr.h"
#undef DEF_INSTR
    NotUsed
};

class Token
{
    public:
    Token(const char *str, size_t len, TokenType type, Instruction instr, uint32_t num);
    ~Token();

    const char *Str;
    size_t Len;
    TokenType Type;
    Instruction Instr;
    uint32_t Num;
};

Token::Token(const char *str, size_t len, TokenType type, Instruction instr, uint32_t num)
    :Str (str), Len (len), Type (type), Instr (instr),  Num (num) {}

Token::~Token() {}
