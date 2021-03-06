/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CM_PARSING_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED
#define CM_PARSING_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED

#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Keyword.hpp>
#include <Cm.Parsing/Visitor.hpp>
#include <Cm.Util/CodeFormatter.hpp>

namespace Cm { namespace Parsing { namespace Syntax {

using Cm::Util::CodeFormatter;
using namespace Cm::Parsing;

class CodeGeneratorVisitor : public Cm::Parsing::Visitor
{
public:
    CodeGeneratorVisitor(CodeFormatter& cppFormatter_, CodeFormatter& hppFormatter_);
    virtual void BeginVisit(Grammar& grammar);
    virtual void EndVisit(Grammar& grammar);
    virtual void Visit(CharParser& parser);
    virtual void Visit(StringParser& parser);
    virtual void Visit(CharSetParser& parser);
    virtual void Visit(KeywordParser& parser);
    virtual void Visit(KeywordListParser& parser);
    virtual void Visit(EmptyParser& parser);
    virtual void Visit(SpaceParser& parser);
    virtual void Visit(LetterParser& parser);
    virtual void Visit(DigitParser& parser);
    virtual void Visit(HexDigitParser& parser);
    virtual void Visit(PunctuationParser& parser);
    virtual void Visit(AnyCharParser& parser);
    virtual void Visit(NonterminalParser& parser);
    virtual void Visit(RuleLink& link);
    virtual void BeginVisit(Rule& parser);
    virtual void EndVisit(Rule& parser);
    virtual void BeginVisit(OptionalParser& parser);
    virtual void EndVisit(OptionalParser& parser);
    virtual void BeginVisit(PositiveParser& parser);
    virtual void EndVisit(PositiveParser& parser);
    virtual void BeginVisit(KleeneStarParser& parser);
    virtual void EndVisit(KleeneStarParser& parser);
    virtual void BeginVisit(ActionParser& parser);
    virtual void EndVisit(ActionParser& parser);
    virtual void BeginVisit(ExpectationParser& parser);
    virtual void EndVisit(ExpectationParser& parser);
    virtual void BeginVisit(CCOptParser& parser);
    virtual void EndVisit(CCOptParser& parser);
    virtual void BeginVisit(TokenParser& parser);
    virtual void EndVisit(TokenParser& parser);
    virtual void BeginVisit(SequenceParser& parser);
    virtual void Visit(SequenceParser& parser);
    virtual void EndVisit(SequenceParser& parser);
    virtual void BeginVisit(AlternativeParser& parser);
    virtual void Visit(AlternativeParser& parser);
    virtual void EndVisit(AlternativeParser& parser);
    virtual void BeginVisit(DifferenceParser& parser);
    virtual void Visit(DifferenceParser& parser);
    virtual void EndVisit(DifferenceParser& parser);
    virtual void BeginVisit(IntersectionParser& parser);
    virtual void Visit(IntersectionParser& parser);
    virtual void EndVisit(IntersectionParser& parser);
    virtual void BeginVisit(ExclusiveOrParser& parser);
    virtual void Visit(ExclusiveOrParser& parser);
    virtual void EndVisit(ExclusiveOrParser& parser);
    virtual void BeginVisit(ListParser& parser);
    virtual void Visit(ListParser& parser);
    virtual void EndVisit(ListParser& parser);
private:
    CodeFormatter& cppFormatter;
    CodeFormatter& hppFormatter;
    std::string CharSetStr();
};

} } } // namespace Cm::Parsing::Syntax

#endif // CM_PARSING_SYNTAX_CODEGENERATOR_VISITOR_INCLUDED
