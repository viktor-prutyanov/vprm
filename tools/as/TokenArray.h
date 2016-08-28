#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "Token.h"

#define SKIP_BAD_CHAR               \
    while ((*ptr == 0xD || *ptr == 0xA || *ptr == ' ' || *ptr == ';' || is_comment) && ptr != asmText + asmFileLen) \
    {                               \
        if (*ptr == ';')            \
            is_comment = true;      \
        else if (*ptr == '\n')      \
            is_comment = false;     \
        ++ptr;                      \
    }

using std::pair;
using std::vector;

class TokenArray
{
public:
    TokenArray(FILE *asmFile, size_t asmFileLen);
    ~TokenArray();
    void Dump();
    size_t Size();
    bool Make();
    bool ResolveLabels();
    size_t MakeBin(FILE *binFile); 
    size_t MakeHex(FILE *hexFile); //Must be called before MakeBin

private:
    Token *getNextToken(const char *&ptr, const Token *prevToken);
	Instruction matchInstruction(const char *ptr, size_t len);

    vector<Token *> tokens;
    char *asmText;
    size_t asmFileLen;
	size_t instrCount;
    size_t numCount;  
    uint16_t *binData;
    vector<pair<Token *, uint16_t>> labels;
};

TokenArray::TokenArray(FILE *asmFile, size_t asmFileLen)
   :tokens (vector<Token *>()), 
    asmText ((char *)calloc(asmFileLen, sizeof(char))),
    asmFileLen (asmFileLen),
    instrCount (0),
    numCount (0),
    binData (nullptr),
    labels (vector<pair<Token *, uint16_t>>())
{
    fread(asmText, sizeof(char), asmFileLen, asmFile);
    const char *ptr = asmText;
    Token *prevInstrToken = nullptr;
    Token *curToken = nullptr;
    while (ptr != asmText + asmFileLen)
    {
        curToken = getNextToken(ptr, prevInstrToken);
        tokens.push_back(curToken);
        if (curToken->Type == TokenType::Instr)
        {
            prevInstrToken = curToken;
        }
    }

    binData = (uint16_t *)calloc(instrCount + numCount, sizeof(uint16_t));
}

TokenArray::~TokenArray() 
{
    free(binData);
    free(asmText);
    std::for_each(tokens.begin(), tokens.end(), [](const Token *token)
    {
        delete token;
    });
}

size_t TokenArray::Size()
{
    return tokens.size();
}

void TokenArray::Dump()
{
    printf("tokens at %p, size = %lu\n", this, tokens.size());
    printf("\t   i  Len               Str Instr        Num Type\n");
    size_t i = 0;
    std::for_each(tokens.begin(), tokens.end(), [&i](const Token *token) 
    {
        printf("\t[%3lu] %3lu {%16.*s}  %3d 0x%8X ", i, token->Len, token->Len, token->Str, (int)token->Instr, token->Num);
        switch (token->Type)
        {
        case TokenType::NoType:
            printf("NoType\n");
            break;
        case TokenType::Instr:
            printf("Instr\n");
            break;
        case TokenType::FromLabel:
            printf("FromLabel\n");
            break;
        case TokenType::ToLabel:
            printf("ToLabel\n");
            break;
        case TokenType::Reg:
            printf("Reg\n");
            break;
        case TokenType::Num:
            printf("Num\n");
            break;
        case TokenType::Directive:
            printf("Directive\n");
            break;
        default:
            printf("INVALID TYPE\n");
            break;
        }
        ++i;
    });
    i = 0;
    printf("labels at %p, size = %lu\n", this, labels.size());
    printf("\t   i  Len               Str LblTo\n");
    std::for_each(labels.begin(), labels.end(), [&i](const pair<Token *, uint16_t> label)
    {
        printf("\t[%3lu] %3lu {%16.*s}    %d\n", i, label.first->Len, label.first->Len, label.first->Str, label.second);
        ++i;
    });
}

