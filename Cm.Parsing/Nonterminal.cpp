/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/Visitor.hpp>

namespace Cm { namespace Parsing {

NonterminalParser::NonterminalParser(const std::string& name_, const std::string& ruleName_):
    Parser(name_, "<" + name_ + ">"), name(name_), ruleName(ruleName_), numberOfArguments(0), preCall(), postCall()
{
}

NonterminalParser::NonterminalParser(const std::string& name_, const std::string& ruleName_, int numberOfArguments_):
    Parser(name_, "<" + name_ + ">"), name(name_), ruleName(ruleName_), numberOfArguments(numberOfArguments_), preCall(), postCall()
{
}

void NonterminalParser::SetPreCall(PreCall* preCall_) 
{ 
    preCall = std::unique_ptr<PreCall>(preCall_); 
}

void NonterminalParser::SetPostCall(PostCall* postCall_) 
{ 
    postCall = std::unique_ptr<PostCall>(postCall_); 
}

void NonterminalParser::SetArguments(const ArgumentVector& arguments_)
{
    arguments = arguments_; 
    for (Cm::Parsing::CppObjectModel::CppObject* arg : arguments)
    {
        if (!arg->IsOwned())
        {
            arg->SetOwned();
            ownedArgs.push_back(std::unique_ptr<Cm::Parsing::CppObjectModel::CppObject>(arg));
        }
    }
}

bool NonterminalParser::Specialized() const 
{ 
    return numberOfArguments > 0 || !arguments.empty() || 
        (rule && rule->TakesOrReturnsValuesOrHasLocals()); 
}

Match NonterminalParser::Parse(Scanner& scanner, ObjectStack& stack)
{
    if (rule)
    {
        if (preCall)
        {
            (*preCall)(stack);
        }
        Match match = rule->Parse(scanner, stack);
        if (postCall)
        {
            (*postCall)(stack, match.Hit());
        }
        return match;
    }
    return Match::Nothing();
}

void NonterminalParser::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace Cm::Parsing
