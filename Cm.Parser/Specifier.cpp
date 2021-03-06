#include "Specifier.hpp"
#include <Cm.Parsing/Action.hpp>
#include <Cm.Parsing/Rule.hpp>
#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Primitive.hpp>
#include <Cm.Parsing/Composite.hpp>
#include <Cm.Parsing/Nonterminal.hpp>
#include <Cm.Parsing/Exception.hpp>
#include <Cm.Parsing/StdLib.hpp>
#include <Cm.Parsing/XmlLog.hpp>

namespace Cm { namespace Parser {

using namespace Cm::Ast;
using namespace Cm::Parsing;

SpecifierGrammar* SpecifierGrammar::Create()
{
    return Create(new Cm::Parsing::ParsingDomain());
}

SpecifierGrammar* SpecifierGrammar::Create(Cm::Parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    SpecifierGrammar* grammar(new SpecifierGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

SpecifierGrammar::SpecifierGrammar(Cm::Parsing::ParsingDomain* parsingDomain_): Cm::Parsing::Grammar("SpecifierGrammar", parsingDomain_->GetNamespaceScope("Cm.Parser"), parsingDomain_)
{
    SetOwner(0);
}

Cm::Ast::Specifiers SpecifierGrammar::Parse(const char* start, const char* end, int fileIndex, const std::string& fileName)
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
    if (!match.Hit() || !CC() && stop.Start() != int(end - start))
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
    Cm::Ast::Specifiers result = *static_cast<Cm::Parsing::ValueObject<Cm::Ast::Specifiers>*>(value.get());
    stack.pop();
    return result;
}

class SpecifierGrammar::SpecifiersRule : public Cm::Parsing::Rule
{
public:
    SpecifiersRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Cm::Ast::Specifiers");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Cm::Ast::Specifiers>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifiersRule>(this, &SpecifiersRule::A0Action));
        Cm::Parsing::NonterminalParser* specifierNonterminalParser = GetNonterminal("Specifier");
        specifierNonterminalParser->SetPostCall(new Cm::Parsing::MemberPostCall<SpecifiersRule>(this, &SpecifiersRule::PostSpecifier));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = context.value | context.fromSpecifier;
    }
    void PostSpecifier(Cm::Parsing::ObjectStack& stack, bool matched)
    {
        if (matched)
        {
            std::unique_ptr<Cm::Parsing::Object> fromSpecifier_value = std::move(stack.top());
            context.fromSpecifier = *static_cast<Cm::Parsing::ValueObject<Cm::Ast::Specifiers>*>(fromSpecifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context
    {
        Context(): value(), fromSpecifier() {}
        Cm::Ast::Specifiers value;
        Cm::Ast::Specifiers fromSpecifier;
    };
    std::stack<Context> contextStack;
    Context context;
};

class SpecifierGrammar::SpecifierRule : public Cm::Parsing::Rule
{
public:
    SpecifierRule(const std::string& name_, Scope* enclosingScope_, Parser* definition_):
        Cm::Parsing::Rule(name_, enclosingScope_, definition_), contextStack(), context()
    {
        SetValueTypeName("Cm::Ast::Specifiers");
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
            stack.push(std::unique_ptr<Cm::Parsing::Object>(new Cm::Parsing::ValueObject<Cm::Ast::Specifiers>(context.value)));
        }
        context = std::move(contextStack.top());
        contextStack.pop();
    }
    virtual void Link()
    {
        Cm::Parsing::ActionParser* a0ActionParser = GetAction("A0");
        a0ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A0Action));
        Cm::Parsing::ActionParser* a1ActionParser = GetAction("A1");
        a1ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A1Action));
        Cm::Parsing::ActionParser* a2ActionParser = GetAction("A2");
        a2ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A2Action));
        Cm::Parsing::ActionParser* a3ActionParser = GetAction("A3");
        a3ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A3Action));
        Cm::Parsing::ActionParser* a4ActionParser = GetAction("A4");
        a4ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A4Action));
        Cm::Parsing::ActionParser* a5ActionParser = GetAction("A5");
        a5ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A5Action));
        Cm::Parsing::ActionParser* a6ActionParser = GetAction("A6");
        a6ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A6Action));
        Cm::Parsing::ActionParser* a7ActionParser = GetAction("A7");
        a7ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A7Action));
        Cm::Parsing::ActionParser* a8ActionParser = GetAction("A8");
        a8ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A8Action));
        Cm::Parsing::ActionParser* a9ActionParser = GetAction("A9");
        a9ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A9Action));
        Cm::Parsing::ActionParser* a10ActionParser = GetAction("A10");
        a10ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A10Action));
        Cm::Parsing::ActionParser* a11ActionParser = GetAction("A11");
        a11ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A11Action));
        Cm::Parsing::ActionParser* a12ActionParser = GetAction("A12");
        a12ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A12Action));
        Cm::Parsing::ActionParser* a13ActionParser = GetAction("A13");
        a13ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A13Action));
        Cm::Parsing::ActionParser* a14ActionParser = GetAction("A14");
        a14ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A14Action));
        Cm::Parsing::ActionParser* a15ActionParser = GetAction("A15");
        a15ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A15Action));
        Cm::Parsing::ActionParser* a16ActionParser = GetAction("A16");
        a16ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A16Action));
        Cm::Parsing::ActionParser* a17ActionParser = GetAction("A17");
        a17ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A17Action));
        Cm::Parsing::ActionParser* a18ActionParser = GetAction("A18");
        a18ActionParser->SetAction(new Cm::Parsing::MemberParsingAction<SpecifierRule>(this, &SpecifierRule::A18Action));
    }
    void A0Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::public_;
    }
    void A1Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::protected_;
    }
    void A2Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::private_;
    }
    void A3Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::internal_;
    }
    void A4Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::static_;
    }
    void A5Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::virtual_;
    }
    void A6Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::override_;
    }
    void A7Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::abstract_;
    }
    void A8Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::suppress;
    }
    void A9Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::default_;
    }
    void A10Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::explicit_;
    }
    void A11Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::external;
    }
    void A12Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::inline_;
    }
    void A13Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::constexpr_;
    }
    void A14Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::cdecl_;
    }
    void A15Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::nothrow_;
    }
    void A16Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::throw_;
    }
    void A17Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::new_;
    }
    void A18Action(const char* matchBegin, const char* matchEnd, const Span& span, const std::string& fileName, bool& pass)
    {
        context.value = Specifiers::unit_test;
    }
