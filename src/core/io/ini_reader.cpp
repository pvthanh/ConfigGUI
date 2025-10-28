// SPDX-License-Identifier: MIT
// INIReader - Implementation

#include "ini_reader.h"
#include "ini_parser.h"

namespace configgui {
namespace io {

JsonResult INIReader::readFile(const std::string& file_path)
{
    return INIParser::parseFile(file_path);
}

JsonResult INIReader::readString(const std::string& ini_string)
{
    return INIParser::parse(ini_string);
}

} // namespace io
} // namespace configgui
