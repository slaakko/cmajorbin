/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Emit/InitDone.hpp>
#include <Cm.Emit/SourceFileCache.hpp>

namespace Cm { namespace Emit {

void Init()
{
    SourceFileCacheInit();
}

void Done()
{
    SourceFileCacheDone();
}

}  } // namespace Cm::Emit