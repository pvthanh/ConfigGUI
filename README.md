# ConfigGUI - Schema-Driven Configuration Manager

A dual-platform configuration management tool providing both Qt GUI and HTML Server interfaces for managing JSON/YAML configurations based on JSON Schema.

## Quick Start

### Prerequisites
- C++17 compiler (GCC/Clang)
- CMake 3.20+
- Qt6 (for GUI) or cpp-httplib (for server)

### Build & Run

#### Qt GUI Only
```bash
mkdir build && cd build
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF ..
cmake --build . --parallel 4
./src/qt/ConfigGUI
```

#### HTML Server Only
```bash
mkdir build && cd build
cmake -DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON ..
cmake --build . --parallel 4
./src/html/ConfigGUIServer  # Runs on http://localhost:8080
```

#### Both Targets
```bash
mkdir build && cd build
cmake -DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON ..
cmake --build . --parallel 4
```

## Documentation

- **[SOFTWARE_ARCHITECTURE.md](SOFTWARE_ARCHITECTURE.md)** - System design, module organization, and technical architecture
- **[INSTALLATION_AND_USAGE_GUIDE.md](INSTALLATION_AND_USAGE_GUIDE.md)** - Detailed setup, build, and usage instructions
- **[BUG_FIXES_AND_SOLUTIONS.md](BUG_FIXES_AND_SOLUTIONS.md)** - Known issues, fixes applied, and lessons learned

## Key Features

✨ **Schema-Driven UI Generation**
- Automatic form generation from JSON Schema
- Supports all JSON data types (objects, arrays, enums, etc.)
- Type validation and constraints

🎨 **Dual Interfaces**
- Qt6 Desktop Application (Windows, Linux, macOS)
- HTML5 Server with responsive web UI (Docker-ready)

💾 **Multiple Format Support**
- JSON configuration files
- YAML configuration files
- Round-trip conversion with data preservation

📋 **Advanced Features**
- Collapsible nested object sections
- Array property labels for clarity
- Dictionary (key-value map) widgets
- Lazy-loaded arrays for performance
- Comprehensive error handling

## Project Structure

```
configGui/
├── src/
│   ├── core/          # Shared C++17 library (no Qt dependency)
│   ├── qt/            # Qt GUI application
│   ├── html/          # HTTP server application
│   ├── ui/            # Qt UI widgets
│   ├── io/            # File I/O (JSON/YAML)
│   ├── validators/    # Validation logic
│   └── utils/         # Utility functions
├── resources/
│   ├── schemas/       # Example JSON schemas
│   ├── configs/       # Example configurations
│   └── html/          # Web assets
└── tests/             # Unit and integration tests
```

## Testing

```bash
cd build
ctest --output-on-failure
```

**Test Results:**
- ✅ 19/19 Integration tests passing (100%)
- ✅ All form generation and data collection verified
- ✅ Cross-platform compatibility confirmed

## Build Variants

| Variant | Command | Produces | Qt Required |
|---------|---------|----------|-------------|
| GUI Only | `-DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=OFF` | ConfigGUI | ✅ Yes |
| Server Only | `-DBUILD_QT_APP=OFF -DBUILD_HTML_SERVER=ON` | ConfigGUIServer | ❌ No |
| Both | `-DBUILD_QT_APP=ON -DBUILD_HTML_SERVER=ON` | Both | ✅ Yes |

## Known Limitations

- Maximum 10 levels of nested object depth (safety limit)
- Large schemas (1000+ fields) may show degraded performance on older hardware
- Web version tested on Chrome 90+, Firefox 88+, Safari 14+

## License

See LICENSE file for details.

## Contributing

Contributions welcome! Please ensure:
- Code follows MISRA C++ guidelines
- All tests pass
- New features include test coverage
- Documentation is updated

---

**Latest Release:** Phase 4 Complete  
**Status:** ✅ Production Ready  
**Last Updated:** October 2025
