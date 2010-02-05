/*
 * Lexer.h
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

#ifndef BAMBOOTALK_LEXER_H
#define BAMBOOTALK_LEXER_H

#include "llvm/Support/MemoryBuffer.h"
#include "Token.h"
#include "IdentifierTable.h"
#include <string>
#include <cassert>
#include <iostream>

namespace BambooTalk
{
    class Lexer
    {
    private:
        const char *bufferStart;       // Start of the buffer.
        const char *bufferEnd;         // End of the buffer.
        const char *bufferPtr;
        
        Lexer(const Lexer&);          // DO NOT IMPLEMENT
        void operator=(const Lexer&); // DO NOT IMPLEMENT
        
        void initLexer(const char *bufStart, const char *bufPtr, const char *bufEnd);
        
        void lexTokenInternal(Token &result);
        
        void lexNumericConstant(Token &result, const char *curPtr);
        void lexIdentifier     (Token &result, const char *curPtr);
        void lexBinaryMessage  (Token &result, const char *curPtr);
        void lexCharConstant   (Token &result, const char *curPtr);
        void lexStringLiteral  (Token &result, const char *curPtr);
        
        void skipCommentLine   (Token &result, const char *curPtr);
        void skipCommentBlock  (Token &result, const char *curPtr);
        
        IdentifierTable identifiers;
        
    public:
        Lexer(llvm::MemoryBuffer *inputFile) : identifiers()
        {
            initLexer(inputFile->getBufferStart(), inputFile->getBufferStart(), inputFile->getBufferEnd());
        }
        
        void lex(Token &result)
        {
            result.startToken();
            
            lexTokenInternal(result);
        }
        
        IdentifierInfo *getIdentifierInfo(const char *nameStart, const char *nameEnd)
        {
            return &identifiers.get(nameStart, nameEnd);
        }
        
        IdentifierInfo *lookUpIdentifierInfo(Token &identifier, const char *bufPtr);
        
        void formTokenWithChars(Token &result, const char *tokEnd, tok::TokenKind kind)
        {
            unsigned tokLen = tokEnd - bufferPtr;
            std::string str = std::string(bufferPtr, tokLen);
            result.setTokenChars(str);
            result.setKind(kind);
            bufferPtr = tokEnd;
        }
    };
}

#endif