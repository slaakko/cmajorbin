/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_STATIC_MEMBER_VARIABLE_REPOSITORY_INCLUDED
#define CM_STATIC_MEMBER_VARIABLE_REPOSITORY_INCLUDED
#include <Cm.Sym/MemberVariableSymbol.hpp>
#include <Cm.IrIntf/Fwd.hpp>
#include <unordered_map>

namespace Cm { namespace Core {

class StaticMemberVariableRepository
{
public:
    virtual ~StaticMemberVariableRepository();
    void Add(Cm::Sym::MemberVariableSymbol* staticMemberVariableSymbol);
    Ir::Intf::Object* GetStaticMemberVariableIrObject(Cm::Sym::MemberVariableSymbol* staticMemberVariableSymbol) const;
    Ir::Intf::Object* GetDestructionNode(Cm::Sym::MemberVariableSymbol* staticMemberVariableSymbol) const;
    virtual void Write(Cm::Util::CodeFormatter& codeFormatter) = 0;
protected:
    typedef std::unordered_map<Cm::Sym::MemberVariableSymbol*, Ir::Intf::Object*> StaticMemberVariableMap;
    const StaticMemberVariableMap& GetStaticMemberVariableMap() const { return staticMemberVariableMap; }
    const StaticMemberVariableMap& GetDestructionNodeMap() const { return destructionNodeMap; }
private:
    typedef StaticMemberVariableMap::const_iterator StaticMemberVariableMapIt;
    StaticMemberVariableMap staticMemberVariableMap;
    StaticMemberVariableMap destructionNodeMap;
    std::vector<std::unique_ptr<Ir::Intf::Object>> ownedIrObjects;
};

class LlvmStaticMemberVariableRepository : public StaticMemberVariableRepository
{
public:
    void Write(Cm::Util::CodeFormatter& codeFormatter) override;
};

class CStaticMemberVariableRepository : public StaticMemberVariableRepository
{
public:
    void Write(Cm::Util::CodeFormatter& codeFormatter) override;
};

} } // namespace Cm::Core

#endif // CM_STATIC_MEMBER_VARIABLE_REPOSITORY_INCLUDED