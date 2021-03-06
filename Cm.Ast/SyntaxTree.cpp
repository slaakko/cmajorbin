/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Ast/SyntaxTree.hpp>
#include <Cm.Ast/Writer.hpp>

namespace Cm { namespace Ast {

SyntaxTree::SyntaxTree()
{
}

SyntaxTree::SyntaxTree(SyntaxTree&& that) : compileUnits(std::move(that.compileUnits))
{
}

SyntaxTree& SyntaxTree::operator=(SyntaxTree&& that)
{
    std::swap(compileUnits, that.compileUnits);
    return *this;
}

void SyntaxTree::AddCompileUnit(CompileUnitNode* compileUnit)
{
    compileUnits.push_back(std::unique_ptr<CompileUnitNode>(compileUnit));
}

void SyntaxTree::Write(Writer& writer)
{
    uint16_t numCompileUnits = static_cast<uint16_t>(compileUnits.size());
    writer.Write(numCompileUnits);
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        writer.Write(compileUnit.get());
    }
}

} } // namespace Cm::Ast
