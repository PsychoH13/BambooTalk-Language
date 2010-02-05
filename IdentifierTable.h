/*
 * IdentifierTable.h
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

#include "Token.h"
//#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/OwningPtr.h"
//#include "llvm/Support/PointerLikeTypeTraits.h"
#include <string>
#include <cassert>

namespace BambooTalk
{
    class MultiKeywordSelector;
    
    class IdentifierInfo
    {
        unsigned  tokenID;
        void     *FETokenInfo;
        llvm::StringMapEntry<IdentifierInfo *> *entry;
        
        IdentifierInfo(const IdentifierInfo&);  // NONCOPYABLE.
        void operator=(const IdentifierInfo&);  // NONASSIGNABLE.
        
        friend class IdentifierTable;
        
    public:
        IdentifierInfo();
        
        tok::TokenKind getTokenID() const { return (tok::TokenKind)tokenID; }
        void setTokenID(tok::TokenKind tkid) { tokenID = tkid; }
    };
    
    
    class IdentifierTable
    {
    public:
        IdentifierTable();
        
        typedef llvm::StringMap<IdentifierInfo*, llvm::BumpPtrAllocator> HashTableTy;
        HashTableTy hashTable;
        
        llvm::BumpPtrAllocator& getAllocator() { return hashTable.getAllocator(); }
        
        void addKeywords();
        
        IdentifierInfo &get(const char *nameStart, const char *nameEnd)
        {
            llvm::StringMapEntry<IdentifierInfo*> &entry = hashTable.GetOrCreateValue(nameStart, nameEnd);
            
            IdentifierInfo *identInfo = entry.getValue();
            if (identInfo) return *identInfo;
            
            // No infomration available, make a new IdentifierInfo.
            void *mem = getAllocator().Allocate<IdentifierInfo>();
            identInfo = new (mem) IdentifierInfo();
            entry.setValue(identInfo);
            
            // Make sure getName() knows how to find the IdentifierInfo
            // contents.
            identInfo->entry = &entry;
            
            return *identInfo;
        }
        
        IdentifierInfo &createIdentifierInfo(const char *nameStart, const char *nameEnd)
        {
            llvm::StringMapEntry<IdentifierInfo*> &entry =
            hashTable.GetOrCreateValue(nameStart, nameEnd);
            
            IdentifierInfo *identInfo = entry.getValue();
            assert(!identInfo && "IdentifierInfo already exists");
            
            // Lookups failed, make a new IdentifierInfo.
            void *mem = getAllocator().Allocate<IdentifierInfo>();
            identInfo = new (mem) IdentifierInfo();
            entry.setValue(identInfo);
            
            // Make sure getName() knows how to find the IdentifierInfo
            // contents.
            identInfo->entry = &entry;
            
            return *identInfo;
        }
        /*
        IdentifierInfo &createIdentifierInfo(llvm::StringRef name)
        {
            return CreateIdentifierInfo(name.begin(), name.end());
        }
        
        IdentifierInfo &get(llvm::StringRef name)
        {
            return get(name.begin(), name.end());
        }
        */
        typedef HashTableTy::const_iterator iterator;
        typedef HashTableTy::const_iterator const_iterator;
        
        iterator begin() const { return hashTable.begin(); }
        iterator end  () const { return hashTable.end  (); }
        unsigned size () const { return hashTable.size (); }
        
        /// PrintStats - Print some statistics to stderr that indicate how well the
        /// hashing is doing.
        void PrintStats() const;
    };
    
    class Selector
    {
        friend class DiagnosticInfo;
        
        enum IdentifierInfoFlag
        {
            // MultiKeywordSelector = 0.
            UnarySelector  = 0x1,
            BinarySelector = 0x2,
            OneArgSelector = 0x4,
            ArgFlags = UnarySelector | BinarySelector | OneArgSelector
        };
        uintptr_t infoPtr; // a pointer to the MultiKeywordSelector or IdentifierInfo.
        
        Selector(IdentifierInfo *ii, unsigned nArgs)
        {
            infoPtr = reinterpret_cast<uintptr_t>(ii);
            assert((infoPtr & ArgFlags) == 0 && "Insufficiently aligned IdentifierInfo");
            assert(nArgs < 2 && "nArgs not equal to 0/1");
            infoPtr |= nArgs + 1;
        }
        Selector(MultiKeywordSelector *si)
        {
            infoPtr = reinterpret_cast<uintptr_t>(si);
            assert((infoPtr & ArgFlags) == 0 && "Insufficiently aligned IdentifierInfo");
        }
        
        IdentifierInfo *getAsIdentifierInfo() const
        {
            if (getIdentifierInfoFlag())
                return reinterpret_cast<IdentifierInfo *>(infoPtr & ~ArgFlags);
            return 0;
        }
        unsigned getIdentifierInfoFlag() const
        {
            return infoPtr & ArgFlags;
        }
        
    public:
        friend class SelectorTable; // only the SelectorTable can create these
        
        // FIXME: wtf ?
        //friend class DeclarationName; // and the AST's DeclarationName.
        
        /// The default ctor should only be used when creating data structures that
        ///  will contain selectors.
        Selector() : infoPtr(0) {}
        Selector(uintptr_t V) : infoPtr(V) {}
        
        /// operator==/!= - Indicate whether the specified selectors are identical.
        bool operator==(Selector RHS) const
        {
            return infoPtr == RHS.infoPtr;
        }
        bool operator!=(Selector RHS) const
        {
            return infoPtr != RHS.infoPtr;
        }
        void *getAsOpaquePtr() const
        {
            return reinterpret_cast<void*>(infoPtr);
        }
        
        /// \brief Determine whether this is the empty selector.
        bool isNull() const { return infoPtr == 0; }
        
        // Predicates to identify the selector type.
        bool isKeywordSelector() const
        {
            return (getIdentifierInfoFlag() != UnarySelector &&
                    getIdentifierInfoFlag() != BinarySelector);
        }
        bool isUnarySelector() const
        {
            return getIdentifierInfoFlag() == UnarySelector;
        }
        bool isBinarySelector() const
        {
            return getIdentifierInfoFlag() == UnarySelector;
        }
        unsigned getNumArgs() const;
        IdentifierInfo *getIdentifierInfoForSlot(unsigned argIndex) const;
        
        /// getAsString - Derive the full selector name (e.g. "foo:bar:") and return
        /// it as an std::string.
        std::string getAsString() const;
        
        static Selector getEmptyMarker()
        {
            return Selector(uintptr_t(-1));
        }
        static Selector getTombstoneMarker()
        {
            return Selector(uintptr_t(-2));
        }
    };
    
    /// SelectorTable - This table allows us to fully hide how we implement
    /// multi-keyword caching.
    class SelectorTable
    {
        void *impl;  // Actually a SelectorTableImpl
        SelectorTable(const SelectorTable&); // DISABLED: DO NOT IMPLEMENT
        void operator=(const SelectorTable&); // DISABLED: DO NOT IMPLEMENT
    public:
        SelectorTable();
        ~SelectorTable();
        
        /// getSelector - This can create any sort of selector.  NumArgs indicates
        /// whether this is a no argument selector "foo", a single argument selector
        /// "foo:" or multi-argument "foo:bar:".
        Selector getSelector(unsigned numArgs, IdentifierInfo **iiv);
        
        Selector getUnarySelector(IdentifierInfo *ID) {
            return Selector(ID, 1);
        }
        Selector getNullarySelector(IdentifierInfo *ID) {
            return Selector(ID, 0);
        }
        
        /*
        /// constructSetterName - Return the setter name for the given
        /// identifier, i.e. "set" + Name where the initial character of Name
        /// has been capitalized.
        static Selector constructSetterName(IdentifierTable &idents,
                                            SelectorTable &selTable,
                                            const IdentifierInfo *name)
        {
            llvm::SmallString<100> selectorName;
            selectorName = "set";
            selectorName += name->getName();
            selectorName[3] = toupper(selectorName[3]);
            IdentifierInfo *setterName =
            &idents.get(selectorName.data(), selectorName.data() + selectorName.size());
            return selTable.getUnarySelector(setterName);
        }
        */
    };
}
