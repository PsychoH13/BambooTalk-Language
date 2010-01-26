/*
 Copyright (c) 2010, Remy Demarest
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY Remy Demarest ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BAMBOOTALK_TOKEN_H
#define BAMBOOTALK_TOKEN_H

#include <string>

namespace BambooTalk
{
    namespace tok
    {
        enum TokenKind
        {
#define TOK(X) X,
#include "TokenKinds.def"
            TOKEN_COUNT
        };
        const char *getTokenName(enum TokenKind kind);
    }
    
    class Token
    {
        tok::TokenKind kind;
        
        std::string tokenChars;
        
    public:
        tok::TokenKind getKind() const { return kind; }
        void setKind(tok::TokenKind k) { kind = k; }
        
        std::string getTokenChars() const { return tokenChars; }
        void setTokenChars(std::string value) { tokenChars = value; }
        
        /// is/isNot - Predicates to check if this token is a specific kind, as in
        /// "if (Tok.is(tok::l_brace)) {...}".
        bool is(tok::TokenKind k) const { return kind == (unsigned) k; }
        bool isNot(tok::TokenKind k) const { return kind != (unsigned) k; }
        
        /// isLiteral - Return true if this is a "literal", like a numeric
        /// constant, string, etc.
        bool isLiteral() const {
            return is(tok::numeric_constant) || is(tok::char_constant) ||
            is(tok::string_literal);
        }
        
        void startToken(void)
        {
            kind = tok::unknown;
            tokenChars = "";
        }
    };
}

#endif