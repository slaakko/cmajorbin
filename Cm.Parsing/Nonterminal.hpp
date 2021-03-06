/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CM_PARSING_NONTERMINAL_INCLUDED
#define CM_PARSING_NONTERMINAL_INCLUDED
#include <Cm.Parsing/Parser.hpp>
#include <Cm.Parsing.CppObjectModel/Object.hpp>
#include <cctype>

namespace Cm { namespace Parsing {

class Rule;
class PreCall;
class PostCall;

class NonterminalParser: public Parser
{
public:
    typedef std::vector<Cm::Parsing::CppObjectModel::CppObject*> ArgumentVector;
    NonterminalParser(const std::string& name_, const std::string& ruleName_);
    NonterminalParser(const std::string& name_, const std::string& ruleName_, int numberOfArguments_);
    const std::string& RuleName() const { return ruleName; }
    const ArgumentVector& Arguments() const { return arguments; }
    Rule* GetRule() const { return rule; }
    void SetRule(Rule* rule_) { rule = rule_; }
    void SetArguments(const ArgumentVector& arguments_);
    int NumberOfArguments() const { return numberOfArguments != 0 ? numberOfArguments : int(arguments.size()); }
    void SetPreCall(PreCall* preCall_);
    void SetPostCall(PostCall* postCall_);
    const Span& GetSpan() const { return span; }
    void SetSpan(const Span& span_) { span = span_; }
    std::string PreCallMethodName() const { return "Pre" + name; }
    std::string PostCallMethodName() const { return "Post" + name; }
    std::string ValueFieldName() const { return "from" + name; }
    std::string VariableName() const { return (!name.empty() ? std::string(1, std::tolower(name[0])) + name.substr(1) : name) + "NonterminalParser"; }
    bool Specialized() const;
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
    virtual bool IsNonterminalParser() const { return true; }
private:
    std::string name;
    std::string ruleName;
    int numberOfArguments;
    ArgumentVector arguments;
    std::vector<std::unique_ptr<Cm::Parsing::CppObjectModel::CppObject>> ownedArgs;
    std::unique_ptr<PreCall> preCall;
    std::unique_ptr<PostCall> postCall;
    Span span;
    Rule* rule;
};

} } // namespace Cm::Parsing

#endif // CM_PARSING_NONTERMINAL_INCLUDED
