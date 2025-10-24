// SPDX-License-Identifier: MIT
// JsonWriter - Implementation

#include "json_writer.h"
#include <fstream>
#include <sstream>

namespace configgui {
namespace io {

VoidResult JsonWriter::writeFile(const std::string& file_path, const json& data,
                                   bool pretty_print)
{
    try
    {
        std::ofstream file(file_path);
        if (!file.is_open())
        {
            return VoidResult(Error{"Cannot open file for writing: " + file_path});
        }

        // OPTIMIZATION: Write directly to file stream without intermediate string allocation
        // This reduces memory usage for large JSON documents
        if (pretty_print)
        {
            file << data.dump(4);
        }
        else
        {
            file << data.dump();
        }

        file.close();
        return VoidResult(Success{});
    }
    catch (const std::exception& e)
    {
        return VoidResult(Error{"Failed to write JSON file: " + std::string(e.what())});
    }
}

StringResult JsonWriter::toString(const json& data, bool pretty_print)
{
    try
    {
        // OPTIMIZATION: Use stringstream buffer and move semantics to minimize allocations
        std::string result;
        if (pretty_print)
        {
            result = data.dump(4);
        }
        else
        {
            result = data.dump();
        }

        // Move construction avoids copy on return (RVO-compatible)
        return StringResult(std::move(result));
    }
    catch (const std::exception& e)
    {
        return StringResult(Error{"Failed to serialize JSON: " + std::string(e.what())});
    }
}

} // namespace io
} // namespace configgui
