/*
 * IdentifierTable.cpp
 * BambooTalk
 *
 * Created by Remy Demarest on 04/02/2010.
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

#include "IdentifierTable.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>

using namespace BambooTalk;

IdentifierInfo::IdentifierInfo()
{
    tokenID = tok::identifier;
    FETokenInfo = 0;
    entry = 0;
}

IdentifierTable::IdentifierTable() : hashTable(8192)
{
    addKeywords();
}

static void addKeyword(const char *kw, unsigned kwLen, tok::TokenKind tokenCode, IdentifierTable &table)
{
    IdentifierInfo &info = table.get(kw, kw + kwLen);
    info.setTokenID(tokenCode);
}

void IdentifierTable::addKeywords()
{
#define KEYWORD(NAME) addKeyword(#NAME, strlen(#NAME), tok::kw_ ## NAME, *this);
#include "TokenKinds.def"
}

namespace BambooTalk
{
    class MultiKeywordSelector : public llvm::FoldingSetNode
    {
        unsigned numArgs;
        
        MultiKeywordSelector(unsigned nKeys)
        {
            numArgs = nKeys;
        }
    public:
        // Constructor for keyword selectors.
        MultiKeywordSelector(unsigned nKeys, IdentifierInfo **IIV)
        {
            assert((nKeys > 1) && "not a multi-keyword selector");
            numArgs = nKeys;
            
            // Fill in the trailing keyword array.
            IdentifierInfo **KeyInfo = reinterpret_cast<IdentifierInfo **>(this+1);
            for (unsigned i = 0; i != nKeys; ++i)
                KeyInfo[i] = IIV[i];
        }
        
        // getName - Derive the full selector name and return it.
        std::string getName() const;
        
        unsigned getNumArgs() const { return numArgs; }
        
        typedef IdentifierInfo *const *keyword_iterator;
        keyword_iterator keyword_begin() const
        {
            return reinterpret_cast<keyword_iterator>(this+1);
        }
        keyword_iterator keyword_end() const
        {
            return keyword_begin()+getNumArgs();
        }
        IdentifierInfo *getIdentifierInfoForSlot(unsigned i) const
        {
            assert(i < getNumArgs() && "getIdentifierInfoForSlot(): illegal index");
            return keyword_begin()[i];
        }
        static void Profile(llvm::FoldingSetNodeID &ID, keyword_iterator ArgTys, unsigned NumArgs)
        {
            ID.AddInteger(NumArgs);
            for (unsigned i = 0; i != NumArgs; ++i)
                ID.AddPointer(ArgTys[i]);
        }
        void Profile(llvm::FoldingSetNodeID &ID) 
        {
            Profile(ID, keyword_begin(), getNumArgs());
        }
    };
}