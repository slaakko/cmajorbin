/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <Cm.Ast/BasicType.hpp>
#include <Cm.Ast/Visitor.hpp>

namespace Cm { namespace Ast {

BoolNode::BoolNode(const Span& span_): Node(span_)
{
}

Node* BoolNode::Clone(CloneContext& cloneContext) const
{
    return new BoolNode(GetSpan());
}

void BoolNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

SByteNode::SByteNode(const Span& span_): Node(span_)
{
}

Node* SByteNode::Clone(CloneContext& cloneContext) const
{
    return new SByteNode(GetSpan());
}

void SByteNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ByteNode::ByteNode(const Span& span_): Node(span_)
{
}

Node* ByteNode::Clone(CloneContext& cloneContext) const
{
    return new ByteNode(GetSpan());
}

void ByteNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ShortNode::ShortNode(const Span& span_): Node(span_)
{
}

Node* ShortNode::Clone(CloneContext& cloneContext) const
{
    return new ShortNode(GetSpan());
}

void ShortNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UShortNode::UShortNode(const Span& span_): Node(span_)
{
}

Node* UShortNode::Clone(CloneContext& cloneContext) const
{
    return new UShortNode(GetSpan());
}

void UShortNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

IntNode::IntNode(const Span& span_): Node(span_)
{
}

Node* IntNode::Clone(CloneContext& cloneContext) const
{
    return new IntNode(GetSpan());
}

void IntNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UIntNode::UIntNode(const Span& span_): Node(span_)
{
}

Node* UIntNode::Clone(CloneContext& cloneContext) const
{
    return new UIntNode(GetSpan());
}

void UIntNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

LongNode::LongNode(const Span& span_): Node(span_)
{
}

Node* LongNode::Clone(CloneContext& cloneContext) const
{
    return new LongNode(GetSpan());
}

void LongNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ULongNode::ULongNode(const Span& span_): Node(span_)
{
}

Node* ULongNode::Clone(CloneContext& cloneContext) const
{
    return new ULongNode(GetSpan());
}

void ULongNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

FloatNode::FloatNode(const Span& span_): Node(span_)
{
}

Node* FloatNode::Clone(CloneContext& cloneContext) const
{
    return new FloatNode(GetSpan());
}

void FloatNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DoubleNode::DoubleNode(const Span& span_): Node(span_)
{
}

Node* DoubleNode::Clone(CloneContext& cloneContext) const
{
    return new DoubleNode(GetSpan());
}

void DoubleNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

CharNode::CharNode(const Span& span_): Node(span_)
{
}

Node* CharNode::Clone(CloneContext& cloneContext) const
{
    return new CharNode(GetSpan());
}

void CharNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

WCharNode::WCharNode(const Span& span_) : Node(span_)
{
}

Node* WCharNode::Clone(CloneContext& cloneContext) const
{
    return new WCharNode(GetSpan());
}

void WCharNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

UCharNode::UCharNode(const Span& span_) : Node(span_)
{
}

Node* UCharNode::Clone(CloneContext& cloneContext) const
{
    return new UCharNode(GetSpan());
}

void UCharNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

VoidNode::VoidNode(const Span& span_): Node(span_)
{
}

Node* VoidNode::Clone(CloneContext& cloneContext) const
{
    return new VoidNode(GetSpan());
}

void VoidNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace Cm::Ast
