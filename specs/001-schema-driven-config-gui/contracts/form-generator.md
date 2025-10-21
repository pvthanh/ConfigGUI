# Component Contract: Form Generator

**Scope**: FormGenerator, WidgetFactory components  
**Responsibility**: Dynamically create GUI forms from JSON Schema, map schema types to Qt widgets

## Interface Definition

### Primary Component: FormGenerator

**Class**: `FormGenerator`  
**Location**: `src/ui/form_generator.h/cpp`  
**Parent**: `QWidget`

**Purpose**: Generate interactive form layout from JSONSchema, manage form widgets, handle field interactions

#### Public Methods

```cpp
class FormGenerator : public QWidget {
    Q_OBJECT
    
public:
    explicit FormGenerator(QWidget* parent = nullptr);
    
    // Initialize form from schema
    Result<void, GenerationError> generateForm(
        const JSONSchema& schema);
    
    // Populate form from configuration data
    Result<void, DataError> populateFromData(
        const ConfigurationData& data);
    
    // Extract current form values
    nlohmann::json getFormData() const;
    
    // Field access
    QWidget* getWidget(const QString& fieldPath) const;
    QVariant getFieldValue(const QString& fieldPath) const;
    void setFieldValue(
        const QString& fieldPath,
        const QVariant& value);
    
    // State queries
    bool isDirty() const;
    void markClean();
    
    // Validation display
    void showFieldError(
        const QString& fieldPath,
        const QString& message);
    void clearFieldError(const QString& fieldPath);
    void clearAllErrors();

signals:
    void fieldChanged(const QString& fieldPath, const QVariant& value);
    void formDirtyStatusChanged(bool isDirty);
    void validationRequested();
    
private:
    JSONSchema currentSchema_;
    QMap<QString, QWidget*> fieldWidgets_;
    FormState formState_;
    WidgetFactory widgetFactory_;
    QVBoxLayout* mainLayout_;
    bool isDirty_ = false;
    
    void connectWidgetSignals();
    void createFormLayout();
};
```

#### Input Contract

| Parameter | Type | Constraints | Error Behavior |
|-----------|------|-------------|----------------|
| schema | JSONSchema | Valid, contains fields | GenerationError::InvalidSchema |
| data | ConfigurationData | Matches schema | DataError::SchemaMismatch |
| fieldPath | QString | Dot-notation path | DataError::FieldNotFound |
| value | QVariant | Matches field type | DataError::TypeMismatch |

#### Output Contract

**Form Generation Returns**:
```cpp
// Success: Form widgets created and laid out
// Contains widgets for:
//   - All schema fields
//   - Grouped nested objects
//   - Validation display areas
//   - Help/description tooltips

// Error types:
enum class GenerationError {
    InvalidSchema,              // Schema structure invalid
    UnsupportedFieldType,       // Type mapping not available
    FieldCreationFailed,        // Widget creation failed
    LayoutCreationFailed        // Cannot arrange widgets
};
```

**Data Population Returns**:
```cpp
// Success: Form widgets populated with data values
// - Required fields validated present
// - Optional fields populated if data exists
// - Invalid values highlighted

// Error types:
enum class DataError {
    SchemaMismatch,             // Data schema doesn't match form schema
    FieldNotFound,              // Data field missing in schema
    TypeMismatch,               // Data type incompatible with schema
    ConstraintViolation         // Data violates field constraints
};
```

#### Widget Type Mapping

| JSON Schema Type | Qt Widget | Constraints Handled |
|-----------------|-----------|-------------------|
| string | QLineEdit | minLength, maxLength, pattern, regex validator |
| integer | QSpinBox | minimum, maximum, multipleOf |
| number | QDoubleSpinBox | minimum, maximum, multipleOf |
| boolean | QCheckBox | N/A |
| enum (string/integer) | QComboBox | populated with enum values |
| array (items with type) | QListWidget or QTableWidget | minItems, maxItems, delete/add buttons |
| object (nested) | QGroupBox with nested form | Recursive widget generation |

#### Layout Structure

