#include "Identifier.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>
#include <Cm.Parsing.Cpp/Keyword.hpp>

namespace Cm { namespace Parsing { namespace Cpp {

using namespace Cm::Parsing;

IdentifierGrammar* IdentifierGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

IdentifierGrammar* IdentifierGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    IdentifierGrammar* grammar(new IdentifierGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

IdentifierGrammar::IdentifierGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("IdentifierGrammar", parsingDomain_->GetNamespaceScope("Cm.Parsing.Cpp"), parsingDomain_)
{
    SetOwner(0);
}

std::string IdentifierGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
{
    Cm::Parsing::Scanner scanner(start, end, fileName, fileIndex, SkipRule());
    std::unique_ptr<Cm::Parsing::XmlLog> xmlLog;
    if (Log())
    {
        xmlLog.reset(new Cm::Parsing::XmlLog(*Log(), MaxLogLineLength()));
        scanner.SetLog(xmlLog.get());
        xmlLog->WriteBeginRule("parse");
    }
    Cm::Parsing::ObjectStack stack;
    Cm::Parsing::Match match = Cm::Parsing::Grammar::Parse(scanner, stack);
    Cm::Parsing::Span stop = scanner.GetSpan();
    if (Log())
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw Cm::Parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw Cm::Parsing::ParsingException("grammar '" + Name() + "' has no start rule", fileName, scanner.GetSpan(), start, end);
        }
    }
    std::unique_ptr<Cm::Parsing::Object> value = std::move(stack.top());
    std::string result = *static_cast<Cm::Parsing::ValueObject<std::string>*>(value.get());
    stack.pop();
    return result;
}

class IdentifierGrammar::IdentifierRule : public Cm::Parsing::Rule
{
public:
    IdentifierRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::string");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<std::string>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<IdentifierRule>(this, &IdentifierRule::A0Action));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<IdentifierRule>(this, &IdentifierRule::Postidentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
    void Postidentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromidentifier_value = std::move(stack.top());
            context.fromidentifier = *static_cast<Cm::Parsing::ValueObject<std::string>*>(fromidentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromidentifier() {}
        std::string value;
        std::string fromidentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class IdentifierGrammar::QualifiedIdRule : public Cm::Parsing::Rule
{
public:
    QualifiedIdRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("std::string");
    }
    virtual void Enter(Cm::Parsing::ObjectStack& stack)
    {
        contextStack.push(std::move(context));
        context = Context();
    }
    virtual void Leave(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<std::string>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<QualifiedIdRule>(this, &QualifiedIdRule::A0Action));
        Cm::Parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal("Identifier");
        identifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<QualifiedIdRule>(this, &QualifiedIdRule::PostIdentifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = std::string(matchBegin, matchEnd);
    }
    void PostIdentifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromIdentifier_value = std::move(stack.top());
            context.fromIdentifier = *static_cast<Cm::Parsing::ValueObject<std::string>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromIdentifier() {}
        std::string value;
        std::string fromIdentifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

void IdentifierGrammar::GetReferencedGrammars()
{
    Cm::Parsing::ParsingDomain* pd = GetParsingDomain();
    Cm::Parsing::Grammar* grammar0 = pd->GetGrammar("Cm.Parsing.stdlib");
    if (!grammar0)
    {
        grammar0 = Cm::Parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    Cm::Parsing::Grammar* grammar1 = pd->GetGrammar("Cm.Parsing.Cpp.KeywordGrammar");
    if (!grammar1)
    {
        grammar1 = Cm::Parsing::Cpp::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void IdentifierGrammar::CreateRules()
{
    AddRuleLink(new Cm::Parsing::RuleLink("identifier", this, "Cm.Parsing.stdlib.identifier"));
    AddRuleLink(new Cm::Parsing::RuleLink("Keyword", this, "KeywordGrammar.Keyword"));
    AddRule(new IdentifierRule("Identifier", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::DifferenceParser(
                new Cm::Parsing::NonterminalParser("identifier", "identifier", 0),
                new Cm::Parsing::NonterminalParser("Keyword", "Keyword", 0)))));
    AddRule(new QualifiedIdRule("QualifiedId", GetScope(),
        new Cm::Parsing::ActionParser("A0",
            new Cm::Parsing::TokenParser(
                new Cm::Parsing::SequenceParser(
                    new Cm::Parsing::OptionalParser(
                        new Cm::Parsing::StringParser("::")),
                    new Cm::Parsing::ListParser(
                        new Cm::Parsing::NonterminalParser("Identifier", "Identifier", 0),
                        new Cm::Parsing::StringParser("::")))))));
}

} } } // namespace Cm.Parsing.Cpp
