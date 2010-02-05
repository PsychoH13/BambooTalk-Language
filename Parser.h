/*
 * Parser.h
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


#ifndef BAMBOOTALK_PARSER_H
#define BAMBOOTALK_PARSER_H

#include "Lexer.h"
#include "Token.h"

namespace BambooTalk
{
    class Parser
    {
    private:
        Lexer &lexer;
        
        /// Tok - The current token we are peeking ahead.  All parsing methods assume
        /// that this is valid.
        Token tok;
        
        unsigned short parenCount, bracketCount, braceCount;
        
    public:
        Parser(Lexer &aLexer) : lexer(aLexer)
        {
            parenCount   = 0;
            bracketCount = 0;
            braceCount   = 0;
        }
        
        void consumeToken()
        {
            
        }
    };
}

#endif
