/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CM_PARSING_PRIMITIVE_INCLUDED
#define CM_PARSING_PRIMITIVE_INCLUDED
#include <Cm.Parsing/Parser.hpp>
#include <limits.h>
#include <bitset>

namespace Cm { namespace Parsing {

class Visitor;
class Rule;

class CharParser : public Parser
{
public:
    CharParser(char c_);
    char GetChar() const { return c; }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
    void SetCCRule(Rule* ccRule_)
    {
        ccRule = ccRule_;
    }
private:
    char c;
    Rule* ccRule;
};

class StringParser : public Parser
{
public:
    StringParser(const std::string& s_);
    const std::string& GetString() const { return s; }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
private:
    std::string s;
};

class CharSetParser : public Parser
{
public:
    CharSetParser(const std::string& s_);
    CharSetParser(const std::string& s_, bool inverse_);
    const std::string& Set() const { return s; }
    bool Inverse() const { return inverse; }
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
private:
    std::string s;
    bool inverse;
    std::bitset<UCHAR_MAX + 1> bits;
    void InitBits();
};

class EmptyParser : public Parser
{
public:
    EmptyParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class SpaceParser : public Parser
{
public:
    SpaceParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class LetterParser : public Parser
{
public:
    LetterParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class DigitParser : public Parser
{
public:
    DigitParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class HexDigitParser : public Parser
{
public:
    HexDigitParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class PunctuationParser : public Parser
{
public:
    PunctuationParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

class AnyCharParser : public Parser
{
public:
    AnyCharParser();
    virtual Match Parse(Scanner& scanner, ObjectStack& stack);
    virtual void Accept(Visitor& visitor);
};

} } // namespace Cm::Parsing

#endif // CM_PARSING_PRIMITIVE_INCLUDED