bool TokenArray::ResolveLabels()
{
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens.at(i)->Type == TokenType::FromLabel)
        {
            for (size_t j = 0; j < labels.size(); j++)
            {
                if ((labels.at(j).first->Len == tokens.at(i)->Len) && 
                    (memcmp(labels.at(j).first->Str + 1, tokens.at(i)->Str + 1, tokens.at(i)->Len - 1) == 0))
                {
                    tokens.at(i)->Num = labels.at(j).second;
                    break;
                }
                else if (j == labels.size() - 1)
                {
                    printf("ERROR: Label error: {%16.*s} FromLabel token has no pair\n", tokens.at(i)->Len, tokens.at(i)->Str);
                    return false;
                }
            }
        }
    }
    return true;
}

Token *TokenArray::getNextToken(const char *&ptr, const Token *prevInstrToken)
{
    bool is_comment = false;
    SKIP_BAD_CHAR;
    const char *start_ptr = ptr;
    while (*ptr != 0x0D && *ptr != 0x0A && *ptr != ' ' && ptr != asmText + asmFileLen)
    {
        ++ptr;
    }
    const char *end_ptr = ptr;
    SKIP_BAD_CHAR;
    Token *token = new Token(start_ptr, end_ptr - start_ptr, TokenType::NoType, Instruction::NoInstruction, 0);
    if (isdigit(*start_ptr) || (('A' <= *start_ptr) && (*start_ptr <= 'F')))
	{
		token->Type = TokenType::Num;
        uint32_t multiplifier = 1;
        for (int64_t i = end_ptr - start_ptr - 1; i >= 0; --i)
        {
            token->Num += (isdigit(start_ptr[i]) ? start_ptr[i] - '0' : start_ptr[i] - 'A' + 10) * multiplifier;
            multiplifier *= 16;
        }
	}
    else if (*start_ptr == '$')
    {
        if (false)
            ;
#define DEF_REG(name, str, num, type)                                                                       \
        else if ((end_ptr - start_ptr == sizeof(str)) && (memcmp(start_ptr + 1, str, sizeof(str) - 1) == 0))\
        {                                                                                                   \
            token->Num = num;                                                                               \
            token->Type = type;                                                                             \
        }
#include "def_reg.h"
#undef DEF_REG      
        else
            token->Num = UINT32_MAX;
    }
    else if (isalpha(*start_ptr))
	{
		token->Type = TokenType::Instr;
		token->Instr = matchInstruction(start_ptr, end_ptr - start_ptr);
		++instrCount;
	}
	else if (*start_ptr == '_')
		token->Type = TokenType::FromLabel;
	else if (*start_ptr == ':')
	{
		token->Type = TokenType::ToLabel;
        labels.push_back(pair<Token *, uint16_t>(token, instrCount + numCount));
	}
    else if (*start_ptr == '.')
        token->Type = TokenType::Directive;

    return token;
}

Instruction TokenArray::matchInstruction(const char *ptr, size_t len)
{

	if (false)
        ;
#define DEF_INSTR(instr, str)														\
	else if ((len == sizeof(str) - 1) && (memcmp(ptr, str, sizeof(str) - 1) == 0))	\
		return Instruction::instr;
#include "def_instr.h"
#undef DEF_INSTR
	else
		return Instruction::NoInstruction;
}


