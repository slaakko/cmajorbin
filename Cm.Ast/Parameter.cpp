/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Ast/Parameter.hpp>
#include <Cm.Ast/Identifier.hpp>
#include <Cm.Ast/Reader.hpp>
#include <Cm.Ast/Writer.hpp>
#include <Cm.Ast/Visitor.hpp>

namespace Cm { namespace Ast {

ParameterNode::ParameterNode(const Span& span_) : Node(span_)
{
}

ParameterNode::ParameterNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_) : Node(span_), typeExpr(typeExpr_), id(id_)
{
    typeExpr->SetParent(this);
    if (id)
    {
        id->SetParent(this);
    }
}

Node* ParameterNode::Clone(CloneContext& cloneContext) const
{
    IdentifierNode* clonedId = nullptr;
    if (id)
    {
        clonedId = static_cast<IdentifierNode*>(id->Clone(cloneContext));
    }
    return new ParameterNode(GetSpan(), typeExpr->Clone(cloneContext), clonedId);
}

void ParameterNode::Read(Reader& reader)
{
    typeExpr.reset(reader.ReadNode());
    typeExpr->SetParent(this);
    bool hasId = reader.ReadBool();
    if (hasId)
    {
        id.reset(reader.ReadIdentifierNode());
        id->SetParent(this);
    }
}

void ParameterNode::Write(Writer& writer)
{
    writer.Write(typeExpr.get());
    bool hasId = id != nullptr;
    writer.Write(hasId);
    if (hasId)
    {
        writer.Write(id.get());
    }
}

void ParameterNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

std::string ParameterNode::ToString() const
{
    std::string s = typeExpr->ToString();
    if (id)
    {
        s.append(1, ' ').append(id->ToString());
    }
    return s;
}

ParameterNodeList::ParameterNodeList()
{
}

void ParameterNodeList::Read(Reader& reader)
{
    uint32_t n = reader.ReadUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        parameterNodes.push_back(std::unique_ptr<ParameterNode>(reader.ReadParameterNode()));
    }
}

void ParameterNodeList::Write(Writer& writer)
{
    uint32_t n = static_cast<uint32_t>(parameterNodes.size());
    writer.Write(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        writer.Write(parameterNodes[i].get());
    }
}

void ParameterNodeList::Accept(Visitor& visitor)
{
    for (const std::unique_ptr<ParameterNode>& parameter : parameterNodes)
    {
        parameter->Accept(visitor);
    }
}

std::string ParameterNodeList::ToString() const
{
    std::string s(1, '(');
    bool first = true;
    for (const std::unique_ptr<ParameterNode>& parameter : parameterNodes)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            s.append(", ");
        }
        s.append(parameter->ToString());
    }
    s.append(1, ')');
    return s;
}

void ParameterNodeList::SetParent(Node* parent)
{
    for (const std::unique_ptr<ParameterNode>& parameter : parameterNodes)
    {
        parameter->SetParent(parent);
    }
}

} } // namespace Cm::Ast