```
QWidget (FormGenerator root)
└─ QVBoxLayout
   ├─ [Schema Title QLabel]
   ├─ QScrollArea
   │  └─ QWidget
   │     └─ QVBoxLayout
   │        ├─ Field Group 1 (Basic Fields)
   │        │  ├─ QLabel "fieldName *"
   │        │  ├─ QLineEdit (widget)
   │        │  └─ QLabel (description/error)
   │        │
   │        ├─ Field Group 2 (Nested Object)
   │        │  └─ QGroupBox "objectName"
   │        │     └─ QVBoxLayout
   │        │        ├─ nested field 1
   │        │        └─ nested field 2
   │        │
   │        └─ Field Group 3 (Arrays)
   │           └─ QGroupBox "arrayName"
   │              ├─ QListWidget or QTableWidget
   │              └─ [Add] [Remove] buttons
   │
   └─ QHBoxLayout (buttons)
      ├─ [Reset Button]
      └─ [Validate Button]
```

#### Example Usage

```cpp
// Create form generator
FormGenerator* formGen = new FormGenerator(this);
layout->addWidget(formGen);

// Load schema and generate form
JSONSchema schema = schemaLoader.loadSchema("app.schema.json").unwrap();
auto genResult = formGen->generateForm(schema);

if (!genResult.isOk()) {
    showError("Failed to generate form: " + genResult.unwrapErr().toString());
    return;
}

// Connect to changes
connect(formGen, &FormGenerator::fieldChanged,
        this, &MainWindow::onFieldChanged);

// Populate from existing config
ConfigurationData data = configLoader.load("app.json").unwrap();
auto popResult = formGen->populateFromData(data);

if (!popResult.isOk()) {
    // Some fields didn't populate, but form is displayed
    qWarning() << "Populate warning: " << popResult.unwrapErr().toString();
}

// User edits, then get values
connect(saveButton, &QPushButton::clicked, [this, formGen]() {
    nlohmann::json formData = formGen->getFormData();
    // Validate and save...
});
```

---

### Secondary Component: WidgetFactory

**Class**: `WidgetFactory`  
**Location**: `src/ui/widget_factory.h/cpp`

**Purpose**: Create individual Qt widgets based on schema field definitions

#### Public Methods

```cpp
class WidgetFactory {
public:
    // Create widget for field
    Result<QWidget*, WidgetError> createWidget(
        const QString& fieldPath,
        const nlohmann::json& fieldSchema,
        QWidget* parent = nullptr);
    
    // Get field type from schema
    FieldType getFieldType(const nlohmann::json& fieldSchema) const;
    
    // Apply constraints to widget
    void applyConstraints(
        QWidget* widget,
        const nlohmann::json& constraints);
    
    // Extract value from widget
    QVariant getWidgetValue(const QWidget* widget) const;
    
    // Set value to widget
    void setWidgetValue(
        QWidget* widget,
        const QVariant& value);
    
    // Register custom widget creator (for plugins)
    void registerCustomCreator(
        const QString& typeName,
        std::function<QWidget*(QWidget*)> creator);

private:
    // Individual widget creators
    QWidget* createStringWidget(
        const nlohmann::json& schema, 
        QWidget* parent);
    QWidget* createIntegerWidget(
        const nlohmann::json& schema,
        QWidget* parent);
    QWidget* createNumberWidget(
        const nlohmann::json& schema,
        QWidget* parent);
    QWidget* createBooleanWidget(
        const nlohmann::json& schema,
        QWidget* parent);
    QWidget* createEnumWidget(
        const nlohmann::json& schema,
        QWidget* parent);
    QWidget* createArrayWidget(
        const nlohmann::json& schema,
        QWidget* parent);
    
    // Custom creators map (for extensibility)
    QMap<QString, std::function<QWidget*(QWidget*)>> customCreators_;
};
```

#### Creation Details

##### String Widget
```cpp
// Input: {"type": "string", "minLength": 3, "maxLength": 50}
// Output: QLineEdit with validator

auto* lineEdit = new QLineEdit(parent);
if (schema.contains("minLength")) {
    // Set placeholder showing min length
}
if (schema.contains("maxLength")) {
    lineEdit->setMaxLength(maxLength);
}
if (schema.contains("pattern")) {
    lineEdit->setValidator(
        new QRegularExpressionValidator(...));
}
```

