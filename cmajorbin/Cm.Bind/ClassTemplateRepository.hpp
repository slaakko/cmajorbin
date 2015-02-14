/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_BIND_CLASS_TEMPLATE_REPOSITORY_INCLUDED
#define CM_BIND_CLASS_TEMPLATE_REPOSITORY_INCLUDED
#include <Cm.Core/ClassTemplateRepository.hpp>
#include <Cm.BoundTree/BoundCompileUnit.hpp>

namespace Cm { namespace Bind {

class ClassTemplateRepository : public Cm::Core::ClassTemplateRepository
{
public:
    ClassTemplateRepository(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::string& groupName, int arity, const std::vector<Cm::Core::Argument>& arguments, const Cm::Parsing::Span& span, Cm::Sym::ContainerScope* containerScope,
        std::unordered_set<Cm::Sym::FunctionSymbol*>& viableFunctions) override;
    bool Instantiated(Cm::Sym::FunctionSymbol* memberFunctionSymbol) const override;
    void Instantiate(Cm::Sym::ContainerScope* containerScope, Cm::Sym::FunctionSymbol* memberFunctionSymbol) override;
private:
    Cm::BoundTree::BoundCompileUnit& boundCompileUnit;
    typedef std::unordered_set<Cm::Sym::FunctionSymbol*> InstantiatedMemberFunctionSet;
    typedef InstantiatedMemberFunctionSet::const_iterator InstantiatedMemberFunctionSetIt;
    InstantiatedMemberFunctionSet instantiatedMemberFunctionSet;
};

} } // namespace Cm::Bind

#endif // CM_BIND_CLASS_TEMPLATE_REPOSITORY_INCLUDED