bool TokenArray::Make()
{
    uint16_t *binDataPos = binData;
    size_t tokensPos = 0;
    while (tokensPos < tokens.size())
    {
        
#define INSTR
#include "instructions.h"
        switch (tokens.at(tokensPos)->Type)
        {
        case TokenType::Instr:
            switch (tokens.at(tokensPos)->Instr)
        	{
		    case Instruction::Shl:  GN(SHL);   break;     
            case Instruction::Shr:  GN(SHR);   break;
            case Instruction::Ror:  GN(ROR);   break;
            case Instruction::Xbn:  GN(XBN);   break;
            case Instruction::Jr:   G1(JR);    break; 
            case Instruction::Jalr: G1(JALR);  break;
            case Instruction::Stl:  G2(STL);   break;
            case Instruction::Sth:  G2(STH);   break;
            case Instruction::Stw:  G2(STW);   break;
            case Instruction::Cmp:  G2(CMP);   break;
            case Instruction::Ldl:  G2(LDL);   break;
            case Instruction::Ldh:  G2(LDH);   break;
            case Instruction::Ldw:  G2(LDW);   break;
            case Instruction::Out:  G2(OUT);   break;
            case Instruction::Mul:  G2(MUL);   break;
            case Instruction::And:  G2(AND);   break;
            case Instruction::Or:   G2(OR);     break;
            case Instruction::Xor:  G2(XOR);   break;
            case Instruction::Bic:  G2(BIC);   break;
            case Instruction::Not:  G2(NOT);   break;
            case Instruction::Sh:   G2(SH);    break;
            case Instruction::In:   G2(IN);    break;
            case Instruction::Add:  G3(ADD);   break;
            case Instruction::Sub:  G3(SUB);   break;
            case Instruction::Movb: MB(MOVB);  break;
            case Instruction::Addb: GN(ADDB);  break;
            case Instruction::Subb: GN(SUBB);  break;
            case Instruction::B:    AD(B);     break;
            case Instruction::Bal:  AD(BAL);   break;
            case Instruction::Bz:   AD(BZ);    break;
            case Instruction::Bnz:  AD(BNZ);   break;
            case Instruction::Blz:  AD(BLZ);   break;
            case Instruction::Bgez: AD(BGEZ);  break;

            default:
                printf("ERROR: Invalid instruction: {%8.*s}\n", tokens.at(tokensPos)->Len, tokens.at(tokensPos)->Str);
                return false;
                break;
        	}
            ++binDataPos;
        	break;
        case TokenType::ToLabel:
        	break;
        case TokenType::Directive:
            if ((tokens.at(tokensPos)->Len == 5) && (memcmp(tokens.at(tokensPos)->Str + 1, "half", 4) == 0))
            {
                if ((tokens.at(tokensPos + 1)->Type == TokenType::Num) ||
                    (tokens.at(tokensPos + 1)->Type == TokenType::FromLabel))
                {
                    *binDataPos = (uint16_t)(tokens.at(tokensPos + 1)->Num);
                    ++numCount;
                }
                else
                {
                    printf("ERROR: Invalid .vect directive argument TokenType\n");
                    return false;
                }
                ++tokensPos;
            }
            else
            {
                printf("ERROR: Invalid directive\n");
                return false;
            }
            ++binDataPos;
            break;
        default:
            printf("ERROR: TokenType mismatch: {%8.*s}\n", tokens.at(tokensPos)->Len, tokens.at(tokensPos)->Str);
        	return false;
        	break;
        } 
        ++tokensPos;
#undef INSTR
    }
    
	return true;
}

size_t TokenArray::MakeBin(FILE *binFile)
{
    for (size_t i = 0; i < instrCount + numCount; ++i)
    {
        binData[i] = (((binData[i] & 0x00FF) << 8) + ((binData[i] & 0xFF00) >> 8));
    }

    return (sizeof(uint16_t) * fwrite(binData, sizeof(uint16_t), instrCount + numCount, binFile));
}

size_t TokenArray::MakeHex(FILE *hexFile)
{
    size_t hexFileSize = 0;
    uint8_t checksum = 0x00;
    for (size_t addr = 0; addr < instrCount + numCount; ++addr)
    {
        checksum = 0x100 - (0x02 + ((addr & 0x00FF) + ((addr & 0xFF00) >> 8)) + ((binData[addr] & 0x00FF) + ((binData[addr] & 0xFF00) >> 8)));
        hexFileSize += fprintf(hexFile, ":02%04X00%04X%02X\n", addr, binData[addr], checksum);
    }
    hexFileSize += fprintf(hexFile, ":00000001FF");

    return hexFileSize;
}
