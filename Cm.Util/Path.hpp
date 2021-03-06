/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_UTIL_PATH_INCLUDED
#define CM_UTIL_PATH_INCLUDED
#include <stdexcept>

namespace Cm { namespace Util {

class InvalidPathException: public std::runtime_error
{
public:
    InvalidPathException(const std::string& message_);
};

std::string GetCurrentWorkingDirectory();
bool FileExists(const std::string& filePath);
bool DirectoryExists(const std::string& directoryPath);
bool PathExists(const std::string& path);
std::string GetFullPath(const std::string& path);

class Path
{
public:
    static std::string MakeCanonical(const std::string& path);
    static std::string ChangeExtension(const std::string& path, const std::string& extension);
    static bool HasExtension(const std::string& path);
    static std::string GetExtension(const std::string& path);
    static std::string GetFileName(const std::string& path);
    static std::string GetFileNameWithoutExtension(const std::string& path);
    static std::string GetDirectoryName(const std::string& path);
    static std::string Combine(const std::string& path1, const std::string& path2);
    static bool IsAbsolute(const std::string& path);
    static bool IsRelative(const std::string& path);
};

} } // namespace Cm::Util

#endif // CM_UTIL_PATH_INCLUDED
