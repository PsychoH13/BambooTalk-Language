/*
 * Lexer.cpp
 * BambooTalk
 *
 * Created by Remy Demarest on 28/01/2010.
 * Copyright (c) 2010, Remy Demarest
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the OpenEmu Team nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY Remy Demarest ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Lexer.h"

using namespace BambooTalk;

enum {
    CHAR_HORZ_WS  = 0x01,  // ' ', '\t', '\f', '\v'.  Note, no '\0'
    CHAR_VERT_WS  = 0x02,  // '\r', '\n'
    CHAR_LETTER   = 0x04,  // a-z,A-Z
    CHAR_NUMBER   = 0x08,  // 0-9
    CHAR_UNDER    = 0x10,  // _
    CHAR_PERIOD   = 0x20,  // .
    CHAR_BIN_MES  = 0x40   // ~ ! @ % & * - + = | \ < > , ? / ^ `
};

// Statically initialize CharInfo table based on ASCII character set
// Reference: FreeBSD 7.2 /usr/share/misc/ascii
static const unsigned char CharInfo[256] =
{
    //  0 NUL        1 SOH         2 STX         3 ETX          4 EOT         5 ENQ         6 ACK         7 BEL
    0           , 0           , 0           , 0           , 0           , 0           , 0           , 0           ,
    //  8 BS         9 HT         10 NL         11 VT          12 NP         13 CR         14 SO         15 SI
    0           , CHAR_HORZ_WS, CHAR_VERT_WS, CHAR_HORZ_WS, CHAR_HORZ_WS, CHAR_VERT_WS, 0           , 0           ,
    // 16 DLE       17 DC1        18 DC2        19 DC3         20 DC4        21 NAK        22 SYN        23 ETB
    0           , 0           , 0           , 0           , 0           , 0           , 0           , 0           ,
    // 24 CAN       25 EM         26 SUB        27 ESC         28 FS         29 GS         30 RS         31 US
    0           , 0           , 0           , 0           , 0           , 0           , 0           , 0           ,
    // 32 SP        33  !         34  "         35  #          36  $         37  %         38  &         39  '
    CHAR_HORZ_WS, CHAR_BIN_MES, 0           , 0           , 0           , CHAR_BIN_MES, CHAR_BIN_MES, 0           ,
    // 40  (        41  )         42  *         43  +          44  ,         45  -         46  .         47  /
    0           , 0           , CHAR_BIN_MES, CHAR_BIN_MES, CHAR_BIN_MES, CHAR_BIN_MES, CHAR_PERIOD , CHAR_BIN_MES,
    // 48  0        49  1         50  2         51  3          52  4         53  5         54  6         55  7
    CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER , CHAR_NUMBER ,
    // 56  8        57  9         58  :         59  ;          60  <         61  =         62  >         63  ?
    CHAR_NUMBER , CHAR_NUMBER , 0           , 0           , CHAR_BIN_MES, CHAR_BIN_MES, CHAR_BIN_MES, CHAR_BIN_MES,
    // 64  @        65  A         66  B         67  C          68  D         69  E         70  F         71  G
    CHAR_BIN_MES, CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    // 72  H        73  I         74  J         75  K          76  L         77  M         78  N         79  O
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    // 80  P        81  Q         82  R         83  S          84  T         85  U         86  V         87  W
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    // 88  X        89  Y         90  Z         91  [          92  \         93  ]         94  ^         95  _
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , 0           , CHAR_BIN_MES, 0           , CHAR_BIN_MES, CHAR_UNDER  ,
    // 96  `        97  a         98  b         99  c         100  d       101  e        102  f        103  g
    CHAR_BIN_MES, CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //104  h       105  i        106  j        107  k         108  l       109  m        110  n        111  o
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //112  p       113  q        114  r        115  s         116  t       117  u        118  v        119  w
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , CHAR_LETTER ,
    //120  x       121  y        122  z        123  {         124  |       125  }        126  ~        127 DEL
    CHAR_LETTER , CHAR_LETTER , CHAR_LETTER , 0           , CHAR_BIN_MES, 0           , CHAR_BIN_MES, 0
};

static void initCharacterInfo() {
    static bool isInited = false;
    if (isInited) return;
    // check the statically-initialized CharInfo table
    assert(CHAR_HORZ_WS == CharInfo[(int)' ']);
    assert(CHAR_HORZ_WS == CharInfo[(int)'\t']);
    assert(CHAR_HORZ_WS == CharInfo[(int)'\f']);
    assert(CHAR_HORZ_WS == CharInfo[(int)'\v']);
    assert(CHAR_VERT_WS == CharInfo[(int)'\n']);
    assert(CHAR_VERT_WS == CharInfo[(int)'\r']);
    assert(CHAR_UNDER   == CharInfo[(int)'_']);
    assert(CHAR_PERIOD  == CharInfo[(int)'.']);
    for (unsigned i = 'a'; i <= 'z'; ++i) {
        assert(CHAR_LETTER == CharInfo[i]);
        assert(CHAR_LETTER == CharInfo[i + 'A' - 'a']);
    }
    for (unsigned i = '0'; i <= '9'; ++i)
        assert(CHAR_NUMBER == CharInfo[i]);
    
    assert(CHAR_BIN_MES == CharInfo[(int)'~']);
    assert(CHAR_BIN_MES == CharInfo[(int)'!']);
    assert(CHAR_BIN_MES == CharInfo[(int)'@']);
    assert(CHAR_BIN_MES == CharInfo[(int)'%']);
    assert(CHAR_BIN_MES == CharInfo[(int)'&']);
    assert(CHAR_BIN_MES == CharInfo[(int)'*']);
    assert(CHAR_BIN_MES == CharInfo[(int)'-']);
    assert(CHAR_BIN_MES == CharInfo[(int)'+']);
    assert(CHAR_BIN_MES == CharInfo[(int)'=']);
    assert(CHAR_BIN_MES == CharInfo[(int)'|']);
    assert(CHAR_BIN_MES == CharInfo[(int)'\\']);
    assert(CHAR_BIN_MES == CharInfo[(int)'<']);
    assert(CHAR_BIN_MES == CharInfo[(int)'>']);
    assert(CHAR_BIN_MES == CharInfo[(int)',']);
    assert(CHAR_BIN_MES == CharInfo[(int)'?']);
    assert(CHAR_BIN_MES == CharInfo[(int)'/']);
    assert(CHAR_BIN_MES == CharInfo[(int)'^']);
    assert(CHAR_BIN_MES == CharInfo[(int)'`']);
    
    isInited = true;
}

/// isIdentifierBody - Return true if this is the body character of an
/// identifier, which is [a-zA-Z0-9_].
static inline bool isIdentifierBody(unsigned char c) {
    return (CharInfo[c] & (CHAR_LETTER | CHAR_NUMBER | CHAR_UNDER)) ? true : false;
}

/// isHorizontalWhitespace - Return true if this character is horizontal
/// whitespace: ' ', '\t', '\f', '\v'.  Note that this returns false for '\0'.
static inline bool isHorizontalWhitespace(unsigned char c) {
    return (CharInfo[c] & CHAR_HORZ_WS) ? true : false;
}

/// isWhitespace - Return true if this character is horizontal or vertical
/// whitespace: ' ', '\t', '\f', '\v', '\n', '\r'.  Note that this returns false
/// for '\0'.
static inline bool isWhitespace(unsigned char c) {
    return (CharInfo[c] & (CHAR_HORZ_WS | CHAR_VERT_WS)) ? true : false;
}

/// isNumberBody - Return true if this is the body character of a
/// preprocessing number, which is [a-zA-Z0-9_.].
static inline bool isNumberBody(unsigned char c) {
    return (CharInfo[c] & (CHAR_LETTER | CHAR_NUMBER | CHAR_PERIOD)) ? true : false;
}

/// isBinaryMessageBody - Return true if this is the body character of a
/// preprocessing binary message, which is [~!@%&*-+=|\<>,?/^`].
static inline bool isBinaryMessageBody(unsigned char c) {
    return (CharInfo[c] & CHAR_BIN_MES) ? true : false;
}

void Lexer::initLexer(const char *bufStart, const char *bufPtr, const char *bufEnd)
{
    initCharacterInfo();
    bufferStart = bufStart;
    bufferEnd   = bufEnd;
    bufferPtr   = bufPtr;
}

void Lexer::lexNumericConstant(Token &result, const char *curPtr)
{
    char prev;
    char c = *curPtr;
    bool hasPeriod = false;
    
    while(isNumberBody(c))
    {
        if(c == '.') hasPeriod = true;
        prev = c;
        c = *curPtr++;
    }
    
    if(prev == '.') curPtr--;
    
    curPtr--;
    
    formTokenWithChars(result, curPtr, tok::numeric_constant);
}

IdentifierInfo *Lexer::lookUpIdentifierInfo(Token &ident, const char *bufPtr)
{
    assert(ident.is(tok::identifier) && "Not an identifier!");
    assert(ident.getIdentifierInfo() == 0 && "Identinfo already exists!");
    
    IdentifierInfo *identInfo = getIdentifierInfo(bufPtr, bufPtr + ident.getLength());
    
    ident.setIdentifierInfo(identInfo);
    
    return identInfo;
}

void Lexer::lexIdentifier(Token &result, const char *curPtr)
{
    char prev;
    char c = *curPtr++;
    
    while(isIdentifierBody(c))
    {
        prev = c;
        c = *curPtr++;
    }
    
    tok::TokenKind k = tok::identifier;
    
    curPtr--;
    
    const char *idStart = bufferPtr;
    formTokenWithChars(result, curPtr, k);
    
    IdentifierInfo *identInfo = lookUpIdentifierInfo(result, idStart);
    
    result.setKind(identInfo->getTokenID());
}

void Lexer::lexBinaryMessage(Token &result, const char *curPtr)
{
    char prev;
    char c = *curPtr++;
    
    while(isBinaryMessageBody(c))
    {
        prev = c;
        c = *curPtr++;
    }
    
    curPtr--;
    
    formTokenWithChars(result, curPtr, tok::binary_message);
}

void Lexer::lexCharConstant(Token &result, const char *curPtr)
{
    char c = *curPtr++;
    if(c == '\'')
    {
        formTokenWithChars(result, curPtr, tok::unknown);
        return;
    }
    else if(c == '\\')
    {
        c = *curPtr++;
    }
    
    if(c && c != '\n' && c != '\r' && curPtr[0] == '\'')
        curPtr++;
    else
    {
        // Fall back on generic code for embedded nulls, newlines, wide chars.
        do
        {
            // Skip escaped characters.
            if(c == '\\')
            {
                // Skip the escaped character.
                c = *curPtr++;
            }
            else if(c == '\n' || c == '\r' ||            // Newline.
                    (c == 0 && curPtr - 1 == bufferEnd)) // End of file.
            {
                formTokenWithChars(result, curPtr - 1, tok::unknown);
                return;
            }
            c = *curPtr++;
        } while(c != '\'');
    }
    
    formTokenWithChars(result, curPtr, tok::char_constant);
}

void Lexer::lexStringLiteral(Token &result, const char *curPtr)
{
    char c = *curPtr++;
    while(c != '"')
    {
        // Skip escaped characters.
        if(c == '\\')
        {
            // Skip the escaped character.
            c = *curPtr++;
        }
        else if(c == '\n' || c == '\r' ||            // Newline.
                (c == 0 && curPtr - 1 == bufferEnd)) // End of file.
        {
            formTokenWithChars(result, curPtr - 1, tok::unknown);
            return;
        }
        c = *curPtr++;
    }
    
    formTokenWithChars(result, curPtr, tok::string_literal);
}

void Lexer::skipCommentLine(Token &result, const char *curPtr)
{
    char c = *curPtr++;
    while(c != '\n' && c != '\r' && c != '\0') c = *curPtr++;
    
    ++curPtr;
    
    bufferPtr = curPtr;
    
    std::cerr << "Was line comment" << std::endl;
}

void Lexer::skipCommentBlock(Token &result, const char *curPtr)
{
    unsigned nested = 1;
    char c = *curPtr++;
    while(1)
    {
        if(c == ':' && curPtr[0] == '#')
        {
            curPtr++;
            c = *curPtr++;
            nested--;
            
            if(nested == 0) break;
        }
        
        if(c == '#' && curPtr[0] == ':')
        {
            curPtr++;
            c = *curPtr++;
            nested++;
        }
        
        c = *curPtr++;
    }
    
    bufferPtr = curPtr;
    
    std::cerr << "Was block comment" << std::endl;
}

void Lexer::lexTokenInternal(Token &result)
{
LexNextToken:
    const char *currentPtr = bufferPtr;
    
    if((*currentPtr == ' ') || (*currentPtr == '\t'))
    {
        ++currentPtr;
        while((*currentPtr == ' ') || (*currentPtr == '\t'))
            ++currentPtr;
        
        bufferPtr = currentPtr;
    }
    
    char theChar = *currentPtr++;
    
    tok::TokenKind kind = tok::unknown;
    
    switch(theChar)
    {
        case 0 :
            result.setKind(tok::eof);
            return;
            break;
        case '\n':
        case '\r':
            bufferPtr++;
            goto LexNextToken;
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return lexNumericConstant(result, currentPtr - 1);
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case '_':
            return lexIdentifier(result, currentPtr);
            
#define CHECK_BINARY_MESSAGE(tokkind) if(!isBinaryMessageBody(*currentPtr)) { kind = tokkind; break; }
        case ',': CHECK_BINARY_MESSAGE(tok::comma);
        case '|': CHECK_BINARY_MESSAGE(tok::bar);
        case '+': CHECK_BINARY_MESSAGE(tok::plus);
        case '-': CHECK_BINARY_MESSAGE(tok::minus);
        case '*': CHECK_BINARY_MESSAGE(tok::star);
        case '@': CHECK_BINARY_MESSAGE(tok::at);
        case '<': CHECK_BINARY_MESSAGE(tok::l_angle);
        case '>': CHECK_BINARY_MESSAGE(tok::r_angle);
#undef CHECK_BINARY_MESSAGE
        case '~': case '!': case '%': case '&': case '=':
        case '?': case '/': case '^': case '`': case '\\':
            return lexBinaryMessage(result, currentPtr);
        case '"':
            return lexStringLiteral(result, currentPtr);
        case '\'':
            return lexCharConstant(result, currentPtr);
        case '#':
            // Comment line
            if(*currentPtr == ':')
            {
                skipCommentBlock(result, currentPtr + 1);
                goto LexNextToken;
            }
            else if(*currentPtr == '#')
            {
                skipCommentLine(result, currentPtr + 1);
                goto LexNextToken;
            }
            else kind = tok::hash;
            break;
        case ':':
            // Assignment operator
            if(*currentPtr == '=')
            {
                currentPtr++;
                kind = tok::colonequal;
            }
            else kind = tok::colon;
            break;
        case '[': kind = tok::l_square; break;
        case ']': kind = tok::r_square; break;
        case '(': kind = tok::l_paren;  break;
        case ')': kind = tok::r_paren;  break;
        case '{': kind = tok::l_brace;  break;
        case '}': kind = tok::r_brace;  break;
        case '.': kind = tok::period;   break;
        case ';': kind = tok::semi;     break;
        case '$': kind = tok::dollar;   break;
    }
    
    formTokenWithChars(result, currentPtr, kind);
}
