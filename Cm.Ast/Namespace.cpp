/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Ast/Namespace.hpp>
#include <Cm.Ast/Identifier.hpp>
#include <Cm.Ast/Reader.hpp>
#include <Cm.Ast/Writer.hpp>
#include <Cm.Ast/Visitor.hpp>

namespace Cm { namespace Ast {

NamespaceNode::NamespaceNode(const Span& span_) : Node(span_), parent(nullptr)
{
}

NamespaceNode::NamespaceNode(const Span& span_, IdentifierNode* id_) : Node(span_), id(id_), parent(nullptr)
{
}

void NamespaceNode::AddMember(Node* member)
{
    member->SetParent(this);
    members.Add(member);
}

Node* NamespaceNode::Clone() const
{
    NamespaceNode* clone = new NamespaceNode(GetSpan(), static_cast<IdentifierNode*>(id->Clone()));
    for (const std::unique_ptr<Node>& member : members)
    {
        clone->AddMember(member->Clone());
    }
    return clone;
}

void NamespaceNode::Read(Reader& reader)
{
    id.reset(reader.ReadIdentifierNode());
    members.Read(reader);
    for (const std::unique_ptr<Node>& member : members)
    {
        member->SetParent(this);
    }
}

void NamespaceNode::Write(Writer& writer)
{
    writer.Write(id.get());
    members.Write(writer);
}

void NamespaceNode::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("namespace " + id->ToString());
    formatter.WriteLine("{");
    formatter.IncIndent();
    members.Print(formatter);
    formatter.DecIndent();
    formatter.WriteLine("}");
}

Node* NamespaceNode::Parent() const
{
    return parent;
}

void NamespaceNode::SetParent(Node* parent_)
{
    parent = parent_;
}

void NamespaceNode::Accept(Visitor& visitor)
{
    visitor.BeginVisit(*this);
    members.Accept(visitor);
    visitor.EndVisit(*this);
}

AliasNode::AliasNode(const Span& span_) : Node(span_), parent(nullptr)
{
}

AliasNode::AliasNode(const Span& span_, IdentifierNode* id_, IdentifierNode* qid_) : Node(span_), id(id_), qid(qid_), parent(nullptr)
{
}

Node* AliasNode::Clone() const
{
    return new AliasNode(GetSpan(), static_cast<IdentifierNode*>(id->Clone()), static_cast<IdentifierNode*>(qid->Clone()));
}

void AliasNode::Read(Reader& reader)
{
    id.reset(reader.ReadIdentifierNode());
    qid.reset(reader.ReadIdentifierNode());
}

void AliasNode::Write(Writer& writer)
{
    writer.Write(id.get());
    writer.Write(qid.get());
}

void AliasNode::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("using " + id->ToString() + " = " + qid->ToString() + ";");
}

Node* AliasNode::Parent() const
{
    return parent;
}

void AliasNode::SetParent(Node* parent_)
{
    parent = parent_;
}

void AliasNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

NamespaceImportNode::NamespaceImportNode(const Span& span_) : Node(span_), parent(nullptr)
{
}

NamespaceImportNode::NamespaceImportNode(const Span& span_, IdentifierNode* ns_) : Node(span_), ns(ns_), parent(nullptr)
{
}

Node* NamespaceImportNode::Clone() const
{
    return new NamespaceImportNode(GetSpan(), static_cast<IdentifierNode*>(ns->Clone()));
}

void NamespaceImportNode::Read(Reader& reader)
{
    ns.reset(reader.ReadIdentifierNode());
}

void NamespaceImportNode::Write(Writer& writer)
{
    writer.Write(ns.get());
}

void NamespaceImportNode::Print(CodeFormatter& formatter)
{
    formatter.WriteLine("using " + ns->ToString() + ";");
}

Node* NamespaceImportNode::Parent() const
{
    return parent;
}

void NamespaceImportNode::SetParent(Node* parent_)
{
    parent = parent_;
}

void NamespaceImportNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace Cm::Ast