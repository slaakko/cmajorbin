/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Sym/Exception.hpp>
#include <Cm.Parser/Error.hpp>

namespace Cm { namespace Sym {

Exception::Exception(const std::string& message_, const Span& defined_) : Cm::Ast::Exception(Cm::Parser::Expand(message_, defined_)), message(message_), defined(defined_), referenced(Span())
{
}

Exception::Exception(const std::string& message_, const Span& defined_, const Span& referenced_) : Cm::Ast::Exception(Cm::Parser::Expand(message_, defined_, referenced_)), message(message_),
    defined(defined_), referenced(referenced_)
{
}

void Exception::AddReference(const Span& span)
{
    if (!referenced.Valid())
    {
        referenced = span;
    }
}

} } // namespace Cm::Sym
