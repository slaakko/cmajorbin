/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <Cm.Parsing/Namespace.hpp>
#include <Cm.Parsing/Scope.hpp>
#include <Cm.Parsing/Visitor.hpp>

namespace Cm { namespace Parsing {

Namespace::Namespace(const std::string& name_, Scope* enclosingScope_): ParsingObject(name_, enclosingScope_)
{
    SetScope(new Scope(Name(), EnclosingScope())); 
}

void Namespace::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    GetScope()->Accept(visitor);
    visitor.EndVisit(*this);
}

UsingObject::UsingObject(Cm::Parsing::CppObjectModel::UsingObject* subject_, Scope* enclosingScope_): ParsingObject(subject_->Name(), enclosingScope_), subject(subject_)
{
    if (!subject->IsOwned())
    {
        subject->SetOwned();
        ownedSubject.reset(subject);
    }
}

void UsingObject::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace Cm::Parsing
