/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_SYM_READER_INCLUDED
#define CM_SYM_READER_INCLUDED
#include <Cm.Sym/TypeId.hpp>
#include <Cm.Ser/BinaryReader.hpp>
#include <Cm.Ast/TypeExpr.hpp>
#include <Cm.Ast/Reader.hpp>
#include <Cm.Parsing/Scanner.hpp>
#include <unordered_map>
#include <unordered_set>
#include <forward_list>

namespace Cm { namespace Sym {

using Cm::Ser::BinaryReader;
using Cm::Parsing::Span;
class Symbol;
class ClassTypeSymbol;
class SymbolTable;
class TypeSymbol;
class Value;
enum class ValueType : uint8_t;
enum class SymbolType : uint8_t;

class Reader
{
public:
    Reader(const std::string& fileName, SymbolTable& symbolTable_);
    BinaryReader& GetBinaryReader() { return binaryReader; }
    Cm::Ast::Reader& GetAstReader() { return astReader; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
    Symbol* ReadSymbol();
    SymbolType ReadSymbolType();
    Span ReadSpan();
    TypeId ReadTypeId();
    Cm::Ast::Derivation ReadDerivation();
    Cm::Ast::DerivationList ReadDerivationList();
    ValueType ReadValueType();
    Value* ReadValue();
    void FetchTypeFor(Symbol* symbol, int index);
    void BackpatchType(TypeSymbol* type);
    void SetSpanFileIndexOffset(int spanFileIndexOffset_);
    void MarkSymbolsBound();
    void MarkTemplateTypeSymbolsBound();
    void MarkSymbolsProject();
    bool AllTypesFetched();
    void EnqueueMakeIrTypeFor(Symbol* symbol);
    void MakeIrTypes();
    void InitVTablesAndITables();
    void SetFetchCidForVirtualClasses() { fetchCidForVirtualClasses = true; }
private:
    BinaryReader binaryReader;
    SymbolTable& symbolTable;
    Cm::Ast::Reader astReader;
    typedef std::forward_list<std::pair<Symbol*, int>> FetchTypeList;
    typedef std::unordered_map<TypeId, FetchTypeList, TypeIdHash> FetchTypeMap;
    typedef FetchTypeMap::iterator FetchTypeMapIt;
    FetchTypeMap fetchTypeMap;
    int spanFileIndexOffset;
    bool markSymbolsBound;
    bool markSymbolsProject;
    bool markTemplateTypeSymbolsBound;
    bool fetchCidForVirtualClasses;
    std::unordered_set<Symbol*> makeIrTypeSet;
    std::unordered_set<ClassTypeSymbol*> initVTableAndITableSet;
};

} } // namespace Cm::Sym

#endif // CM_SYM_READER_INCLUDED
