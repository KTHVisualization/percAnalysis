#pragma once

#include <string.h>
#include <kxtools/kxtoolsmoduledefine.h>

namespace inviwo
{

IVW_MODULE_KXTOOLS_API bool ContainsOnlyWhiteSpacesOrIsComment(const char* buf);
IVW_MODULE_KXTOOLS_API bool ReadNextLine(FILE* fp, char* buf, const int bufsize);
IVW_MODULE_KXTOOLS_API char* JumpToFirstValue(char* buf);
IVW_MODULE_KXTOOLS_API char* ReadAndJump(FILE* fp, char* buf, const int bufsize);
IVW_MODULE_KXTOOLS_API std::string ReadNextQuotedString(FILE* fp, char* buf, const int bufsize);

//IVW_MODULE_KXTOOLS_API bool ScanAndJump(FILE* fp, char* buf, const int bufsize, const char* format, ...);

};
