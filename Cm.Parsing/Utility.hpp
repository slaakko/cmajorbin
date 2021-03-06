/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef CM_PARSING_UTILITY_INCLUDED
#define CM_PARSING_UTILITY_INCLUDED
#include <stdint.h>
#include <string>
#include <vector>

namespace Cm { namespace Parsing {

std::string HexEscape(char c);
std::string XmlCharStr(char c);
std::string XmlEscape(const std::string& s);

} } // namespace Cm::Parsing

#endif // CM_PARSING_UTILITY_INCLUDED
