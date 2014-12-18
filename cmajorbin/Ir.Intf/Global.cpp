/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <Ir.Intf/Global.hpp>

namespace Ir { namespace Intf {

Global::Global(const std::string& name_, Type* type_): Object(name_, type_)
{
}

} } // namespace Ir::Intf
