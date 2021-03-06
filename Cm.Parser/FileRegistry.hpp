/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_PARSER_FILE_REGISTRY
#define CM_PARSER_FILE_REGISTRY
#include <memory>
#include <string>
#include <vector>

namespace Cm { namespace Parser {

class FileRegistry
{
public:
    static void Init();
    static void Done();
    static FileRegistry* Instance();
    int RegisterParsedFile(const std::string& filePath);
    const std::string& GetParsedFileName(int parsedFileIndex) const;
    int GetNumberOfParsedFiles() const;
private:
    static std::unique_ptr<FileRegistry> instance;
    std::vector<std::string> parsedFiles;
};

} } // namespace Cm::Parser

#endif // CM_PARSER_FILE_REGISTRY
