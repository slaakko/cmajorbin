/*========================================================================
Copyright (c) 2012-2015 Seppo Laakko
http://sourceforge.net/projects/cmajor/

Distributed under the GNU General Public License, version 3 (GPLv3).
(See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Ast/Delegate.hpp>
#include <Cm.Ast/Identifier.hpp>
#include <Cm.Ast/Parameter.hpp>
#include <Cm.Ast/Reader.hpp>
#include <Cm.Ast/Writer.hpp>

namespace Cm { namespace Ast {

DelegateNode::DelegateNode(const Span& span_) : Node(span_), specifiers(Specifiers::none)
{
}

DelegateNode::DelegateNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, IdentifierNode* id_) : Node(span_), specifiers(specifiers_), returnTypeExpr(returnTypeExpr_), id(id_)
{
}

Node* DelegateNode::Clone() const
{
    DelegateNode* clone = new DelegateNode(GetSpan(), specifiers, returnTypeExpr->Clone(), static_cast<IdentifierNode*>(id->Clone()));
    for (const std::unique_ptr<Node>& parameterNode : parameters)
    {
        ParameterNode* parameter = static_cast<ParameterNode*>(parameterNode.get());
        clone->AddParameter(parameter->GetSpan(), parameter->TypeExpr()->Clone(), static_cast<IdentifierNode*>(parameter->Id()->Clone()));
    }
    return clone;
}

void DelegateNode::AddParameter(const Span& span_, Node* paramTypeExpr, IdentifierNode* paramId)
{
    if (paramId == nullptr)
    {
        paramId = new IdentifierNode(span_, "__parameter" + std::to_string(parameters.Count()));
    }
    parameters.Add(new ParameterNode(span_, paramTypeExpr, paramId));
}

void DelegateNode::Read(Reader& reader)
{
    specifiers = reader.ReadSpecifiers();
    returnTypeExpr.reset(reader.ReadNode());
    id.reset(reader.ReadIdentifierNode());
    parameters.Read(reader);
}

void DelegateNode::Write(Writer& writer)
{
    writer.Write(specifiers);
    writer.Write(returnTypeExpr.get());
    writer.Write(id.get());
    parameters.Write(writer);
}

ClassDelegateNode::ClassDelegateNode(const Span& span_) : Node(span_), specifiers(Specifiers::none)
{
}

ClassDelegateNode::ClassDelegateNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, IdentifierNode* id_) : Node(span_), specifiers(specifiers_), returnTypeExpr(returnTypeExpr_), id(id_)
{
}

Node* ClassDelegateNode::Clone() const
{
    ClassDelegateNode* clone = new ClassDelegateNode(GetSpan(), specifiers, returnTypeExpr->Clone(), static_cast<IdentifierNode*>(id->Clone()));
    for (const std::unique_ptr<Node>& parameterNode : parameters)
    {
        ParameterNode* parameter = static_cast<ParameterNode*>(parameterNode.get());
        clone->AddParameter(parameter->GetSpan(), parameter->TypeExpr()->Clone(), static_cast<IdentifierNode*>(parameter->Id()->Clone()));
    }
    return clone;
}

void ClassDelegateNode::AddParameter(const Span& span_, Node* paramTypeExpr, IdentifierNode* paramId)
{
    if (paramId == nullptr)
    {
        paramId = new IdentifierNode(span_, "__parameter" + std::to_string(parameters.Count()));
    }
    parameters.Add(new ParameterNode(span_, paramTypeExpr, paramId));
}

void ClassDelegateNode::Read(Reader& reader)
{
    specifiers = reader.ReadSpecifiers();
    returnTypeExpr.reset(reader.ReadNode());
    id.reset(reader.ReadIdentifierNode());
    parameters.Read(reader);
}

void ClassDelegateNode::Write(Writer& writer)
{
    writer.Write(specifiers);
    writer.Write(returnTypeExpr.get());
    writer.Write(id.get());
    parameters.Write(writer);
}

} } // namespace Cm::Ast
