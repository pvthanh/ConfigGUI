# Simplified Rules Format Design

## Current Format (Complex, Verbose)
```json
"rules": [
  {
    "name": "type",
    "type": "string"
  },
  {
    "name": "link id",
    "type": "integer",
    "minimum": 0
  },
  {
    "name": "latitude",
    "type": "float",
    "minimum": -90,
    "maximum": 90
  },
  {
    "name": "node type",
    "type": "string",
    "allowEmpty": false,
    "enum": ["intersection", "termination of a link", ...]
  }
]
```

## Desired Format (Simplified, Shorthand)
```json
"rules": {
  "type": "string",
  "link id": "integer[0,]+",
  "latitude": "float[-90,90]",
  "longitude": "float[-180,180]",
  "node type": "string{required,enum:intersection|termination|change|shape}",
  "shape point": "string?",
  "aligned": "boolean",
  "address scheme": "string{optional,enum:Even|Odd|Mixed}",
  "functional class": "integer[1,5]",
  "phone number": "string{optional,pattern:^\\+?[0-9 \\-/()]{3,}$}",
  "open 24 hours": "boolean?"
}
```

## Shorthand Syntax Rules

### Type Specifications

1. **Basic Type + Optional Marker**
   - `"string"` - required string, no constraints
   - `"string?"` - optional string (allowEmpty: true)
   - `"integer"` - required integer
   - `"integer?"` - optional integer
   - `"float"` - required float
   - `"float?"` - optional float
   - `"boolean"` - required boolean
   - `"boolean?"` - optional boolean

2. **With Range Constraints**
   - `"integer[0,]"` - integer, minimum 0, no maximum
   - `"integer[1,5]"` - integer, minimum 1, maximum 5
   - `"float[-90,90]"` - float, minimum -90, maximum 90
   - `"float[-180,180]"` - float, minimum -180, maximum 180

3. **With Modifiers (Curly Braces)**
   - `"string{required}"` - explicitly required
   - `"string{optional}"` - explicitly optional (allowEmpty: true)
   - `"string{required,enum:val1|val2|val3}"` - enum values
   - `"string{optional,pattern:regex}"` - regex pattern
   - Combined: `"string{optional,enum:A|B|C}"`

### Parsing Examples

| Input | Parsed |
|-------|--------|
| `"string"` | type: string, required: true |
| `"string?"` | type: string, required: false |
| `"integer[0,]"` | type: integer, required: true, minimum: 0 |
| `"float[-90,90]"` | type: float, required: true, minimum: -90, maximum: 90 |
| `"string{required,enum:A\|B}"` | type: string, required: true, enum: ["A", "B"] |
| `"string{optional,pattern:.*\.json$}"` | type: string, required: false, pattern: ".*\.json$" |

## UI Widget Generation Flow

```
User View (Simplified)
├─ Rule Name: [text input]
├─ Type Selector: [dropdown: string|integer|float|boolean]
│
├─ If TYPE = STRING (show conditionally):
│  ├─ Allow Empty: [checkbox]
│  ├─ Pattern: [text input - optional]
│  └─ Enum Values: [array widget - optional]
│
├─ If TYPE = INTEGER/FLOAT (show conditionally):
│  ├─ Minimum: [number input]
│  └─ Maximum: [number input]
│
└─ If TYPE = BOOLEAN:
   └─ (no additional options)

User clicks SAVE → Converts to shorthand → Saves to rules{} object
```

## Storage Format Mapping

### String Type
**Form → Shorthand:**
- name: "type", type: "string" → `"type": "string"`
- name: "shape point", type: "string" → `"shape point": "string?"`

**Shorthand → Form (on load):**
- `"type": "string"` → name: "type", type: "string"
- `"shape point": "string?"` → name: "shape point", type: "string" (allowEmpty: true)

### Integer Type with Range
**Form → Shorthand:**
- name: "link id", type: "integer", minimum: 0 → `"link id": "integer[0,]+"`
- name: "functional class", type: "integer", minimum: 1, maximum: 5 → `"functional class": "integer[1,5]"`

### Float Type with Range
**Form → Shorthand:**
- name: "latitude", type: "float", minimum: -90, maximum: 90 → `"latitude": "float[-90,90]"`

### String with Enum
**Form → Shorthand:**
- name: "node type", type: "string", enum: ["intersection", "termination", "change"] → 
  `"node type": "string{required,enum:intersection|termination|change}"`

### String with Pattern
**Form → Shorthand:**
- name: "phone number", type: "string", pattern: "^\\+?[0-9 \\-/()]{3,}$" →
  `"phone number": "string{optional,pattern:^\\+?[0-9 \\-/()]{3,}$}"`

## Implementation Strategy

### Phase 1: Schema Update
- Add new simplified format to schema
- Keep backward compatibility with old format
- Auto-detect format on load

### Phase 2: Parser/Serializer
- Create parser: shorthand string → structured object
- Create serializer: structured object → shorthand string

### Phase 3: UI Widget
- Create RuleEditorWidget for key-value editing
- Dynamic widget generation based on type
- Real-time preview of shorthand format

### Phase 4: Integration
- Update ObjectArrayWidget or replace with new widget
- Ensure save/load preserves shorthand format
- Test with all rule types

## Benefits
✅ Simpler visual representation
✅ Easier for users to understand constraints
✅ Compact storage format
✅ Type-driven UI generation
✅ Backward compatible schema