##### Numeric Widgets
```cpp
// Input: {"type": "integer", "minimum": 1024, "maximum": 65535}
// Output: QSpinBox with range limits

auto* spinBox = new QSpinBox(parent);
spinBox->setMinimum(minimum);
spinBox->setMaximum(maximum);
if (schema.contains("multipleOf")) {
    spinBox->setSingleStep(multipleOf);
}
```

##### Enum Widget
```cpp
// Input: {"type": "string", "enum": ["option1", "option2"]}
// Output: QComboBox populated with options

auto* comboBox = new QComboBox(parent);
for (const auto& option : schema["enum"]) {
    comboBox->addItem(QString::fromStdString(option));
}
```

##### Array Widget
```cpp
// Input: {"type": "array", "items": {"type": "string"}}
// Output: QListWidget with add/remove buttons

auto* container = new QWidget(parent);
auto* layout = new QVBoxLayout(container);
auto* listWidget = new QListWidget();
layout->addWidget(listWidget);

auto* addBtn = new QPushButton("Add");
auto* removeBtn = new QPushButton("Remove");
layout->addWidget(addBtn);
layout->addWidget(removeBtn);
```

#### Error Handling

```cpp
enum class WidgetError {
    UnsupportedType,        // Type not mapped to widget
    InvalidSchema,          // Schema structure invalid
    ConstraintError,        // Cannot apply constraint
    CreationFailed          // Widget instantiation failed
};
```

---

## Signals & Slots

### FormGenerator Signals

| Signal | When Fired | Data |
|--------|-----------|------|
| fieldChanged | User edits any field | (fieldPath, newValue) |
| formDirtyStatusChanged | Form marked dirty/clean | (isDirty: bool) |
| validationRequested | User clicks Validate button | (none) |

### Widget Connections

```cpp
// Each created widget connected to FormGenerator
connect(widget, &QLineEdit::textChanged,
        this, [this, fieldPath](const QString& text) {
            emit fieldChanged(fieldPath, QVariant(text));
        });

connect(widget, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this, fieldPath](int value) {
            emit fieldChanged(fieldPath, QVariant(value));
        });
```

---

## Real-Time Validation Integration

### Form Generation → Validation Flow

```cpp
// In FormGenerator slot
void FormGenerator::onFieldChanged(
    const QString& fieldPath,
    const QVariant& value) {
    
    // 1. Mark form dirty
    isDirty_ = true;
    emit formDirtyStatusChanged(true);
    
    // 2. Emit signal (triggers validator in parent)
    emit fieldChanged(fieldPath, value);
    
    // 3. Validator returns error → call showFieldError()
}

// In MainWindow slot
void MainWindow::onFieldChanged(
    const QString& fieldPath,
    const QVariant& value) {
    
    // Validate field
    auto result = validator_.validateField(fieldPath, value);
    
    if (!result.isValid) {
        formGen->showFieldError(
            fieldPath,
            result.errors[0].message);
    } else {
        formGen->clearFieldError(fieldPath);
    }
}
```

---

## Testing Contract

### Unit Tests

- ✅ generateForm with simple schema → widgets created
- ✅ generateForm with nested objects → grouped widgets
- ✅ generateForm with arrays → list widgets
- ✅ populateFromData → form populated correctly
- ✅ getFormData → returns current values
- ✅ setFieldValue → updates widget and marks dirty
- ✅ Widget factory maps all schema types correctly
- ✅ Constraints applied to widgets (validators, ranges)

### Integration Tests

- ✅ Generate form → Populate → Get data → Matches original
- ✅ User edits field → fieldChanged signal emitted
- ✅ Show/clear errors → Error UI displayed correctly
- ✅ Large schema with 100 fields → rendered in <2 seconds

---

## Performance Considerations

### Optimization Strategies

1. **Lazy Widget Creation** (for large forms)
   - Create widgets in visible area first
   - Create others as user scrolls (if needed)

2. **Signal Debouncing**
   - Don't validate on every keystroke
   - Use timer to batch changes (100ms)

3. **Cached Widget References**
   - Store widget map for O(1) field lookup
   - Don't traverse layout on each access

### Performance Targets

| Operation | Target |
|-----------|--------|
| Generate form (100 fields) | <2 seconds |
| Populate from data | <500ms |
| Extract form data | <100ms |
| Show/clear error | <10ms |

---

## Contract Version

**Version**: 1.0  
**Last Updated**: 2025-10-20  
**Status**: Ready for Implementation