private:
    struct Context
    {
        Context(): value() {}
        Cm::Ast::Specifiers value;
    };
    std::stack<Context> contextStack;
    Context context;
};

void SpecifierGrammar::GetReferencedGrammars()
{
}

void SpecifierGrammar::CreateRules()
{
    AddRule(new SpecifiersRule("Specifiers", GetScope(),
        new Cm::Parsing::KleeneStarParser(
            new Cm::Parsing::ActionParser("A0",
                new Cm::Parsing::NonterminalParser("Specifier", "Specifier", 0)))));
    AddRule(new SpecifierRule("Specifier", GetScope(),
        new Cm::Parsing::AlternativeParser(
            new Cm::Parsing::AlternativeParser(
                new Cm::Parsing::AlternativeParser(
                    new Cm::Parsing::AlternativeParser(
                        new Cm::Parsing::AlternativeParser(
                            new Cm::Parsing::AlternativeParser(
                                new Cm::Parsing::AlternativeParser(
                                    new Cm::Parsing::AlternativeParser(
                                        new Cm::Parsing::AlternativeParser(
                                            new Cm::Parsing::AlternativeParser(
                                                new Cm::Parsing::AlternativeParser(
                                                    new Cm::Parsing::AlternativeParser(
                                                        new Cm::Parsing::AlternativeParser(
                                                            new Cm::Parsing::AlternativeParser(
                                                                new Cm::Parsing::AlternativeParser(
                                                                    new Cm::Parsing::AlternativeParser(
                                                                        new Cm::Parsing::AlternativeParser(
                                                                            new Cm::Parsing::AlternativeParser(
                                                                                new Cm::Parsing::ActionParser("A0",
                                                                                    new Cm::Parsing::KeywordParser("public")),
                                                                                new Cm::Parsing::ActionParser("A1",
                                                                                    new Cm::Parsing::KeywordParser("protected"))),
                                                                            new Cm::Parsing::ActionParser("A2",
                                                                                new Cm::Parsing::KeywordParser("private"))),
                                                                        new Cm::Parsing::ActionParser("A3",
                                                                            new Cm::Parsing::KeywordParser("internal"))),
                                                                    new Cm::Parsing::ActionParser("A4",
                                                                        new Cm::Parsing::KeywordParser("static"))),
                                                                new Cm::Parsing::ActionParser("A5",
                                                                    new Cm::Parsing::KeywordParser("virtual"))),
                                                            new Cm::Parsing::ActionParser("A6",
                                                                new Cm::Parsing::KeywordParser("override"))),
                                                        new Cm::Parsing::ActionParser("A7",
                                                            new Cm::Parsing::KeywordParser("abstract"))),
                                                    new Cm::Parsing::ActionParser("A8",
                                                        new Cm::Parsing::KeywordParser("suppress"))),
                                                new Cm::Parsing::ActionParser("A9",
                                                    new Cm::Parsing::KeywordParser("default"))),
                                            new Cm::Parsing::ActionParser("A10",
                                                new Cm::Parsing::KeywordParser("explicit"))),
                                        new Cm::Parsing::ActionParser("A11",
                                            new Cm::Parsing::KeywordParser("extern"))),
                                    new Cm::Parsing::ActionParser("A12",
                                        new Cm::Parsing::KeywordParser("inline"))),
                                new Cm::Parsing::ActionParser("A13",
                                    new Cm::Parsing::KeywordParser("constexpr"))),
                            new Cm::Parsing::ActionParser("A14",
                                new Cm::Parsing::KeywordParser("cdecl"))),
                        new Cm::Parsing::ActionParser("A15",
                            new Cm::Parsing::KeywordParser("nothrow"))),
                    new Cm::Parsing::ActionParser("A16",
                        new Cm::Parsing::KeywordParser("throw"))),
                new Cm::Parsing::ActionParser("A17",
                    new Cm::Parsing::KeywordParser("new"))),
            new Cm::Parsing::ActionParser("A18",
                new Cm::Parsing::KeywordParser("unit_test")))));
}

} } // namespace Cm.Parser
