/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Ast/TypeExpr.hpp>
#include <Cm.Ast/Reader.hpp>
#include <Cm.Ast/Writer.hpp>

namespace Cm { namespace Ast {

std::string DerivationStr(Derivation d)
{
    switch (d)
    {
        case Derivation::const_: return "const";
        case Derivation::reference: return "&";
        case Derivation::rvalueRef: return "&&";
        case Derivation::pointer: return "*";
        case Derivation::leftParen: return "(";
        case Derivation::rightParen: return ")";
    }
    return "";
}

DerivationList::DerivationList() : numDerivations(0)
{
    memset(&derivations[0], 0, sizeof(derivations));
}

void DerivationList::Add(Derivation derivation)
{
    if (numDerivations >= maxDerivations)
    {
        throw std::runtime_error("too many derivations");
    }
    derivations[numDerivations++] = derivation;
}

DerivedTypeExprNode::DerivedTypeExprNode(const Span& span_): Node(span_)
{
}

DerivedTypeExprNode::DerivedTypeExprNode(const Span& span_, const DerivationList& derivations_, Node* baseTypeExprNode_): Node(span_), derivations(derivations_), baseTypeExprNode(baseTypeExprNode_)
{
}

Node* DerivedTypeExprNode::Clone() const
{
    return new DerivedTypeExprNode(GetSpan(), derivations, baseTypeExprNode->Clone());
}

void DerivedTypeExprNode::Read(Reader& reader)
{
    derivations = reader.ReadDerivationList();
    baseTypeExprNode.reset(reader.ReadNode());
}

void DerivedTypeExprNode::Write(Writer& writer)
{
    writer.Write(derivations);
    writer.Write(baseTypeExprNode.get());
}

std::string DerivedTypeExprNode::ToString() const
{
    std::string s;
    uint8_t derivationIndex = 0;
    uint8_t n = derivations.NumDerivations();
    if (derivationIndex < n)
    {
        Derivation d = derivations[derivationIndex];
        if (d == Derivation::const_)
        {
            s.append(DerivationStr(d));
            ++derivationIndex;
        }
    }
    if (derivationIndex < n)
    {
        Derivation d = derivations[derivationIndex];
        if (d == Derivation::leftParen)
        {
            if (!s.empty())
            {
                s.append(1, ' ');
            }
            s.append(DerivationStr(d));
            ++derivationIndex;
            if (derivationIndex < n)
            {
                Derivation d = derivations[derivationIndex];
                if (d == Derivation::const_)
                {
                    s.append(DerivationStr(d));
                    ++derivationIndex;
                }
            }
        }
    }
    if (!s.empty())
    {
        s.append(1, ' ');
    }
    s.append(baseTypeExprNode->ToString());
    for (uint8_t i = derivationIndex; i < n; ++i)
    {
        s.append(DerivationStr(derivations[i]));
    }
    return s;
}

void DerivedTypeExprNode::Add(Derivation derivation)
{
    derivations.Add(derivation);
}

void DerivedTypeExprNode::SetBaseTypeExpr(Node* baseTypeExprNode_)
{
    baseTypeExprNode.reset(baseTypeExprNode_);
}

Node* MakeTypeExprNode(DerivedTypeExprNode* derivedTypeExprNode)
{
    if (derivedTypeExprNode->Derivations().NumDerivations() == 0)
    {
        Node* baseTypeExprNode = derivedTypeExprNode->ReleaseBaseTypeExprNode();
        delete derivedTypeExprNode;
        return baseTypeExprNode;
    }
    return derivedTypeExprNode;
}

} } // namespace Cm::Ast
