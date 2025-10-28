# ConfigGUI Software Design Document

**Version**: 2.0  
**Date**: October 28, 2025  
**Status**: ✅ Production Ready  
**Last Updated**: Phase 2D Complete (INI Support, Multi-Format Save)

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [System Architecture](#system-architecture)
3. [Class Diagrams](#class-diagrams)
4. [Component Architecture](#component-architecture)
5. [Design Patterns](#design-patterns)
6. [Data Flow](#data-flow)
7. [Module Dependencies](#module-dependencies)
8. [Technology Stack](#technology-stack)

---

## Executive Summary

ConfigGUI is a dual-target configuration management system that provides both desktop (Qt GUI) and web (HTTP Server) interfaces for schema-driven form generation and configuration file management.

**Key Architectural Principles:**
- **Qt-Free Core**: Core library has zero Qt dependencies, enabling code reuse
- **Modular Design**: Separate concerns across core, I/O, UI, and validators
- **Consistent Patterns**: Result<T> for error handling, static reader/writer classes
- **Multi-Format Support**: JSON, YAML, INI file handling
- **Hybrid Storage**: Local PC + Server backup for configurations

---

## System Architecture

### Three-Layer Architecture Diagram

```mermaid
graph TD
    subgraph UI["User Interface Layer"]
        QtGUI["Qt GUI Application<br/>(main_window, forms)"]
        HtmlUI["HTML/JavaScript<br/>(index.html, main.js)"]
    end
    
    subgraph Business["Business Logic Layer"]
        FormGen["Form Generator<br/>(schema → UI)"]
        Validation["Validation Engine<br/>(JSON Schema)"]
        StateManage["State Manager<br/>(AppState, FormState)"]
    end
    
    subgraph Core["Core Library (ConfigGUICore)<br/>No Qt Dependencies"]
        Schema["Schema<br/>(definition & loading)"]
        DataConfig["ConfigurationData<br/>(value storage)"]
        Validators["Validators<br/>(business rules)"]
        IO["I/O Operations"]
        subgraph Readers["Readers"]
            JsonR["JsonReader"]
            YamlR["YamlReader"]
            IniR["INIReader"]
        end
        subgraph Writers["Writers"]
            JsonW["JsonWriter"]
            YamlW["YamlWriter"]
            IniW["INIWriter"]
        end
        IO --> Readers
        IO --> Writers
        ErrorHandle["Result&lt;T, E&gt;<br/>Error Handler"]
    end
    
    subgraph Deps["External Dependencies"]
        NlJson["nlohmann/json"]
        JsonSchema["json-schema-validator"]
        Yaml["yaml-cpp"]
        HttpLib["cpp-httplib<br/>(HTTP Server only)"]
    end
    
    QtGUI --> Business
    HtmlUI --> Business
    Business --> Core
    Core --> Deps
    
    style Core fill:#e1f5ff
    style UI fill:#fff3e0
    style Business fill:#f3e5f5
    style Deps fill:#fce4ec
```

### Dual-Target Build Configuration

```mermaid
graph LR
    CMake["CMakeLists.txt<br/>Root Configuration"]
    
    CMake -->|BUILD_QT_APP=ON| QtBuild["Qt Application Build"]
    CMake -->|BUILD_HTML_SERVER=ON| HttpBuild["HTTP Server Build"]
    CMake -->|Always| CoreBuild["ConfigGUICore<br/>Static Library"]
    
    QtBuild -->|Links| CoreLib["ConfigGUICore"]
    QtBuild -->|Qt6 Deps| Qt["Qt6 Core/Gui/Widgets"]
    QtBuild -->|Produces| ConfigGUI["ConfigGUI Executable"]
    
    HttpBuild -->|Links| CoreLib
    HttpBuild -->|Header-only| CppHttp["cpp-httplib"]
    HttpBuild -->|Produces| ConfigServer["ConfigGUIServer Executable"]
    
    CoreBuild -->|Always Built| CoreLib
    CoreLib -->|Exports| PublicAPI["Public API<br/>Schema, I/O, Validation"]
    
    style CMake fill:#fff9c4
    style QtBuild fill:#c8e6c9
    style HttpBuild fill:#b3e5fc
    style CoreBuild fill:#e1bee7
    style ConfigGUI fill:#a5d6a7
    style ConfigServer fill:#81d4fa
```

---

## Class Diagrams

### Core I/O Class Hierarchy

```mermaid
classDiagram
    direction TB
    
    class IReader {
        <<interface>>
        +readFile(path) Result~json~
        +readString(content) Result~json~
    }
    
    class IWriter {
        <<interface>>
        +writeFile(path, data) VoidResult
        +toString(data) StringResult
    }
    
    class JsonReader {
        -parseJson(content)
        +readFile(path) Result~json~*
        +readString(content) Result~json~*
    }
    
    class JsonWriter {
        +writeFile(path, data) VoidResult*
        +toString(data) StringResult*
    }
    
    class YamlReader {
        -parseYaml(content)
        +readFile(path) Result~json~*
        +readString(content) Result~json~*
    }
    
    class YamlWriter {
        +writeFile(path, data) VoidResult*
        +toString(data) StringResult*
    }
    
    class INIParser {
        -current_section: string
        -parseContext: ParseContext
        +parse(ini_content)$ Result~json~
        +parseFile(file_path)$ Result~json~
        -isSectionHeader(line)$ bool
        -parseKeyValue(line)$ Pair~string,string~
        -inferType(value)$ json
    }
    
    class INIReader {
        +readFile(path) Result~json~*
        +readString(content) Result~json~*
    }
    
    class INIWriter {
        +writeFile(path, data) VoidResult*
        +toString(data) StringResult*
    }
    
    class Result {
        -value: T
        -error: E
        +is_success() bool
        +is_failure() bool
        +value() T
        +error() E
    }
    
    IReader <|-- JsonReader
    IReader <|-- YamlReader
    IReader <|-- INIReader
    IWriter <|-- JsonWriter
    IWriter <|-- YamlWriter
    IWriter <|-- INIWriter
    
    INIParser --* INIReader
    JsonReader ..> Result
    YamlReader ..> Result
    INIReader ..> Result
    JsonWriter ..> Result
    YamlWriter ..> Result
    INIWriter ..> Result
```

### Schema and Validation Architecture

```mermaid
classDiagram
    direction TB
    
    class Schema {
        -id: string
        -name: string
        -description: string
        -_fullSchema: json
        +getProperty(name) Property
        +validate(data) Result~bool~
    }
    
    class SchemaLoader {
        +loadFromFile(path) Result~Schema~$
        +loadFromString(content) Result~Schema~$
        +validateSchema(schema) Result~bool~$
    }
    
    class SchemaValidator {
        -schema: Schema
        -validator: json_schema_validator
        +validate(data) Result~bool~
        +getErrors() vector~ValidationError~
    }
    
    class ValidationError {
        -message: string
        -field: string
        -line: int
        -column: int
        +toString() string
    }
    
    class ConfigurationData {
        -schemaId: string
        -data: json
        -state: FormState
        +getValue(key) json
        +setValue(key, value) void
        +getNestedValue(path) json
        +setNestedValue(path, value) void
        +toJson() json
    }
    
    class FormState {
        -isDirty: bool
        -errors: map~string, ValidationError~
        -isValid: bool
        +markDirty() void
        +clearErrors() void
        +addError(field, error) void
        +isFieldValid(field) bool
    }
    
    Schema --> SchemaValidator
    SchemaValidator --> ValidationError
    SchemaLoader --> Schema
    ConfigurationData --> FormState
    ConfigurationData --> Schema
```

### Qt GUI Architecture

```mermaid
classDiagram
    direction TB
    
    class MainWindow {
        -schema: Schema
        -config: ConfigurationData
        -formLayout: QVBoxLayout
        -actionLoad: QAction
        -actionSave: QAction
        +loadSchemaFile(path) void
        +loadConfigurationFile(path) void
        +saveConfiguration() void
        +generateForm() void
        -populateFormFromSchema() void
        -collectFormData() json
    }
    
    class FormFieldFactory {
        +createField(name, schema) QWidget*$
        +createTextField(name, schema) QLineEdit*$
        +createNumberField(name, schema) QSpinBox*$
        +createBoolField(name, schema) QCheckBox*$
        +createArrayField(name, schema) QWidget*$
        +createObjectField(name, schema) QWidget*$
    }
    
    class FileIOManager {
        +detectFormat(path) string$
        +load(path) Result~json~$
        +save(path, data, format) VoidResult$
        -loadJson(path) Result~json~$
        -loadYaml(path) Result~json~$
        -loadIni(path) Result~json~$
        -saveJson(path, data) VoidResult$
        -saveYaml(path, data) VoidResult$
        -saveIni(path, data) VoidResult$
    }
    
    class ValidationManager {
        -validator: SchemaValidator
        +validate(data) Result~bool~
        +getFieldErrors(field) vector~string~
        +clearErrors() void
    }
    
    MainWindow --> FormFieldFactory
    MainWindow --> FileIOManager
    MainWindow --> ValidationManager
    FileIOManager --> INIReader
    FileIOManager --> YamlReader
    FileIOManager --> JsonReader
```

### HTML/JavaScript Architecture

```mermaid
classDiagram
    direction TB
    
    class AppState {
        -currentState: string
        -selectedSchema: Schema
        -selectedSchemaData: json
        -selectedConfig: json
        -formData: json
        -configs: Config[]*
        +setState(state) void
        +addFormData(field, value) void
        +clearFormData() void
        +toPayload() json
    }
    
    class FormGenerator {
        +renderForm(schema, container) void$
        +createFormField(name, schema) HTMLElement$
        +createArrayField(name, schema) HTMLElement$
        +createObjectField(name, schema) HTMLElement$
        +collectFormData() json$
    }
    
    class FileHandler {
        +handleFileUpload(file) Promise~void~$
        +handleConfigUpload(file) Promise~void~$
        +detectFormat(filename) string$
        +parseContent(content, format) json$
    }
    
    class SaveManager {
        +saveConfiguration(format) Promise~void~$
        +showFormatDialog(formData) void$
        +showFilenameDialog(format) void$
        +saveToLocalPC(data, filename, format) Promise~File~$
        +downloadFile(data, filename, format) void$
        +saveToServerBackup(data, filename, format) Promise~void~$
    }
    
    class HttpClient {
        +loadSchemas() Promise~Schema[]~$
        +loadConfigs() Promise~Config[]~$
        +uploadSchema(file) Promise~Schema~$
        +uploadConfig(file) Promise~Config~$
        +saveConfig(data, filename) Promise~Response~$
        +deleteConfig(filename) Promise~void~$
    }
    
    AppState --> FormGenerator
    FormGenerator --> FileHandler
    SaveManager --> HttpClient
    FileHandler --> SaveManager
```

---

## Component Architecture

### Module Organization

```mermaid
graph TB
    subgraph src["Source Code Organization"]
        subgraph core["src/core/ (Qt-Free)"]
            schema["schema/"]
            data["data/"]
            validators["validators/"]
            io["io/"]
            utils["utils/"]
        end
        
        subgraph qt["src/qt/ (Qt-Only)"]
            mainwin["main_window.cpp"]
            qtmain["main.cpp"]
        end
        
        subgraph ui["src/ui/ (Qt Widgets)"]
            forms["form widgets"]
            dialogs["dialogs"]
        end
        
        subgraph html["src/html/ (HTTP Server)"]
            server["server/"]
            handlers["handlers/"]
            assets["assets/"]
        end
        
        subgraph validators["src/validators/"]
            rules["validation_rules/"]
        end
        
        subgraph io2["src/io/"]
            readers["readers (deprecated→core)"]
            writers["writers (deprecated→core)"]
        end
    end
    
    subgraph tests["tests/ Organization"]
        unit["unit/"]
        integration["integration/"]
        html_tests["html/"]
    end
    
    subgraph resources["resources/"]
        configs["configs/"]
        schemas["schemas/"]
        html_assets["html/"]
    end
    
    qt -->|Links| core
    html -->|Links| core
    ui -->|Qt Widgets| core
    
    style core fill:#e1f5ff
    style qt fill:#c8e6c9
    style html fill:#b3e5fc
    style ui fill:#fff9c4
```

### File Format Support Pipeline

```mermaid
graph LR
    subgraph Input["Input"]
        FileUpload["File Upload<br/>.json/.yaml/.ini"]
        TextInput["Text Input<br/>Raw Content"]
    end
    
    subgraph Detection["Format Detection"]
        ExtCheck["Check Extension"]
        MimeCheck["Check MIME Type"]
        ContentCheck["Analyze Content"]
    end
    
    subgraph Parsing["Parse by Format"]
        JsonParse["JSON Parser<br/>(nlohmann/json)"]
        YamlParse["YAML Parser<br/>(yaml-cpp)"]
        IniParse["INI Parser<br/>(Custom)"]
    end
    
    subgraph Validation["Validation"]
        SchemaVal["Schema Validation<br/>(JSON Schema)"]
        ErrorHandle["Error Handling<br/>(Result&lt;T&gt;)"]
    end
    
    subgraph Output["Output Format"]
        JsonOut["JSON Export"]
        YamlOut["YAML Export"]
        IniOut["INI Export"]
    end
    
    Input --> Detection
    Detection --> Parsing
    Parsing --> Validation
    Validation -->|Success| Output
    Validation -->|Failure| ErrorHandle
    ErrorHandle -->|User| Input
    
    style Input fill:#fff3e0
    style Detection fill:#ffe0b2
    style Parsing fill:#ffcc80
    style Validation fill:#ffd54f
    style Output fill:#fff59d
```

---

## Design Patterns

### 1. Result<T, E> Pattern - Error Handling

**Pattern Type**: Functional Error Handling  
**Purpose**: Replace exceptions with Result objects carrying success/failure information

```cpp
// Definition
template<typename T, typename E = std::string>
class Result {
    std::optional<T> value_;
    std::optional<E> error_;
public:
    bool is_success() const { return value_.has_value(); }
    bool is_failure() const { return error_.has_value(); }
    T value() const { return value_.value(); }
    E error() const { return error_.value(); }
};

// Usage
Result<json> result = INIReader::readFile("config.ini");
if (result.is_success()) {
    json config = result.value();
    // Process config
} else {
    std::cerr << "Error: " << result.error() << std::endl;
}
```

**Benefits:**
- ✅ No exception overhead
- ✅ Explicit error handling
- ✅ Composable error chains
- ✅ MISRA C++ compliant

### 2. Static Reader/Writer Pattern

**Pattern Type**: Factory Method + Facade  
**Purpose**: Provide consistent interface across different file formats

```cpp
// Interface
class IReader {
public:
    virtual Result<json> readFile(const std::string& path) = 0;
    virtual Result<json> readString(const std::string& content) = 0;
};

// Concrete implementations
class JsonReader : public IReader { /* ... */ };
class YamlReader : public IReader { /* ... */ };
class INIReader : public IReader { /* ... */ };

// Usage
Result<json> json_data = JsonReader::readFile("config.json");
Result<json> yaml_data = YamlReader::readFile("config.yaml");
Result<json> ini_data = INIReader::readFile("config.ini");
```

**Benefits:**
- ✅ Polymorphic file handling
- ✅ Extensible for new formats
- ✅ Same API for all readers

### 3. Schema-Driven Form Generation

**Pattern Type**: Model-View Pattern  
**Purpose**: Dynamically generate UI from schema definitions

```
Schema (Model)
    ↓
Form Generator (Controller)
    ↓
UI Widgets/HTML Elements (View)
    ↓
User Input
    ↓
Form Data (JSON)
    ↓
Validation (against Schema)
    ↓
Save/Export
```

**Benefits:**
- ✅ Single source of truth (schema)
- ✅ Automatic UI generation
- ✅ Built-in validation
- ✅ Consistent across platforms

### 4. Hybrid Storage Pattern

**Pattern Type**: Dual-Destination Write  
**Purpose**: Save configurations to both local PC and server simultaneously

```
User Clicks Save
    ↓
Format Selection Dialog (JSON/YAML/INI)
    ↓
Filename Selection Dialog
    ↓
Parallel Save Operations:
    ├─→ Local PC (FileSystem Access API / Browser Download)
    └─→ Server (HTTP POST /api/config)
    ↓
Success Confirmation
```

**Benefits:**
- ✅ Local backup on user device
- ✅ Server-side version control
- ✅ Cross-device synchronization
- ✅ Offline capability

### 5. Qt-Free Core Pattern

**Pattern Type**: Layered Architecture  
**Purpose**: Isolate business logic from UI framework

```
Application Layer
    (Qt GUI | HTML+JS)
        ↓ (depends on)
Business Logic Layer
    (Form Generation, Validation, State)
        ↓ (depends on)
Core Library
    (Schema, I/O, Validation) ← NO Qt!
        ↓ (depends on)
External Libraries
    (nlohmann/json, yaml-cpp, json-schema-validator)
```

**Benefits:**
- ✅ Code reuse across platforms
- ✅ Easy testing without GUI
- ✅ HTTP server without Qt overhead
- ✅ Docker deployment friendly

### 6. State Management Pattern (AppState)

**Pattern Type**: Singleton State Container  
**Purpose**: Centralize application state management for form operations

```mermaid
graph LR
    UI["UI Events<br/>(click, change, submit)"]
    
    UI -->|Dispatch Action| State["AppState<br/>Singleton"]
    
    State -->|Update| Data["State Fields<br/>- currentState<br/>- selectedSchema<br/>- formData"]
    
    Data -->|Notify| Listeners["Event Listeners<br/>- Form Render<br/>- Button Enable/Disable<br/>- Toast Messages"]
    
    Listeners -->|Update| UI
    
    style State fill:#c8e6c9
```

---

## Data Flow

### Configuration Loading Flow

```mermaid
sequenceDiagram
    participant User as User
    participant UI as UI (Qt/HTML)
    participant FileIO as File I/O
    participant Parser as Format Parser
    participant Schema as Schema Validator
    participant Form as Form Generator
    participant Storage as Storage

    User->>UI: Click "Load Config"
    UI->>UI: Show file dialog
    User->>UI: Select config.ini
    UI->>FileIO: Load file
    FileIO->>Parser: Detect INI format
    Parser->>Parser: Parse INI content
    Parser-->>FileIO: JSON structure
    FileIO->>Schema: Validate against schema
    Schema-->>FileIO: Valid ✓
    FileIO-->>UI: json data
    UI->>Form: Generate form
    Form->>Form: Populate fields
    Form-->>UI: Render HTML/Widgets
    UI-->>User: Display pre-filled form
    
    Note over Storage: Config stored in<br/>AppState.selectedConfigData
```

### Configuration Saving Flow (Hybrid Storage)

```mermaid
sequenceDiagram
    participant User as User
    participant UI as UI
    participant Dialog as Dialog Manager
    participant Validator as Validator
    participant LocalSave as Local PC
    participant ServerSave as Server API
    participant Confirm as Confirmation

    User->>UI: Click "Save"
    UI->>Dialog: Show format selector
    User->>Dialog: Select INI format
    Dialog->>Dialog: Show filename dialog
    User->>Dialog: Enter "app.ini"
    
    rect rgb(200, 220, 200)
        Note over Dialog,ServerSave: Parallel Save Operations
        
        par Local Save
            Dialog->>Validator: Validate data
            Validator-->>Dialog: Valid ✓
            Dialog->>LocalSave: Save to PC (FileSystem API)
            LocalSave-->>Dialog: File saved
        and Server Backup
            Dialog->>ServerSave: POST /api/config
            ServerSave->>ServerSave: Save "app.ini"
            ServerSave-->>Dialog: 200 OK
        end
    end
    
    Dialog->>Confirm: Both successful
    Confirm-->>User: "Saved to PC and Server"
```

### Multi-Format Support Data Flow

```mermaid
graph TB
    subgraph Input["Input Processing"]
        File["Config File<br/>.json/.yaml/.ini"]
        Text["Raw Text<br/>Content"]
    end
    
    subgraph Detection["Format Detection"]
        Ext["Extension Check<br/>.json → JSON<br/>.yaml → YAML<br/>.ini → INI"]
        MIME["MIME Type<br/>application/json<br/>text/yaml<br/>text/plain"]
        Content["Content Analysis<br/>Look for markers:<br/>[section] → INI<br/>{ → JSON<br/>--- → YAML"]
    end
    
    subgraph Parsing["Format-Specific Parsing"]
        JsonP["JsonReader<br/>nlohmann::json"]
        YamlP["YamlReader<br/>yaml-cpp"]
        IniP["INIReader<br/>INIParser"]
    end
    
    subgraph Processing["Data Processing"]
        Norm["Normalize to JSON<br/>Standard format"]
        Valid["Schema Validation<br/>Verify structure"]
        Store["Store in AppState<br/>Ready for form"]
    end
    
    subgraph Output["Output Support"]
        JsonExp["Export as JSON<br/>Using nlohmann"]
        YamlExp["Export as YAML<br/>Using yaml-cpp"]
        IniExp["Export as INI<br/>Using INIWriter"]
    end
    
    File --> Detection
    Text --> Detection
    
    Ext --> Parsing
    MIME --> Parsing
    Content --> Parsing
    
    Parsing --> JsonP
    Parsing --> YamlP
    Parsing --> IniP
    
    JsonP --> Processing
    YamlP --> Processing
    IniP --> Processing
    
    Processing --> Norm
    Norm --> Valid
    Valid --> Store
    
    Store --> Output
    
    style Input fill:#fff3e0
    style Detection fill:#ffe0b2
    style Parsing fill:#ffcc80
    style Processing fill:#ffd54f
    style Output fill:#fff59d
```

---

## Module Dependencies

### Dependency Graph

```mermaid
graph TB
    subgraph External["External Libraries"]
        NlJson["nlohmann/json"]
        JsonSch["json-schema-validator"]
        Yaml["yaml-cpp"]
        GTest["Google Test<br/>(tests only)"]
        Qt["Qt6<br/>(Qt app only)"]
        CppHttp["cpp-httplib<br/>(server only)"]
    end
    
    subgraph Core["Core Library (ConfigGUICore)"]
        Result["result.h<br/>Error handling"]
        Errors["error_types.h<br/>Error definitions"]
        Schema["schema.h/cpp<br/>Schema definition"]
        SchemaLoader["schema_loader.h/cpp<br/>Load schemas"]
        ConfigData["configuration_data.h/cpp<br/>Value storage"]
        Validator["schema_validator.h/cpp<br/>Validation logic"]
        FormState["form_state.h/cpp<br/>Form tracking"]
        
        JsonReader["json_reader.h/cpp"]
        YamlReader["yaml_reader.h/cpp"]
        IniParser["ini_parser.h/cpp"]
        IniReader["ini_reader.h/cpp"]
        IniWriter["ini_writer.h/cpp"]
        JsonWriter["json_writer.h/cpp"]
        YamlWriter["yaml_writer.h/cpp"]
    end
    
    subgraph QtApp["Qt Application (ConfigGUI)"]
        MainWindow["main_window.cpp"]
        QtMain["main.cpp"]
        UIWidgets["ui/ widgets"]
    end
    
    subgraph HttpServer["HTTP Server (ConfigGUIServer)"]
        Server["server.cpp"]
        Handlers["handlers/"]
        Assets["HTML/CSS/JS"]
    end
    
    subgraph Tests["Test Suite"]
        UnitTests["unit/"]
        IntegTests["integration/"]
        HtmlTests["html/"]
    end
    
    %% External dependencies
    NlJson --> Schema
    NlJson --> ConfigData
    NlJson --> JsonReader
    NlJson --> IniParser
    
    JsonSch --> Validator
    
    Yaml --> YamlReader
    
    Qt --> MainWindow
    
    CppHttp --> Server
    
    %% Core dependencies
    Result --> Errors
    Result --> Schema
    Result --> ConfigData
    Result --> Validator
    
    Schema --> SchemaLoader
    
    ConfigData --> FormState
    Validator --> FormState
    
    JsonReader --> ConfigData
    YamlReader --> ConfigData
    IniReader --> ConfigData
    
    IniParser --> IniReader
    IniParser --> IniWriter
    
    %% Application dependencies
    MainWindow --> Schema
    MainWindow --> ConfigData
    MainWindow --> JsonReader
    MainWindow --> YamlReader
    MainWindow --> IniReader
    UIWidgets --> MainWindow
    
    %% Server dependencies
    Server --> Schema
    Server --> ConfigData
    Server --> JsonReader
    Server --> YamlReader
    Server --> IniReader
    Handlers --> Server
    Assets --> Handlers
    
    %% Test dependencies
    UnitTests --> Core
    IntegTests --> QtApp
    HtmlTests --> HttpServer
    GTest --> Tests
    
    style Core fill:#e1f5ff
    style QtApp fill:#c8e6c9
    style HttpServer fill:#b3e5fc
    style External fill:#fce4ec
    style Tests fill:#f3e5f5
```

---

## Technology Stack

### Core Technologies

| Component | Technology | Version | Purpose |
|-----------|-----------|---------|---------|
| **Language** | C++ | C++17 | Core business logic |
| **JSON** | nlohmann/json | Latest | JSON parsing/serialization |
| **YAML** | yaml-cpp | 0.7.0+ | YAML parsing/serialization |
| **INI** | Custom Parser | - | INI parsing/serialization |
| **Schema Validation** | json-schema-validator | Latest | JSON Schema validation |
| **Error Handling** | Result<T> | Custom | Exception-free error handling |

### GUI Technologies

| Component | Technology | Version | Purpose |
|-----------|-----------|---------|---------|
| **Framework** | Qt | 6.x | Desktop GUI application |
| **Build System** | CMake | 3.16+ | Cross-platform building |
| **Testing** | Google Test | 1.11+ | Unit and integration tests |

### Web Technologies

| Component | Technology | Version | Purpose |
|-----------|-----------|---------|---------|
| **HTTP Server** | cpp-httplib | 0.11.0+ | Lightweight HTTP server |
| **Frontend** | HTML5 | - | Web interface markup |
| **Styling** | CSS3 | - | UI styling |
| **Scripting** | JavaScript (ES6) | - | Dynamic form generation |
| **Storage** | FileSystem API | HTML5 | Local file access (browser) |

### Development Tools

| Tool | Version | Purpose |
|------|---------|---------|
| **Compiler** | GCC 9+, Clang 10+, MSVC 2019+ | C++ compilation |
| **Build** | CMake | Build configuration |
| **Testing** | ctest | Test execution |
| **Code Style** | clang-format | Code formatting |
| **Linting** | clang-tidy | Static analysis |
| **Containerization** | Docker | Deployment |

---

## Summary

This design document provides a comprehensive overview of ConfigGUI's architecture:

- **Three-layer architecture** ensures separation of concerns
- **Qt-free core** enables platform independence
- **Design patterns** provide consistent, maintainable code
- **Multi-format support** handles JSON, YAML, and INI
- **Dual-target build** supports both desktop and web deployment
- **Result<T> pattern** ensures robust error handling

The system is production-ready with comprehensive test coverage, proper validation, and multiple deployment options.

