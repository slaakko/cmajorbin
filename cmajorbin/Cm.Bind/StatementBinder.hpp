/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_BIND_STATEMENT_INCLUDED
#define CM_BIND_STATEMENT_INCLUDED
#include <Cm.Bind/ExpressionBinder.hpp>
#include <Cm.BoundTree/BoundStatement.hpp>

namespace Cm { namespace Bind {

class StatementBinder : public ExpressionBinder
{
public:
    StatementBinder(Cm::Sym::SymbolTable& symbolTable_, Cm::Sym::ConversionTable& conversionTable_, Cm::Core::ClassConversionTable& classConversionTable_,
        Cm::Sym::ContainerScope* containerScope_, Cm::Sym::FileScope* fileScope_, Cm::BoundTree::BoundFunction* currentFunction_);
    Cm::Sym::SymbolTable& SymbolTable() { return symbolTable; }
    Cm::Sym::ContainerScope* ContainerScope() const { return containerScope; }
    Cm::Sym::FileScope* FileScope() const { return fileScope; }
    void SetResult(Cm::BoundTree::BoundStatement* result_) { result = result_; }
    Cm::BoundTree::BoundStatement* Result() const { return result; }
private:
    Cm::Sym::SymbolTable& symbolTable;
    Cm::Sym::ContainerScope* containerScope;
    Cm::Sym::FileScope* fileScope;
    Cm::BoundTree::BoundStatement* result;
};

class ConstructionStatementBinder : public StatementBinder
{
public:
    ConstructionStatementBinder(Cm::Sym::SymbolTable& symbolTable_, Cm::Sym::ConversionTable& conversionTable_, Cm::Core::ClassConversionTable& classConversionTable_, 
        Cm::Sym::ContainerScope* containerScope_, Cm::Sym::FileScope* fileScope_, Cm::BoundTree::BoundFunction* currentFunction_);
    void BeginVisit(Cm::Ast::ConstructionStatementNode& constructionStatementNode) override;
    void EndVisit(Cm::Ast::ConstructionStatementNode& constructionStatementNode) override;
private:
    Cm::BoundTree::BoundConstructionStatement* constructionStatement;
};

class AssignmentStatementBinder : public StatementBinder
{
public:
    AssignmentStatementBinder(Cm::Sym::SymbolTable& symbolTable_, Cm::Sym::ConversionTable& conversionTable_, Cm::Core::ClassConversionTable& classConversionTable_,
        Cm::Sym::ContainerScope* containerScope_, Cm::Sym::FileScope* fileScope_, Cm::BoundTree::BoundFunction* currentFunction_);
    void BeginVisit(Cm::Ast::AssignmentStatementNode& assignmentStatementNode) override;
    void EndVisit(Cm::Ast::AssignmentStatementNode& assignmentStatementNode) override;
};

} } // namespace Cm::Bind

#endif CM_BIND_STATEMENT_INCLUDED
