/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <Cm.Parsing/ParsingDomain.hpp>
#include <Cm.Parsing/Namespace.hpp>
#include <Cm.Parsing/Scope.hpp>
#include <Cm.Parsing/Grammar.hpp>
#include <memory>
#include <unordered_set>

namespace Cm { namespace Parsing {

class ParsingDomainRepository
{
public:
    static void Init();
    static void Done();
    static ParsingDomainRepository& Instance();
    void Register(ParsingDomain* parsingDomain);
private:
    ParsingDomainRepository();
    static std::unique_ptr<ParsingDomainRepository> instance;
    static int initCount;
    typedef std::unordered_set<std::unique_ptr<ParsingDomain>> ParsingDomainSet;
    typedef ParsingDomainSet::const_iterator ParsingDomainSetIt;
    ParsingDomainSet parsingDomains;
};

ParsingDomainRepository::ParsingDomainRepository()
{
}

void ParsingDomainRepository::Register(ParsingDomain* parsingDomain)
{
    if (!parsingDomain->IsOwned())
    {
        parsingDomain->SetOwned();
        parsingDomains.insert(std::unique_ptr<ParsingDomain>(parsingDomain));
    }
}

int ParsingDomainRepository::initCount = 0;

void ParsingDomainRepository::Init()
{
    if (initCount++ == 0)
    {
        instance.reset(new ParsingDomainRepository());
    }
}

void ParsingDomainRepository::Done()
{
    if (--initCount == 0)
    {
        instance.reset();
    }
}

ParsingDomainRepository& ParsingDomainRepository::Instance()
{
    return *instance;
}

std::unique_ptr<ParsingDomainRepository> ParsingDomainRepository::instance;

ParsingDomain::ParsingDomain(): ParsingObject("parsingDomain"), 
    globalNamespace(new Namespace("", nullptr)), globalScope(globalNamespace->GetScope()), currentNamespace(globalNamespace)
{
    Own(globalNamespace);
    globalScope->SetNs(globalNamespace);
    namespaceMap[currentNamespace->FullName()] = currentNamespace;
}

Scope* ParsingDomain::CurrentScope() const 
{ 
    return currentNamespace->GetScope(); 
}

void ParsingDomain::BeginNamespace(const std::string& ns)
{
    std::string fullNamespaceName = currentNamespace->FullName().empty() ? ns : currentNamespace->FullName() + "." + ns;
    namespaceStack.push(currentNamespace);
    NamespaceMapIt i = namespaceMap.find(fullNamespaceName);
    if (i != namespaceMap.end())
    {
        currentNamespace = i->second;
    }
    else
    {
        Namespace* newNs(new Namespace(fullNamespaceName, globalScope));
        Own(newNs);
        newNs->GetScope()->SetNs(newNs);
        namespaceMap[newNs->FullName()] = newNs;
        currentNamespace = newNs;
        globalScope->AddNamespace(newNs);
    }
}

void ParsingDomain::EndNamespace()
{
    currentNamespace = namespaceStack.top();
    namespaceStack.pop();
}

Scope* ParsingDomain::GetNamespaceScope(const std::string& fullNamespaceName)
{
    if (fullNamespaceName.empty())
    {
        return globalScope;
    }
    Namespace* ns = globalScope->GetNamespace(fullNamespaceName);
    if (!ns)
    {
        ns = new Namespace(fullNamespaceName, globalScope);
        Own(ns);
        globalScope->AddNamespace(ns);
    }
    return ns->GetScope();
}

Grammar* ParsingDomain::GetGrammar(const std::string& grammarName)
{
    GrammarMapIt i = grammarMap.find(grammarName);
    if (i != grammarMap.end())
    {
        return i->second;
    }
    return nullptr;
}

void ParsingDomain::AddGrammar(Grammar* grammar)
{
    Own(grammar);
    std::string grammarFullName = grammar->FullName();
    GrammarMapIt i = grammarMap.find(grammarFullName);
    if (i == grammarMap.end())
    {
        grammarMap[grammarFullName] = grammar;
        grammar->SetNs(currentNamespace);
        grammar->AddToScope();
    }
}

void ParsingDomain::Accept(Visitor& visitor)
{
    for (const std::pair<std::string, Grammar*>& p : grammarMap)
    {
        Grammar* grammar = p.second;
        grammar->Accept(visitor);
    }
}

void RegisterParsingDomain(ParsingDomain* parsingDomain)
{
    ParsingDomainRepository::Instance().Register(parsingDomain);
}

void ParsingDomainInit()
{
    ParsingDomainRepository::Init();
}

void ParsingDomainDone()
{
    ParsingDomainRepository::Done();
}

} } // namespace Cm::Parsing
