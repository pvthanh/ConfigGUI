/**
 * ConfigGUI Main JavaScript - Dual-Button Interface with Schema & Config Loading
 * Handles UI state management, schema/config loading, form generation, and saving
 */

console.log('[INIT] JavaScript file loading...');

// ============================================================================
// Global State Management
// ============================================================================

console.log('[INIT] Defining AppState...');

const AppState = {
    schemas: [],
    configs: [],
    selectedSchema: null,
    selectedSchemaData: null,
    selectedConfigData: null,
    loadedConfigFile: null,  // Track loaded config file path for overwrite option
    loadedConfigFilename: null,  // Track filename for reference
    formData: {},
    isLoading: false,
    currentState: 'loading', // loading, main-menu, schema-selector, config-selector, form-generator, error

    setState(newState) {
        this.currentState = newState;
        renderUI();
    },

    addFormData(key, value) {
        this.formData[key] = value;
    },

    clearFormData() {
        this.formData = {};
    },

    setLoadedConfig(filepath, filename) {
        this.loadedConfigFile = filepath;
        this.loadedConfigFilename = filename;
    },

    clearLoadedConfig() {
        this.loadedConfigFile = null;
        this.loadedConfigFilename = null;
    },

    resetSelection() {
        this.selectedSchema = null;
        this.selectedSchemaData = null;
        this.selectedConfigData = null;
        this.clearFormData();
        this.clearLoadedConfig();
    }
};

// ============================================================================
// Initialization
// ============================================================================

document.addEventListener('DOMContentLoaded', initializeApp);

async function initializeApp() {
    console.log('[DEBUG] initializeApp started');
    try {
        // Skip health check - not always available
        // Just load schemas directly
        console.log('[DEBUG] Loading schemas...');
        
        // Load schemas (required)
        try {
            await loadSchemas();
            console.log('[DEBUG] Schemas loaded successfully');
        } catch (error) {
            console.error('[ERROR] Failed to load schemas:', error);
            throw error; // Schemas are required, so re-throw
        }
        
        // Load configurations (optional - may not be implemented)
        console.log('[DEBUG] Loading configs...');
        try {
            await loadConfigs();
            console.log('[DEBUG] Configs loaded successfully');
        } catch (error) {
            console.warn('[WARN] Could not load configs (optional):', error);
            AppState.configs = []; // Set empty array on failure
        }
        
        // Transition to main menu
        console.log('[DEBUG] Transitioning to main-menu state');
        AppState.setState('main-menu');
        
        // Setup event listeners
        console.log('[DEBUG] Setting up event listeners');
        setupEventListeners();
        
        console.log('[DEBUG] Initialization complete!');
    } catch (error) {
        console.error('[ERROR] Initialization error:', error);
        showError('Failed to initialize application: ' + error.message);
        AppState.setState('error');
    }
}

// ============================================================================
// Server Communication
// ============================================================================

async function checkServerHealth() {
    try {
        const response = await fetch('/health');
        if (!response.ok) {
            throw new Error('Server health check failed');
        }
        const data = await response.json();
        console.log('Server healthy:', data);
        updateServerInfo();
    } catch (error) {
        console.warn('Health check error:', error);
        throw error;
    }
}

async function loadSchemas() {
    try {
        const response = await fetch('/api/schemas');
        if (!response.ok) {
            throw new Error('Failed to load schemas');
        }
        
        AppState.schemas = await response.json();
        console.log('Loaded schemas:', AppState.schemas);
        
        if (AppState.schemas.length === 0) {
            console.warn('No schemas found on server');
        }
    } catch (error) {
        console.error('Schema loading error:', error);
        throw error;
    }
}

async function loadConfigs() {
    try {
        const response = await fetch('/api/configs');
        if (!response.ok) {
            // If endpoint doesn't exist (404), that's ok - just means no configs available yet
            if (response.status === 404) {
                AppState.configs = [];
                return;
            }
            throw new Error('Failed to load configurations');
        }
        
        AppState.configs = await response.json();
        console.log('Loaded configurations:', AppState.configs);
    } catch (error) {
        console.warn('Config loading error (may not be implemented yet):', error);
        AppState.configs = [];
    }
}

async function loadSchemaData(schemaId) {
    try {
        // Fetch the full schema from the server using the new endpoint
        const response = await fetch(`/api/schemas/get?id=${encodeURIComponent(schemaId)}`);
        
        if (!response.ok) {
            throw new Error(`Failed to fetch schema: ${response.status}`);
        }
        
        const fullSchema = await response.json();
        
        // Store the full schema
        AppState.selectedSchemaData = fullSchema;
        
        console.log('[SCHEMA] Loaded full schema:', fullSchema);
        return fullSchema;
    } catch (error) {
        console.error('[SCHEMA] Error loading schema data:', error);
        
        // Fallback: use metadata from AppState
        console.log('[SCHEMA] Falling back to metadata from AppState');
        const schema = AppState.schemas.find(s => s.id === schemaId);
        if (schema) {
            AppState.selectedSchemaData = schema;
            return schema;
        }
        
        throw error;
    }
}

async function selectSchema(schemaId) {
    try {
        AppState.selectedSchema = schemaId;
        await loadSchemaData(schemaId);
        AppState.clearFormData();
        AppState.setState('form-generator');
    } catch (error) {
        showError('Failed to select schema: ' + error.message);
    }
}

async function loadConfigFile(configId) {
    try {
        const response = await fetch(`/api/configs/${configId}`);
        if (!response.ok) {
            throw new Error('Failed to load configuration');
        }
        const configData = await response.json();
        return configData;
    } catch (error) {
        console.error('Error loading config:', error);
        throw error;
    }
}

async function selectConfig(configId) {
    try {
        console.log('[CONFIG] selectConfig called with:', configId);
        
        // Find the config in AppState.configs
        // configId is the index or reference to the config
        let configData = null;
        
        if (AppState.configs && AppState.configs.length > 0) {
            // Find config by id or use as index
            configData = AppState.configs.find(c => c.id === configId || c.filename === configId);
            if (!configData) {
                // Try parsing as index
                const index = parseInt(configId);
                if (!isNaN(index) && index < AppState.configs.length) {
                    configData = AppState.configs[index];
                }
            }
        }
        
        if (!configData) {
            throw new Error('Configuration not found: ' + configId);
        }
        
        console.log('[CONFIG] Found config:', configData);
        
        // Determine which schema this config uses
        let schemaId = null;
        
        // Check for schemaId in config metadata
        if (configData.schemaId) {
            schemaId = configData.schemaId;
        } else if (configData._schemaId) {
            schemaId = configData._schemaId;
        }
        
        // Store the loaded config data for populating the form
        AppState.selectedConfigData = configData;
        
        // Track where this config came from for later save decisions
        AppState.setLoadedConfig(configId, configData.filename || configData.id || configId);
        
        // Close the modal if it's open
        const modal = document.getElementById('config-modal');
        if (modal && !modal.classList.contains('hidden')) {
            modal.classList.add('hidden');
        }
        
        // Check if we're already on the form and the schema is the same
        if (AppState.currentState === 'form-generator' && 
            schemaId && 
            AppState.selectedSchema === schemaId) {
            // Same schema - just populate the form without re-rendering
            console.log('[CONFIG] Same schema, populating existing form');
            populateFormWithConfig(configData);
            showToast('success', 'Configuration Loaded', `Pre-filled form with ${configData.filename || configData.id}`);
            return;
        } else if (schemaId) {
            // Different schema - load it and then populate
            console.log('[CONFIG] Different schema, loading schema:', schemaId);
            AppState.selectedSchema = schemaId;
            await loadSchemaData(schemaId);
            AppState.clearFormData();
            AppState.setState('form-generator');
            
            // Wait for form to render, then populate
            setTimeout(() => {
                populateFormWithConfig(configData);
                showToast('success', 'Configuration Loaded', `Loaded ${configData.filename || configData.id} with schema ${AppState.selectedSchemaData.name}`);
            }, 200);
        } else {
            console.warn('[CONFIG] No schema info in config, attempting to match config structure to schema');
            
            // Try to match the config data structure to one of the available schemas
            let bestMatchSchema = null;
            let bestMatchScore = 0;
            
            // Get the top-level keys from the config (exclude metadata fields)
            const configKeys = Object.keys(configData).filter(k => 
                !['id', 'filename', 'schemaName', 'schemaId', 'timestamp', 'data'].includes(k)
            );
            console.log('[CONFIG] Config top-level keys:', configKeys);
            
            // Compare with each schema's top-level properties
            for (const schema of AppState.schemas) {
                // Handle both direct properties and _fullSchema.properties
                let schemaProperties = schema.properties || (schema._fullSchema && schema._fullSchema.properties);
                
                if (schemaProperties) {
                    const schemaKeys = Object.keys(schemaProperties);
                    // Calculate match score (how many config keys match schema)
                    const matchCount = configKeys.filter(k => schemaKeys.includes(k)).length;
                    console.log(`[CONFIG] Schema "${schema.name || schema.title}" (id: ${schema.id}): ${matchCount}/${configKeys.length} keys match`);
                    
                    if (matchCount > bestMatchScore) {
                        bestMatchScore = matchCount;
                        bestMatchSchema = schema;
                    }
                }
            }
            
            if (bestMatchSchema && bestMatchScore > 0) {
                console.log(`[CONFIG] Best match: schema "${bestMatchSchema.name || bestMatchSchema.title}" with ${bestMatchScore} matching keys`);
                const matchedSchemaId = bestMatchSchema.id;
                AppState.selectedSchema = matchedSchemaId;
                
                // Use the schema directly if it has _fullSchema (custom uploaded), otherwise load it
                if (bestMatchSchema._fullSchema) {
                    console.log('[CONFIG] Using custom uploaded schema (has _fullSchema)');
                    AppState.selectedSchemaData = bestMatchSchema;
                } else {
                    console.log('[CONFIG] Loading schema from server');
                    await loadSchemaData(matchedSchemaId);
                }
                
                AppState.clearFormData();
                
                // Check if we're already displaying this schema - if so, preserve expand/collapse state
                const alreadyDisplayed = AppState.currentState === 'form-generator' && AppState.selectedSchema === matchedSchemaId;
                if (!alreadyDisplayed) {
                    console.log('[CONFIG] Matched schema not currently displayed, rendering form');
                    AppState.setState('form-generator');
                    setTimeout(() => {
                        populateFormWithConfig(configData);
                        showToast('success', 'Configuration Loaded', `Loaded ${configData.filename || configData.id} (matched to ${bestMatchSchema.name || bestMatchSchema.title})`);
                    }, 200);
                } else {
                    console.log('[CONFIG] Matched schema already displayed, skipping re-render to preserve expand/collapse state');
                    populateFormWithConfig(configData);
                    showToast('success', 'Configuration Loaded', `Loaded ${configData.filename || configData.id} (matched to ${bestMatchSchema.name || bestMatchSchema.title})`);
                }
            } else if (AppState.schemas.length > 0) {
                // Fallback: use first schema if no match found
                console.warn('[CONFIG] No matching schema found, using first schema');
                const firstSchema = AppState.schemas[0];
                const firstSchemaId = firstSchema.id;
                AppState.selectedSchema = firstSchemaId;
                
                // Use the schema directly if it has _fullSchema (custom uploaded), otherwise load it
                if (firstSchema._fullSchema) {
                    console.log('[CONFIG] Using custom uploaded schema (has _fullSchema)');
                    AppState.selectedSchemaData = firstSchema;
                } else {
                    console.log('[CONFIG] Loading schema from server');
                    await loadSchemaData(firstSchemaId);
                }
                
                AppState.clearFormData();
                
                // Check if we're already displaying this schema - if so, preserve expand/collapse state
                const alreadyDisplayed = AppState.currentState === 'form-generator' && AppState.selectedSchema === firstSchemaId;
                if (!alreadyDisplayed) {
                    console.log('[CONFIG] First schema not currently displayed, rendering form');
                    AppState.setState('form-generator');
                    setTimeout(() => {
                        populateFormWithConfig(configData);
                        showToast('warning', 'Schema Guessed', `Using ${firstSchema.name || firstSchema.title} - may not match perfectly`);
                    }, 200);
                } else {
                    console.log('[CONFIG] First schema already displayed, skipping re-render to preserve expand/collapse state');
                    populateFormWithConfig(configData);
                    showToast('warning', 'Schema Guessed', `Using ${firstSchema.name || firstSchema.title} - may not match perfectly`);
                }
            }
        }
    } catch (error) {
        console.error('[CONFIG] Error:', error);
        showToast('error', 'Error', 'Failed to load configuration: ' + error.message);
    }
}

function populateFormWithConfig(configData) {
    console.log('[POPULATE] populateFormWithConfig called with:', configData);
    
    if (!configData || typeof configData !== 'object') {
        console.warn('[POPULATE] Invalid config data');
        return;
    }
    
    // Extract actual data - config might have structure: { schemaId, schemaName, data: {...}, timestamp }
    // Or it might be flat data directly
    let actualData = configData;
    if (configData.data && typeof configData.data === 'object' && !Array.isArray(configData.data)) {
        console.log('[POPULATE] Config has nested data property, using that');
        actualData = configData.data;
    }
    
    console.log('[POPULATE] Actual data to populate:', actualData);
    
    // Step 1: Populate simple input fields with data-field-name attribute
    // This includes both top-level and nested fields (nested fields are identified by their parent container)
    const formInputs = document.querySelectorAll('[data-field-name]');
    console.log('[POPULATE] Found', formInputs.length, 'inputs with data-field-name');
    
    formInputs.forEach(input => {
        const fieldName = input.getAttribute('data-field-name');
        
        // Check if this field is inside nested object container(s)
        // Handle multiple levels of nesting: configureRules.3DFileCompletenessRule.requiredSuffixes
        let fieldValue = null;
        let nestedPath = []; // Will store [parentField, grandparentField, ...]
        
        // Traverse up through all nested-object containers
        let current = input.closest('.nested-object');
        while (current) {
            const parentGroup = current.closest('.form-group');
            if (parentGroup && parentGroup.id) {
                const parentFieldName = parentGroup.id.replace('field-', '');
                nestedPath.unshift(parentFieldName); // Add to front of path
            }
            current = current.parentElement?.closest('.nested-object');
        }
        
        if (nestedPath.length > 0) {
            // Multiple levels of nesting
            // nestedPath = ['configureRules', '3DFileCompletenessRule']
            // fieldName = 'requiredSuffixes'
            let obj = actualData;
            let pathStr = '';
            
            // Navigate through the nested structure
            for (let i = 0; i < nestedPath.length; i++) {
                const pathKey = nestedPath[i];
                if (obj && typeof obj === 'object' && obj[pathKey]) {
                    obj = obj[pathKey];
                    pathStr += (pathStr ? '.' : '') + pathKey;
                } else {
                    obj = null;
                    break;
                }
            }
            
            // Get the final value
            if (obj && typeof obj === 'object' && obj[fieldName] !== undefined) {
                fieldValue = obj[fieldName];
                pathStr += (pathStr ? '.' : '') + fieldName;
                console.log('[POPULATE] Deeply nested field:', pathStr, '=', fieldValue);
            }
        } else {
            // Top-level field
            fieldValue = actualData[fieldName];
        }
        
        if (fieldValue !== undefined && fieldValue !== null) {
            console.log('[POPULATE] Setting field:', fieldName, '=', fieldValue, '(type:', typeof fieldValue, ')');
            
            if (input.type === 'checkbox') {
                input.checked = Boolean(fieldValue);
            } else if (input.type === 'radio') {
                const radioOption = document.querySelector(`input[name="${fieldName}"][value="${fieldValue}"]`);
                if (radioOption) {
                    radioOption.checked = true;
                }
            } else if (input.type === 'select-one' || input.tagName === 'SELECT') {
                input.value = fieldValue;
            } else {
                input.value = String(fieldValue);
            }
        }
    });
    
    // Step 2: Handle array fields (simple arrays with .array-item-input elements)
    // Support both top-level and deeply nested arrays
    console.log('[POPULATE] Processing array fields');
    const arrayContainers = document.querySelectorAll('.array-field');
    arrayContainers.forEach(container => {
        const fieldName = container.dataset.fieldName;
        let arrayValue = null;
        
        // Check if array is deeply nested
        let nestedPath = [];
        let current = container.closest('.nested-object');
        while (current) {
            const parentGroup = current.closest('.form-group');
            if (parentGroup && parentGroup.id) {
                const parentFieldName = parentGroup.id.replace('field-', '');
                nestedPath.unshift(parentFieldName);
            }
            current = current.parentElement?.closest('.nested-object');
        }
        
        if (nestedPath.length > 0) {
            // Deeply nested array
            let obj = actualData;
            for (let i = 0; i < nestedPath.length; i++) {
                const pathKey = nestedPath[i];
                if (obj && typeof obj === 'object' && obj[pathKey]) {
                    obj = obj[pathKey];
                } else {
                    obj = null;
                    break;
                }
            }
            if (obj && typeof obj === 'object') {
                arrayValue = obj[fieldName];
            }
        } else {
            // Top-level array
            arrayValue = actualData[fieldName];
        }
        
        if (Array.isArray(arrayValue)) {
            console.log('[POPULATE] Populating array:', fieldName, 'with', arrayValue.length, 'items');
            
            const items = container.querySelectorAll('.array-item-input');
            arrayValue.forEach((value, index) => {
                if (items[index]) {
                    items[index].value = value;
                    console.log('[POPULATE] Array item', index, '=', value);
                }
            });
        }
    });
    
    // Step 3: Handle object array fields
    // Support both top-level and deeply nested object arrays
    console.log('[POPULATE] Processing object array fields');
    const objectArrayContainers = document.querySelectorAll('.object-array-field');
    objectArrayContainers.forEach(container => {
        const fieldName = container.dataset.fieldName;
        let arrayValue = null;
        
        // Check if object array is deeply nested
        let nestedPath = [];
        let current = container.closest('.nested-object');
        while (current) {
            const parentGroup = current.closest('.form-group');
            if (parentGroup && parentGroup.id) {
                const parentFieldName = parentGroup.id.replace('field-', '');
                nestedPath.unshift(parentFieldName);
            }
            current = current.parentElement?.closest('.nested-object');
        }
        
        if (nestedPath.length > 0) {
            // Deeply nested object array
            let obj = actualData;
            for (let i = 0; i < nestedPath.length; i++) {
                const pathKey = nestedPath[i];
                if (obj && typeof obj === 'object' && obj[pathKey]) {
                    obj = obj[pathKey];
                } else {
                    obj = null;
                    break;
                }
            }
            if (obj && typeof obj === 'object') {
                arrayValue = obj[fieldName];
            }
        } else {
            // Top-level object array
            arrayValue = actualData[fieldName];
        }
        
        if (Array.isArray(arrayValue)) {
            console.log('[POPULATE] Populating object array:', fieldName, 'with', arrayValue.length, 'objects');
            
            const itemsContainer = container.querySelector('.object-array-items');
            let items = container.querySelectorAll('.object-array-item');
            
            // If items were deleted, recreate them
            if (items.length < arrayValue.length) {
                console.log('[POPULATE] Array has', items.length, 'items but config has', arrayValue.length, 'items - recreating missing items');
                
                // Find the schema for this array field (need to handle deeply nested paths)
                let fieldSchema = null;
                
                if (nestedPath.length > 0) {
                    // Deeply nested - navigate schema using the same path
                    let schemaObj = AppState.selectedSchemaData._fullSchema?.properties || AppState.selectedSchemaData.properties;
                    for (let i = 0; i < nestedPath.length && schemaObj; i++) {
                        const pathKey = nestedPath[i];
                        schemaObj = schemaObj[pathKey];
                        if (schemaObj && schemaObj.properties) {
                            schemaObj = schemaObj.properties;
                        }
                    }
                    if (schemaObj && schemaObj[fieldName]) {
                        fieldSchema = schemaObj[fieldName];
                    }
                } else {
                    // Top-level
                    fieldSchema = AppState.selectedSchemaData._fullSchema?.properties?.[fieldName] || 
                                 AppState.selectedSchemaData.properties?.[fieldName];
                }
                
                console.log('[POPULATE] Field schema found:', fieldSchema ? 'yes' : 'no', 'with items:', fieldSchema?.items ? 'yes' : 'no');
                
                // Recreate missing items
                for (let i = items.length; i < arrayValue.length; i++) {
                    if (fieldSchema && fieldSchema.items) {
                        console.log('[POPULATE] Recreating object array item', i, 'with data:', arrayValue[i]);
                        // Pass the full fieldSchema (it has items property) so addObjectArrayItem can access fieldSchema.items
                        addObjectArrayItem(itemsContainer, fieldSchema, arrayValue[i], i);
                    }
                }
                
                // Re-query items after recreation
                items = container.querySelectorAll('.object-array-item');
            }
            
            arrayValue.forEach((obj, index) => {
                if (items[index] && obj && typeof obj === 'object') {
                    console.log('[POPULATE] Object item', index, ':', obj);
                    
                    // Populate fields within this object
                    const fields = items[index].querySelectorAll('[data-field-name]');
                    fields.forEach(field => {
                        const propName = field.getAttribute('data-field-name');
                        const propValue = obj[propName];
                        
                        if (propValue !== undefined && propValue !== null) {
                            console.log('[POPULATE] Object field', propName, '=', propValue);
                            
                            if (field.type === 'checkbox') {
                                field.checked = Boolean(propValue);
                            } else if (field.tagName === 'SELECT') {
                                field.value = propValue;
                            } else {
                                field.value = String(propValue);
                            }
                        }
                    });
                }
            });
        }
    });
    
    // Step 4: Handle dictionary fields
    // Support both top-level and deeply nested dictionaries
    console.log('[POPULATE] Processing dictionary fields');
    const dictContainers = document.querySelectorAll('.dictionary-field');
    dictContainers.forEach(container => {
        const fieldName = container.dataset.fieldName;
        let dictValue = null;
        
        // Check if dictionary is deeply nested
        let nestedPath = [];
        let current = container.closest('.nested-object');
        while (current) {
            const parentGroup = current.closest('.form-group');
            if (parentGroup && parentGroup.id) {
                const parentFieldName = parentGroup.id.replace('field-', '');
                nestedPath.unshift(parentFieldName);
            }
            current = current.parentElement?.closest('.nested-object');
        }
        
        if (nestedPath.length > 0) {
            // Deeply nested dictionary
            let obj = actualData;
            for (let i = 0; i < nestedPath.length; i++) {
                const pathKey = nestedPath[i];
                if (obj && typeof obj === 'object' && obj[pathKey]) {
                    obj = obj[pathKey];
                } else {
                    obj = null;
                    break;
                }
            }
            if (obj && typeof obj === 'object') {
                dictValue = obj[fieldName];
            }
        } else {
            // Top-level dictionary
            dictValue = actualData[fieldName];
        }
        
        if (dictValue && typeof dictValue === 'object' && !Array.isArray(dictValue)) {
            console.log('[POPULATE] Populating dictionary:', fieldName, 'with', Object.keys(dictValue).length, 'items');
            
            const itemsContainer = container.querySelector('.dict-items');
            const dictItems = itemsContainer.querySelectorAll('.dict-item');
            const entries = Object.entries(dictValue);
            
            entries.forEach((entry, index) => {
                if (dictItems[index]) {
                    const keyInput = dictItems[index].querySelector('.dict-key-input');
                    const valInput = dictItems[index].querySelector('.dict-value-input');
                    
                    if (keyInput) {
                        keyInput.value = entry[0];
                        console.log('[POPULATE] Dict key', index, '=', entry[0]);
                    }
                    if (valInput) {
                        valInput.value = entry[1];
                        console.log('[POPULATE] Dict value', index, '=', entry[1]);
                    }
                }
            });
        }
    });
    
    console.log('[POPULATE] Form population complete - nested objects preserve their expand/collapse state');
}

/**
 * Gather form data from the form UI and rebuild nested structure matching schema
 * Recursively processes schema to properly nest data based on schema structure
 * @returns {Object} Form data with preserved nested structure matching schema
 */
function gatherFormData() {
    console.log('[GATHER] Starting form data collection');
    
    const schema = AppState.selectedSchemaData._fullSchema || AppState.selectedSchemaData;
    const formContainer = document.getElementById('form-content');
    
    if (!schema || !schema.properties) {
        console.log('[GATHER] No schema properties found');
        return {};
    }
    
    if (!formContainer) {
        console.log('[GATHER] Form container not found');
        return {};
    }
    
    // Collect all form data by walking the schema structure
    const data = {};
    
    // Process each top-level property from schema
    Object.keys(schema.properties).forEach(fieldName => {
        const fieldSchema = schema.properties[fieldName];
        const value = collectFieldValue(fieldName, fieldSchema, formContainer);
        
        if (value !== undefined && value !== null) {
            data[fieldName] = value;
            console.log(`[GATHER] Collected: ${fieldName} =`, value);
        }
    });
    
    console.log('[GATHER] Form data collection complete:', data);
    return data;
}

/**
 * Recursively collect value for a field based on its schema type
 */
function collectFieldValue(fieldName, fieldSchema, container) {
    if (!fieldSchema) return undefined;
    
    // Handle arrays
    if (fieldSchema.type === 'array') {
        return collectArrayValue(fieldName, fieldSchema, container);
    }
    
    // Handle nested objects
    if (fieldSchema.type === 'object') {
        return collectObjectValue(fieldName, fieldSchema, container);
    }
    
    // Handle simple types - look for input with this field name
    const input = container.querySelector(`[data-field-name="${fieldName}"]`);
    if (!input) return undefined;
    
    let value = input.value;
    if (value === '' || value === undefined || value === null) {
        return undefined;
    }
    
    // Remove escaped quotes if present (from form input)
    if (typeof value === 'string' && value.startsWith('"') && value.endsWith('"')) {
        value = value.slice(1, -1);
    }
    
    // Type conversion
    if (fieldSchema.type === 'number') {
        value = parseFloat(value);
    } else if (fieldSchema.type === 'integer') {
        value = parseInt(value);
    } else if (fieldSchema.type === 'boolean') {
        value = input.checked;
    }
    
    return value;
}

/**
 * Collect array value from form
 */
function collectArrayValue(fieldName, fieldSchema, container) {
    // Find the array field container
    const arrayField = container.querySelector(`.array-field[data-field-name="${fieldName}"], .object-array-field[data-field-name="${fieldName}"]`);
    if (!arrayField) return undefined;
    
    const items = arrayField.querySelectorAll('.array-item, .object-array-item');
    if (items.length === 0) return undefined;
    
    const arrayData = [];
    
    // Check if this is an object array or simple array
    if (arrayField.classList.contains('object-array-field')) {
        // Object array - collect each item as an object
        items.forEach(item => {
            const obj = {};
            
            // Check if items have nested properties (items.properties in schema)
            if (fieldSchema.items && fieldSchema.items.properties) {
                // Collect properties based on schema
                Object.keys(fieldSchema.items.properties).forEach(propName => {
                    const propSchema = fieldSchema.items.properties[propName];
                    const propInput = item.querySelector(`[data-field-name="${propName}"]`);
                    
                    if (propInput) {
                        let propValue = propInput.value;
                        if (propValue !== '') {
                            // Remove escaped quotes if present
                            if (typeof propValue === 'string' && propValue.startsWith('"') && propValue.endsWith('"')) {
                                propValue = propValue.slice(1, -1);
                            }
                            
                            // Type conversion
                            if (propSchema.type === 'number') {
                                propValue = parseFloat(propValue);
                            } else if (propSchema.type === 'integer') {
                                propValue = parseInt(propValue);
                            } else if (propSchema.type === 'boolean') {
                                propValue = propInput.checked;
                            }
                            obj[propName] = propValue;
                        }
                    }
                });
            } else {
                // Fallback: collect all data-field-name inputs in this item
                const propInputs = item.querySelectorAll('[data-field-name]');
                propInputs.forEach(propInput => {
                    const propName = propInput.getAttribute('data-field-name');
                    let propValue = propInput.value;
                    if (propValue !== '') {
                        // Remove escaped quotes if present
                        if (typeof propValue === 'string' && propValue.startsWith('"') && propValue.endsWith('"')) {
                            propValue = propValue.slice(1, -1);
                        }
                        
                        if (propInput.type === 'number') {
                            propValue = parseFloat(propValue);
                        } else if (propInput.type === 'checkbox') {
                            propValue = propInput.checked;
                        }
                        obj[propName] = propValue;
                    }
                });
            }
            
            if (Object.keys(obj).length > 0) {
                arrayData.push(obj);
            }
        });
    } else {
        // Simple array - collect scalar values
        items.forEach(item => {
            const input = item.querySelector('input, select');
            if (input && input.value !== '') {
                let value = input.value;
                // Remove escaped quotes if present
                if (typeof value === 'string' && value.startsWith('"') && value.endsWith('"')) {
                    value = value.slice(1, -1);
                }
                if (input.type === 'number') {
                    value = parseFloat(value);
                }
                arrayData.push(value);
            }
        });
    }
    
    return arrayData.length > 0 ? arrayData : undefined;
}

/**
 * Collect nested object value from form
 */
function collectObjectValue(fieldName, fieldSchema, container) {
    // Check if this is a dictionary (additionalProperties)
    if (fieldSchema.additionalProperties && !fieldSchema.properties) {
        // Dictionary - collect as key-value pairs
        const dictField = container.querySelector(`.dictionary-field[data-field-name="${fieldName}"]`);
        if (!dictField) return undefined;
        
        const items = dictField.querySelectorAll('.dict-item');
        if (items.length === 0) return undefined;
        
        const dictData = {};
        items.forEach(item => {
            const keyInput = item.querySelector('.dict-key-input');
            const valInput = item.querySelector('.dict-value-input');
            
            if (keyInput && valInput && keyInput.value) {
                let value = valInput.value;
                // Remove escaped quotes if present
                if (typeof value === 'string' && value.startsWith('"') && value.endsWith('"')) {
                    value = value.slice(1, -1);
                }
                dictData[keyInput.value] = value;
            }
        });
        
        return Object.keys(dictData).length > 0 ? dictData : undefined;
    }
    
    // Regular nested object - recursively collect properties
    if (fieldSchema.properties) {
        const nestedData = {};
        
        Object.keys(fieldSchema.properties).forEach(propName => {
            const propSchema = fieldSchema.properties[propName];
            
            // Always use collectFieldValue for any property - it handles all types
            // (simple inputs, arrays, nested objects, dictionaries)
            const propValue = collectFieldValue(propName, propSchema, container);
            
            if (propValue !== undefined && propValue !== null) {
                nestedData[propName] = propValue;
                console.log(`[GATHER] Nested property collected: ${fieldName}.${propName} =`, propValue);
            }
        });
        
        return Object.keys(nestedData).length > 0 ? nestedData : undefined;
    }
    
    return undefined;
}

async function saveConfiguration() {
    try {
        if (!AppState.selectedSchemaData) {
            showError('No schema selected');
            return;
        }
        
        // Gather form data with nested structure preserved
        const formData = gatherFormData();
        
        // Show filename input dialog
        showFilenameInputDialog(formData);
        
    } catch (error) {
        showError('Failed to save configuration: ' + error.message);
    }
}

/**
 * Show dialog to ask user for filename
 * @param {Object} formData - Configuration data to save
 */
function showFilenameInputDialog(formData) {
    const modal = document.createElement('div');
    modal.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background: rgba(0, 0, 0, 0.5);
        display: flex;
        align-items: center;
        justify-content: center;
        z-index: 10000;
    `;
    
    const dialog = document.createElement('div');
    dialog.style.cssText = `
        background: white;
        border-radius: 8px;
        padding: 24px;
        max-width: 500px;
        box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.1);
        font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
    `;
    
    // Title
    const titleEl = document.createElement('h2');
    titleEl.style.cssText = 'margin: 0 0 12px 0; font-size: 1.25rem; color: #2d3748;';
    titleEl.textContent = 'Save Configuration';
    dialog.appendChild(titleEl);
    
    // Message
    const messageEl = document.createElement('p');
    messageEl.style.cssText = 'margin: 0 0 16px 0; color: #718096; font-size: 0.95rem;';
    messageEl.textContent = 'Enter a name for this configuration file (without extension):';
    dialog.appendChild(messageEl);
    
    // Input field
    const input = document.createElement('input');
    input.type = 'text';
    input.placeholder = 'e.g., my-config';
    input.style.cssText = `
        width: 100%;
        padding: 10px;
        border: 1px solid #e2e8f0;
        border-radius: 4px;
        font-size: 1rem;
        margin-bottom: 20px;
        box-sizing: border-box;
    `;
    
    // Generate default name
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-').slice(0, -5);
    input.value = `config_${timestamp}`;
    input.select(); // Auto-select for easy replacement
    
    dialog.appendChild(input);
    
    // Button container
    const buttonContainer = document.createElement('div');
    buttonContainer.style.cssText = 'display: flex; gap: 8px; justify-content: flex-end;';
    
    // Cancel button
    const cancelBtn = document.createElement('button');
    cancelBtn.textContent = 'Cancel';
    cancelBtn.style.cssText = `
        padding: 10px 16px;
        background: #e2e8f0;
        border: none;
        border-radius: 4px;
        cursor: pointer;
        font-size: 0.95rem;
        color: #2d3748;
        transition: background 0.2s;
    `;
    cancelBtn.onmouseover = () => cancelBtn.style.background = '#cbd5e0';
    cancelBtn.onmouseout = () => cancelBtn.style.background = '#e2e8f0';
    cancelBtn.onclick = () => {
        document.body.removeChild(modal);
    };
    
    // Save button
    const saveBtn = document.createElement('button');
    saveBtn.textContent = 'Save';
    saveBtn.style.cssText = `
        padding: 10px 16px;
        background: #48bb78;
        border: none;
        border-radius: 4px;
        cursor: pointer;
        font-size: 0.95rem;
        color: white;
        transition: background 0.2s;
    `;
    saveBtn.onmouseover = () => saveBtn.style.background = '#38a169';
    saveBtn.onmouseout = () => saveBtn.style.background = '#48bb78';
    saveBtn.onclick = async () => {
        const filename = input.value.trim();
        if (!filename) {
            alert('Please enter a filename');
            return;
        }
        
        // Validate filename (alphanumeric, dash, underscore only)
        if (!/^[a-zA-Z0-9_\-]+$/.test(filename)) {
            alert('Filename can only contain letters, numbers, dashes, and underscores');
            return;
        }
        
        document.body.removeChild(modal);
        
        // Add .json extension
        const fullFilename = `${filename}.json`;
        
        // Start hybrid save
        await saveConfigurationHybrid(formData, fullFilename);
    };
    
    buttonContainer.appendChild(cancelBtn);
    buttonContainer.appendChild(saveBtn);
    dialog.appendChild(buttonContainer);
    
    modal.appendChild(dialog);
    document.body.appendChild(modal);
    
    // Focus on input for immediate typing
    input.focus();
    
    // Allow Enter key to submit
    input.onkeypress = (e) => {
        if (e.key === 'Enter') {
            saveBtn.click();
        }
    };
}

/**
 * HYBRID STORAGE: Save configuration to both local PC and server simultaneously
 * @param {Object} formData - Configuration data to save
 * @param {string} defaultFilename - Default filename for saving
 */
async function saveConfigurationHybrid(formData, defaultFilename) {
    try {
        console.log('[HYBRID] Starting hybrid save (local + server)');
        
        let localSaved = false;
        let serverSaved = false;
        let errors = [];
        
        // Save to LOCAL PC (File System Access API with user file picker)
        if ('showSaveFilePicker' in window) {
            try {
                console.log('[HYBRID] Saving to local PC with file picker...');
                await saveToLocalPC(formData, defaultFilename);
                localSaved = true;
                console.log('[HYBRID] Local save successful');
            } catch (error) {
                if (error.name !== 'AbortError') {
                    console.error('[HYBRID] Local save error:', error);
                    errors.push(`Local save: ${error.message}`);
                }
            }
        } else {
            // Fallback: auto-download
            try {
                console.log('[HYBRID] File System API not available - using auto-download...');
                downloadConfigurationFile(formData, defaultFilename);
                localSaved = true;
                console.log('[HYBRID] Auto-download successful');
            } catch (error) {
                console.error('[HYBRID] Download error:', error);
                errors.push(`Download: ${error.message}`);
            }
        }
        
        // Save to SERVER (simultaneously)
        try {
            console.log('[HYBRID] Saving to server...');
            const filename = localSaved ? defaultFilename : 
                           `config_${new Date().getTime()}.json`;
            await saveToServerBackup(formData, filename);
            serverSaved = true;
            console.log('[HYBRID] Server save successful');
        } catch (error) {
            console.error('[HYBRID] Server save error:', error);
            errors.push(`Server save: ${error.message}`);
        }
        
        // Show unified result message
        if (localSaved && serverSaved) {
            showToast('success', 'Save Successful', 
                '✅ Saved to your computer and backed up on server');
            console.log('[HYBRID] Both local and server saves completed successfully');
        } else if (localSaved) {
            showToast('warning', 'Partial Save', 
                '✅ Saved to your computer\n⚠️ Server backup failed - check server connection');
        } else if (serverSaved) {
            showToast('warning', 'Partial Save', 
                '⚠️ Local save cancelled\n✅ Backup saved on server');
        } else {
            showError('Save Failed: ' + errors.join('; '));
        }
        
        // Refresh server file list
        try {
            await loadServerFiles();
        } catch (error) {
            console.warn('[HYBRID] Could not refresh server file list:', error);
        }
        
    } catch (error) {
        console.error('[HYBRID] Hybrid save error:', error);
        showError('Hybrid save failed: ' + error.message);
    }
}

/**
 * Save configuration to LOCAL PC using File System Access API
 * User chooses save location via file picker
 */
async function saveToLocalPC(formData, defaultFilename) {
    return new Promise(async (resolve, reject) => {
        try {
            console.log('[LOCAL] Saving to local PC:', defaultFilename);
            
            // Show file picker dialog
            const fileHandle = await window.showSaveFilePicker({
                suggestedName: defaultFilename,
                types: [
                    {
                        description: 'JSON Configuration Files',
                        accept: { 'application/json': ['.json'] }
                    }
                ],
                startIn: 'documents' // Try to start in Documents folder
            });
            
            console.log('[LOCAL] File picker confirmed:', fileHandle.name);
            
            // Write file
            const writable = await fileHandle.createWritable();
            const jsonString = JSON.stringify(formData, null, 2);
            
            await writable.write(jsonString);
            await writable.close();
            
            console.log('[LOCAL] File saved successfully to:', fileHandle.name);
            resolve({
                name: fileHandle.name,
                path: fileHandle.name
            });
            
        } catch (error) {
            if (error.name === 'AbortError') {
                console.log('[LOCAL] User cancelled file picker');
                reject(new Error('File picker cancelled by user'));
            } else {
                console.error('[LOCAL] Error:', error);
                reject(error);
            }
        }
    });
}

/**
 * Download configuration file (fallback for browsers without File System Access API)
 */
function downloadConfigurationFile(formData, filename) {
    try {
        console.log('[LOCAL] Downloading configuration:', filename);
        
        const jsonString = JSON.stringify(formData, null, 2);
        const blob = new Blob([jsonString], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        
        // Create and trigger download
        const link = document.createElement('a');
        link.href = url;
        link.download = filename;
        document.body.appendChild(link);
        link.click();
        
        // Cleanup
        setTimeout(() => {
            document.body.removeChild(link);
            URL.revokeObjectURL(url);
        }, 100);
        
        console.log('[LOCAL] Download started:', filename);
        
    } catch (error) {
        console.error('[LOCAL] Download error:', error);
        throw error;
    }
}

/**
 * Save configuration to SERVER as backup
 */
async function saveToServerBackup(formData, filename) {
    try {
        console.log('[SERVER] Saving backup to server:', filename);
        
        const response = await fetch('/api/config/save', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                filename: filename,
                data: formData
            })
        });
        
        if (!response.ok) {
            const result = await response.json();
            throw new Error(result.error || 'Server save failed');
        }
        
        const result = await response.json();
        console.log('[SERVER] Backup saved successfully:', result.path);
        
        return result;
        
    } catch (error) {
        console.error('[SERVER] Backup save error:', error);
        throw error;
    }
}

/**
 * Load list of configurations from server
 */
async function loadServerFiles() {
    try {
        console.log('[SERVER] Loading file list from server...');
        
        const response = await fetch('/api/config/list');
        if (!response.ok) {
            throw new Error('Failed to load file list');
        }
        
        const result = await response.json();
        console.log('[SERVER] Found', result.count, 'files on server');
        
        // Display file list in UI
        displayServerFileList(result.files);
        
        return result.files;
        
    } catch (error) {
        console.error('[SERVER] Error loading file list:', error);
        throw error;
    }
}

/**
 * Display server file list in UI
 */
function displayServerFileList(files) {
    try {
        let html = '<h4>📊 Saved Configurations on Server</h4>';
        
        if (!files || files.length === 0) {
            html += '<p>No configurations saved on server yet</p>';
        } else {
            html += '<div class="server-files-list">';
            files.forEach(file => {
                // Extract filename from file object
                const filename = (typeof file === 'string') ? file : (file.filename || file.name || file);
                const size = file.size ? `(${(file.size / 1024).toFixed(1)}KB)` : '';
                
                html += `
                    <div class="server-file-item">
                        <span class="file-name">${escapeHtml(filename)} ${size}</span>
                        <div class="file-actions">
                            <button class="btn-small" onclick="downloadFromServer('${filename.replace(/'/g, "\\'")}')" 
                                    title="Download">📥</button>
                            <button class="btn-small btn-danger" onclick="deleteFromServer('${filename.replace(/'/g, "\\'")}')" 
                                    title="Delete">🗑️</button>
                        </div>
                    </div>
                `;
            });
            html += '</div>';
        }
        
        // Update or create file list container
        let container = document.getElementById('server-file-list');
        if (!container) {
            container = document.createElement('div');
            container.id = 'server-file-list';
            container.className = 'server-files-section';
            document.querySelector('.form-content')?.parentElement.appendChild(container);
        }
        
        container.innerHTML = html;
        container.style.display = 'block';
        
    } catch (error) {
        console.error('[SERVER] Error displaying file list:', error);
    }
}

/**
 * Download configuration from server
 */
async function downloadFromServer(filename) {
    try {
        console.log('[SERVER] Downloading from server:', filename);
        
        const response = await fetch(`/api/config/download/${encodeURIComponent(filename)}`);
        if (!response.ok) {
            throw new Error('Download failed');
        }
        
        const blob = await response.blob();
        const url = URL.createObjectURL(blob);
        
        const link = document.createElement('a');
        link.href = url;
        link.download = filename;
        document.body.appendChild(link);
        link.click();
        
        setTimeout(() => {
            document.body.removeChild(link);
            URL.revokeObjectURL(url);
        }, 100);
        
        showToast('success', 'Downloaded', `Downloaded: ${filename}`);
        console.log('[SERVER] Downloaded successfully');
        
    } catch (error) {
        console.error('[SERVER] Download error:', error);
        showError('Failed to download: ' + error.message);
    }
}

/**
 * Delete configuration from server
 */
async function deleteFromServer(filename) {
    if (!confirm(`Delete "${filename}" from server?`)) {
        return;
    }
    
    try {
        console.log('[SERVER] Deleting from server:', filename);
        
        const response = await fetch(`/api/config/${encodeURIComponent(filename)}`, {
            method: 'DELETE'
        });
        
        if (!response.ok) {
            const result = await response.json();
            throw new Error(result.error || 'Delete failed');
        }
        
        showToast('success', 'Deleted', `Removed: ${filename}`);
        console.log('[SERVER] Deleted successfully');
        
        // Refresh file list
        await loadServerFiles();
        
    } catch (error) {
        console.error('[SERVER] Delete error:', error);
        showError('Failed to delete: ' + error.message);
    }
}

// ============================================================================
// UI Rendering
// ============================================================================

function renderUI() {
    console.log('[RENDER] renderUI called, current state:', AppState.currentState);
    
    // Hide all states
    document.getElementById('main-menu-state').classList.add('hidden');
    document.getElementById('schema-selector-state').classList.add('hidden');
    document.getElementById('config-selector-state').classList.add('hidden');
    document.getElementById('form-generator-state').classList.add('hidden');
    document.getElementById('error-state').classList.add('hidden');
    
    console.log('[RENDER] All states hidden');
    
    // Show current state
    const stateId = `${AppState.currentState}-state`;
    console.log('[RENDER] Showing state:', stateId);
    
    const stateElement = document.getElementById(stateId);
    if (stateElement) {
        stateElement.classList.remove('hidden');
        console.log('[RENDER] State element found and hidden class removed');
    } else {
        console.error('[RENDER] State element not found:', stateId);
    }
    
    // Update state-specific content
    if (AppState.currentState === 'main-menu') {
        console.log('[RENDER] Calling renderMainMenu()');
        renderMainMenu();
    } else if (AppState.currentState === 'schema-selector') {
        console.log('[RENDER] Calling renderSchemaSelector()');
        renderSchemaSelector();
    } else if (AppState.currentState === 'config-selector') {
        console.log('[RENDER] Calling renderConfigSelector()');
        renderConfigSelector();
    } else if (AppState.currentState === 'form-generator') {
        console.log('[RENDER] Calling renderFormGenerator()');
        renderFormGenerator();
    }
    
    console.log('[RENDER] renderUI complete');
}

function renderMainMenu() {
    // Main menu doesn't need dynamic rendering - it's static HTML
    // Just make sure buttons are attached to handlers
}

function renderSchemaSelector() {
    const schemasList = document.getElementById('schemas-list');
    schemasList.innerHTML = '';
    
    if (AppState.schemas.length === 0) {
        schemasList.innerHTML = `
            <div class="empty-state">
                <div class="empty-state-icon">📭</div>
                <div class="empty-state-text">No schemas available</div>
                <p style="color: var(--text-secondary);">Upload a custom schema to get started.</p>
            </div>
        `;
        return;
    }
    
    AppState.schemas.forEach(schema => {
        const card = document.createElement('div');
        card.className = 'schema-card';
        if (AppState.selectedSchema === schema.id) {
            card.classList.add('selected');
        }
        
        card.innerHTML = `
            <div class="schema-icon">📄</div>
            <h3>${escapeHtml(schema.name)}</h3>
            <p>${escapeHtml(schema.description || 'No description available')}</p>
            <p class="schema-meta">Format: ${schema.fileFormat} | Size: ${formatFileSize(schema.fileSize)}</p>
        `;
        
        card.addEventListener('click', () => selectSchema(schema.id));
        schemasList.appendChild(card);
    });
}

function renderConfigSelector() {
    const configsList = document.getElementById('configs-list');
    configsList.innerHTML = '';
    
    if (AppState.configs.length === 0) {
        configsList.innerHTML = `
            <div class="empty-state">
                <div class="empty-state-icon">�</div>
                <div class="empty-state-text">No saved configurations</div>
                <p style="color: var(--text-secondary);">Start by loading a schema and saving your configuration.</p>
            </div>
        `;
        return;
    }
    
    AppState.configs.forEach(config => {
        const card = document.createElement('div');
        card.className = 'schema-card';
        
        // Parse config metadata if available
        const schemeName = config.schemaName || 'Unknown Schema';
        const timestamp = config.timestamp ? new Date(config.timestamp).toLocaleString() : 'Unknown date';
        const filename = config.filename || config.id || 'Unnamed';
        
        card.innerHTML = `
            <div class="schema-icon">�</div>
            <h3>${escapeHtml(filename)}</h3>
            <p>${escapeHtml(schemeName)}</p>
            <p class="schema-meta">${timestamp}</p>
        `;
        
        card.addEventListener('click', () => selectConfig(config.id || config.filename));
        configsList.appendChild(card);
    });
}

function renderConfigModal() {
    const configsList = document.getElementById('modal-configs-list');
    if (!configsList) {
        console.warn('[CONFIG] Modal configs list not found');
        return;
    }
    
    configsList.innerHTML = '';
    
    if (AppState.configs.length === 0) {
        configsList.innerHTML = `
            <div class="empty-state">
                <div class="empty-state-icon">📁</div>
                <div class="empty-state-text">No saved configurations</div>
                <p style="color: var(--text-secondary);">Save a configuration from this form first.</p>
            </div>
        `;
        return;
    }
    
    AppState.configs.forEach(config => {
        const card = document.createElement('div');
        card.className = 'schema-card';
        
        // Parse config metadata if available
        const schemeName = config.schemaName || 'Unknown Schema';
        const timestamp = config.timestamp ? new Date(config.timestamp).toLocaleString() : 'Unknown date';
        const filename = config.filename || config.id || 'Unnamed';
        
        card.innerHTML = `
            <div class="schema-icon">📁</div>
            <h3>${escapeHtml(filename)}</h3>
            <p>${escapeHtml(schemeName)}</p>
            <p class="schema-meta">${timestamp}</p>
        `;
        
        card.addEventListener('click', () => selectConfig(config.id || config.filename));
        configsList.appendChild(card);
    });
}


function renderSchemaSelector() {
    const schemasList = document.getElementById('schemas-list');
    schemasList.innerHTML = '';
    
    AppState.schemas.forEach(schema => {
        const card = document.createElement('div');
        card.className = 'schema-card';
        if (AppState.selectedSchema === schema.id) {
            card.classList.add('selected');
        }
        
        card.innerHTML = `
            <div class="schema-icon">📄</div>
            <h3>${escapeHtml(schema.name)}</h3>
            <p>${escapeHtml(schema.description || 'No description available')}</p>
        `;
        
        card.addEventListener('click', () => selectSchema(schema.id));
        schemasList.appendChild(card);
    });
}

function renderFormGenerator() {
    console.log('[FORM] renderFormGenerator called');
    const formTitle = document.getElementById('form-title');
    const formContent = document.getElementById('form-content');
    
    console.log('[FORM] formTitle element:', formTitle);
    console.log('[FORM] formContent element:', formContent);
    console.log('[FORM] selectedSchemaData:', AppState.selectedSchemaData);
    
    if (AppState.selectedSchemaData) {
        console.log('[FORM] Setting form title:', AppState.selectedSchemaData.name);
        formTitle.textContent = AppState.selectedSchemaData.name || 'Configuration Form';
        console.log('[FORM] Calling renderForm');
        renderForm(AppState.selectedSchemaData, formContent);
        console.log('[FORM] renderForm complete');
    } else {
        console.error('[FORM] No selectedSchemaData available');
    }
}

function renderForm(schema, container) {
    console.log('[FORM] renderForm called with schema:', schema);
    console.log('[FORM] container element:', container);
    
    container.innerHTML = '';
    
    // Create form fields based on schema
    // For MVP, we'll create simple text fields for all properties
    
    if (schema._fullSchema && schema._fullSchema.properties) {
        console.log('[FORM] Schema has properties, creating fields');
        Object.keys(schema._fullSchema.properties).forEach(fieldName => {
            const fieldSchema = schema._fullSchema.properties[fieldName];
            const fieldElement = createFormField(fieldName, fieldSchema);
            container.appendChild(fieldElement);
        });
        console.log('[FORM] Form fields created');
    } else if (schema.properties) {
        console.log('[FORM] Schema has properties (non-full), creating fields');
        Object.keys(schema.properties).forEach(fieldName => {
            const fieldSchema = schema.properties[fieldName];
            const fieldElement = createFormField(fieldName, fieldSchema);
            container.appendChild(fieldElement);
        });
        console.log('[FORM] Form fields created');
    } else {
        console.log('[FORM] No properties found in schema');
        // Simple message if schema has no properties
        const msg = document.createElement('p');
        msg.textContent = 'This schema has no configurable properties.';
        msg.style.color = 'var(--text-secondary)';
        container.appendChild(msg);
    }
}

function createFormField(fieldName, fieldSchema, arrayIndex = null) {
    const group = document.createElement('div');
    group.className = 'form-group';
    group.id = `field-${fieldName}${arrayIndex !== null ? `-${arrayIndex}` : ''}`;
    
    // Create input based on schema type
    let input;
    
    // Handle nested objects
    if (fieldSchema.type === 'object') {
        console.log('[FORM] Creating field for nested object:', fieldName);
        
        // Create collapsible header (replaces label for nested objects)
        const header = document.createElement('div');
        header.className = 'nested-object-header';
        header.style.display = 'flex';
        header.style.alignItems = 'center';
        header.style.gap = '8px';
        header.style.padding = '10px';
        header.style.backgroundColor = '#f5f5f5';
        header.style.borderRadius = '4px';
        header.style.cursor = 'pointer';
        header.style.userSelect = 'none';
        header.style.marginTop = '10px';
        header.style.marginBottom = '0px';
        
        // Collapse button
        const collapseBtn = document.createElement('button');
        collapseBtn.className = 'btn-collapse-object';
        collapseBtn.textContent = '▶';  // Start collapsed
        collapseBtn.type = 'button';
        collapseBtn.style.background = 'none';
        collapseBtn.style.border = 'none';
        collapseBtn.style.cursor = 'pointer';
        collapseBtn.style.fontSize = '12px';
        collapseBtn.style.width = '20px';
        collapseBtn.style.padding = '0px';
        collapseBtn.style.marginRight = '4px';
        
        // Header label (combines field name with required indicator)
        const headerLabel = document.createElement('span');
        headerLabel.textContent = formatFieldName(fieldName);
        headerLabel.style.fontWeight = 'bold';
        headerLabel.style.color = 'var(--text-primary)';
        
        // Add required indicator if needed
        if (fieldSchema.required !== false) {
            const required = document.createElement('span');
            required.className = 'required';
            required.textContent = '*';
            required.style.marginLeft = '2px';
            headerLabel.appendChild(required);
        }
        
        header.appendChild(collapseBtn);
        header.appendChild(headerLabel);
        group.appendChild(header);
        
        // Content container (initially collapsed)
        const objectGroup = document.createElement('div');
        objectGroup.className = 'nested-object collapsed';  // Start collapsed
        objectGroup.style.marginLeft = '20px';
        objectGroup.style.borderLeft = '2px solid var(--border-color)';
        objectGroup.style.paddingLeft = '10px';
        objectGroup.style.display = 'none';  // Hidden by default
        
        // Check if this is a dictionary (additionalProperties) or structured object (properties)
        if (fieldSchema.additionalProperties && !fieldSchema.properties) {
            // This is a dictionary/key-value object - render as editable list
            console.log('[FORM] Dictionary object detected:', fieldName);
            createDictionaryField(fieldName, fieldSchema, objectGroup);
            group.appendChild(objectGroup);
        } else if (fieldSchema.properties) {
            // This is a structured object with defined properties
            Object.keys(fieldSchema.properties).forEach(nestedFieldName => {
                const nestedFieldSchema = fieldSchema.properties[nestedFieldName];
                const nestedField = createFormField(nestedFieldName, nestedFieldSchema, arrayIndex);  // Pass arrayIndex
                objectGroup.appendChild(nestedField);
            });
            group.appendChild(objectGroup);
        } else {
            group.appendChild(objectGroup);
        }
        
        // Toggle collapse/expand on header click
        header.addEventListener('click', (e) => {
            e.preventDefault();
            const isCollapsed = objectGroup.classList.contains('collapsed');
            if (isCollapsed) {
                // Expand
                objectGroup.classList.remove('collapsed');
                objectGroup.style.display = 'block';
                collapseBtn.textContent = '▼';
                console.log('[FORM] Expanded nested object:', fieldName);
            } else {
                // Collapse
                objectGroup.classList.add('collapsed');
                objectGroup.style.display = 'none';
                collapseBtn.textContent = '▶';
                console.log('[FORM] Collapsed nested object:', fieldName);
            }
        });
        
        return group;
    }
    
    // Handle arrays
    if (fieldSchema.type === 'array') {
        console.log('[FORM] Creating field for array:', fieldName);
        
        // Determine item type
        let itemType = 'primitive';
        if (fieldSchema.items) {
            if (fieldSchema.items.type === 'object') {
                itemType = 'object';
            } else if (fieldSchema.items.enum) {
                itemType = 'enum';
            }
        }
        
        if (itemType === 'object') {
            console.log('[FORM] Array of objects detected');
            return createObjectArrayField(fieldName, fieldSchema, group);
        } else {
            console.log('[FORM] Simple array detected, type:', itemType);
            return createSimpleArrayField(fieldName, fieldSchema, group, itemType);
        }
    }
    
    // For non-nested types, add a label
    // Create label for simple fields (not nested objects)
    const label = document.createElement('label');
    label.htmlFor = `input-${fieldName}${arrayIndex !== null ? `-${arrayIndex}` : ''}`;
    label.textContent = formatFieldName(fieldName);
    
    if (fieldSchema.required !== false) {
        const required = document.createElement('span');
        required.className = 'required';
        required.textContent = '*';
        label.appendChild(required);
    }
    
    group.appendChild(label);
    
    // Handle other types
    if (fieldSchema.type === 'string') {
        if (fieldSchema.enum) {
            input = createSelectField(fieldName, fieldSchema);
        } else if (fieldSchema.format === 'textarea') {
            input = document.createElement('textarea');
        } else {
            input = document.createElement('input');
            input.type = 'text';
        }
    } else if (fieldSchema.type === 'number' || fieldSchema.type === 'integer') {
        input = document.createElement('input');
        input.type = 'number';
    } else if (fieldSchema.type === 'boolean') {
        input = document.createElement('input');
        input.type = 'checkbox';
    } else {
        input = document.createElement('input');
        input.type = 'text';
    }
    
    // Only set common properties if input was created (not for objects)
    if (input) {
        input.id = `input-${fieldName}${arrayIndex !== null ? `-${arrayIndex}` : ''}`;
        input.name = fieldName;
        input.setAttribute('data-field-name', fieldName);
        
        if (fieldSchema.default !== undefined) {
            if (fieldSchema.type === 'boolean') {
                input.checked = fieldSchema.default;
            } else if (fieldSchema.type === 'string' || fieldSchema.type === 'number' || fieldSchema.type === 'integer') {
                // For string, number, and integer types, use the value directly without JSON.stringify
                // This ensures enum values like "info" don't become "\"info\""
                input.value = fieldSchema.default;
            } else {
                // For other types, use JSON.stringify
                input.value = JSON.stringify(fieldSchema.default);
            }
        }
        
        if (fieldSchema.placeholder) {
            input.placeholder = fieldSchema.placeholder;
        }
        
        if (fieldSchema.description) {
            input.title = fieldSchema.description;
        }
        
        input.addEventListener('change', (e) => {
            const value = e.target.type === 'checkbox' ? e.target.checked : e.target.value;
            AppState.addFormData(fieldName, value);
        });
        
        group.appendChild(input);
    }
    
    // Add help text
    if (fieldSchema.description) {
        const help = document.createElement('div');
        help.className = 'help-text';
        help.textContent = fieldSchema.description;
        group.appendChild(help);
    }
    
    return group;
}

// ============================================================================
// Array Field Handlers (Simple & Object Arrays)
// ============================================================================

function createSimpleArrayField(fieldName, fieldSchema, group, itemType) {
    console.log('[ARRAY] Creating simple array field:', fieldName, 'itemType:', itemType);
    
    // Array container
    const arrayContainer = document.createElement('div');
    arrayContainer.className = 'array-field';
    arrayContainer.id = `array-${fieldName}`;
    arrayContainer.dataset.fieldName = fieldName;
    arrayContainer.dataset.itemType = itemType;
    
    // Add property name label
    const labelDiv = document.createElement('div');
    labelDiv.className = 'array-property-label';
    labelDiv.textContent = fieldName + ':';
    labelDiv.style.fontWeight = 'bold';
    labelDiv.style.fontSize = '0.95em';
    labelDiv.style.color = '#555';
    labelDiv.style.marginTop = '8px';
    labelDiv.style.marginBottom = '4px';
    labelDiv.style.paddingLeft = '10px';
    labelDiv.style.borderLeft = '3px solid #6c5ce7';
    arrayContainer.appendChild(labelDiv);
    
    // Items container
    const itemsContainer = document.createElement('div');
    itemsContainer.className = 'array-items';
    itemsContainer.id = `array-items-${fieldName}`;
    arrayContainer.appendChild(itemsContainer);
    
    // Initialize with default values if available
    let hasDefaults = false;
    let defaultValues = fieldSchema.default;
    
    // Handle nested array structure (schema may have [ [ {...}, {...} ] ])
    if (defaultValues && Array.isArray(defaultValues)) {
        // If first element is also an array, unwrap it
        if (defaultValues.length > 0 && Array.isArray(defaultValues[0])) {
            defaultValues = defaultValues[0];
        }
        
        if (defaultValues.length > 0) {
            console.log('[ARRAY] Adding default items:', defaultValues.length);
            defaultValues.forEach(value => {
                addSimpleArrayItem(itemsContainer, fieldSchema, value);
            });
            hasDefaults = true;
        }
    }
    
    // If no defaults, add one empty item
    if (!hasDefaults) {
        addSimpleArrayItem(itemsContainer, fieldSchema, '');
    }
    
    // Add/Remove buttons
    const controls = document.createElement('div');
    controls.className = 'array-controls';
    
    const addBtn = document.createElement('button');
    addBtn.textContent = '+';
    addBtn.className = 'btn-add-item';
    addBtn.type = 'button';
    addBtn.onclick = (e) => {
        e.preventDefault();
        addSimpleArrayItem(itemsContainer, fieldSchema, '');
    };
    
    const removeBtn = document.createElement('button');
    removeBtn.textContent = '-';
    removeBtn.className = 'btn-remove-item';
    removeBtn.type = 'button';
    removeBtn.onclick = (e) => {
        e.preventDefault();
        removeArrayItem(itemsContainer);
    };
    
    controls.appendChild(addBtn);
    controls.appendChild(removeBtn);
    arrayContainer.appendChild(controls);
    
    group.appendChild(arrayContainer);
    return group;
}

function addSimpleArrayItem(container, fieldSchema, value) {
    console.log('[ARRAY] Adding simple array item');
    
    const itemDiv = document.createElement('div');
    itemDiv.className = 'array-item';
    
    let input;
    
    // Create appropriate input based on item type
    if (fieldSchema.items && fieldSchema.items.enum) {
        // Enum dropdown
        input = document.createElement('select');
        input.className = 'array-item-input';
        
        const emptyOption = document.createElement('option');
        emptyOption.value = '';
        emptyOption.textContent = '-- Select --';
        input.appendChild(emptyOption);
        
        fieldSchema.items.enum.forEach(enumValue => {
            const option = document.createElement('option');
            option.value = enumValue;
            option.textContent = enumValue;
            if (value === enumValue) {
                option.selected = true;
            }
            input.appendChild(option);
        });
    } else {
        // Text input
        input = document.createElement('input');
        input.className = 'array-item-input';
        input.type = 'text';
        input.value = value;
        input.placeholder = 'Enter value...';
    }
    
    itemDiv.appendChild(input);
    container.appendChild(itemDiv);
}

function createObjectArrayField(fieldName, fieldSchema, group) {
    console.log('[ARRAY] Creating object array field:', fieldName);
    
    // Array container
    const arrayContainer = document.createElement('div');
    arrayContainer.className = 'object-array-field';
    arrayContainer.id = `array-${fieldName}`;
    arrayContainer.dataset.fieldName = fieldName;
    
    // Add property name label
    const labelDiv = document.createElement('div');
    labelDiv.className = 'array-property-label';
    labelDiv.textContent = fieldName + ':';
    labelDiv.style.fontWeight = 'bold';
    labelDiv.style.fontSize = '0.95em';
    labelDiv.style.color = '#555';
    labelDiv.style.marginTop = '8px';
    labelDiv.style.marginBottom = '4px';
    labelDiv.style.paddingLeft = '10px';
    labelDiv.style.borderLeft = '3px solid #6c5ce7';
    arrayContainer.appendChild(labelDiv);
    
    // Items container
    const itemsContainer = document.createElement('div');
    itemsContainer.className = 'object-array-items';
    itemsContainer.id = `array-items-${fieldName}`;
    arrayContainer.appendChild(itemsContainer);
    
    // Initialize with default values if available
    let hasDefaults = false;
    let defaultValues = fieldSchema.default;
    
    // Handle nested array structure (schema may have [ [ {...}, {...} ] ])
    if (defaultValues && Array.isArray(defaultValues)) {
        // If first element is also an array, unwrap it
        if (defaultValues.length > 0 && Array.isArray(defaultValues[0])) {
            defaultValues = defaultValues[0];
        }
        
        if (defaultValues.length > 0) {
            console.log('[ARRAY] Adding default objects:', defaultValues.length);
            defaultValues.forEach((objData, index) => {
                addObjectArrayItem(itemsContainer, fieldSchema, objData, index);
            });
            hasDefaults = true;
        }
    }
    
    // If no defaults, add one empty object
    if (!hasDefaults) {
        addObjectArrayItem(itemsContainer, fieldSchema, {}, 0);
    }
    
    // Add/Remove buttons
    const controls = document.createElement('div');
    controls.className = 'array-controls';
    
    const addBtn = document.createElement('button');
    addBtn.textContent = '+';
    addBtn.className = 'btn-add-item';
    addBtn.type = 'button';
    addBtn.onclick = (e) => {
        e.preventDefault();
        const itemCount = itemsContainer.querySelectorAll('.object-array-item').length;
        addObjectArrayItem(itemsContainer, fieldSchema, {}, itemCount);
    };
    
    const removeBtn = document.createElement('button');
    removeBtn.textContent = '-';
    removeBtn.className = 'btn-remove-item';
    removeBtn.type = 'button';
    removeBtn.onclick = (e) => {
        e.preventDefault();
        removeArrayItem(itemsContainer);
    };
    
    controls.appendChild(addBtn);
    controls.appendChild(removeBtn);
    arrayContainer.appendChild(controls);
    
    group.appendChild(arrayContainer);
    return group;
}

function addObjectArrayItem(container, fieldSchema, objData, index) {
    console.log('[ARRAY] Adding object array item:', index, 'with data:', objData);
    
    const itemDiv = document.createElement('div');
    itemDiv.className = 'object-array-item';
    itemDiv.dataset.itemIndex = index;  // Store index for unique ID generation
    
    // Header with collapse button
    const header = document.createElement('div');
    header.className = 'object-item-header';
    
    // Build item title with name if available
    let itemTitle = `Item #${index + 1}`;
    if (fieldSchema.items && fieldSchema.items.properties) {
        // Try to get a display name from common naming fields
        const nameField = objData.name || objData.title || objData.label || objData.Name || objData.Title;
        if (nameField) {
            itemTitle = `Item #${index + 1}: ${nameField}`;
        }
    }
    
    const title = document.createElement('span');
    title.className = 'object-item-title';
    title.textContent = itemTitle;
    header.appendChild(title);
    
    const collapseBtn = document.createElement('button');
    collapseBtn.className = 'btn-collapse';
    collapseBtn.textContent = '▼';
    collapseBtn.type = 'button';
    collapseBtn.style.background = 'none';
    collapseBtn.style.border = 'none';
    collapseBtn.style.cursor = 'pointer';
    collapseBtn.style.fontSize = '12px';
    
    header.appendChild(collapseBtn);
    itemDiv.appendChild(header);
    
    // Content area (start COLLAPSED - user can expand if needed)
    const content = document.createElement('div');
    content.className = 'object-item-content';  // START COLLAPSED - no 'expanded' class
    collapseBtn.textContent = '▶';  // Show collapsed arrow initially
    
    // Generate form fields for object properties and populate with values
    if (fieldSchema.items && fieldSchema.items.properties) {
        Object.keys(fieldSchema.items.properties).forEach(propName => {
            const propSchema = fieldSchema.items.properties[propName];
            const fieldElement = createFormField(propName, propSchema, index);  // Pass index for unique IDs
            
            // POPULATE FIELD WITH VALUE FROM objData
            const value = objData[propName];
            if (value !== undefined) {
                console.log('[ARRAY] Populating field', propName, 'with value:', value);
                
                // Find the input element and set its value
                const inputElement = fieldElement.querySelector('input, textarea, select');
                if (inputElement) {
                    if (inputElement.type === 'checkbox') {
                        inputElement.checked = value;
                    } else if (inputElement.tagName === 'SELECT') {
                        inputElement.value = value;
                    } else {
                        inputElement.value = value;
                    }
                }
            }
            
            content.appendChild(fieldElement);
        });
    }
    
    itemDiv.appendChild(content);
    
    // Toggle collapse/expand
    collapseBtn.onclick = (e) => {
        e.preventDefault();
        const isExpanded = content.classList.contains('expanded');
        if (isExpanded) {
            content.classList.remove('expanded');
            collapseBtn.textContent = '▶';
        } else {
            content.classList.add('expanded');
            collapseBtn.textContent = '▼';
        }
    };
    
    container.appendChild(itemDiv);
}

function removeArrayItem(container) {
    console.log('[ARRAY] Removing array item');
    
    const items = container.querySelectorAll('.array-item, .object-array-item');
    if (items.length > 0) {
        items[items.length - 1].remove();
    }
}

// ============================================================================
// Dictionary Field Handler (Key-Value Pairs with +/- buttons)
// ============================================================================

function createDictionaryField(fieldName, fieldSchema, container) {
    console.log('[DICT] Creating dictionary field:', fieldName);
    
    // Add label
    const label = document.createElement('label');
    label.textContent = fieldName + ' (key-value pairs):';
    label.style.fontWeight = 'bold';
    label.style.display = 'block';
    label.style.marginBottom = '10px';
    container.appendChild(label);
    
    // Dictionary container
    const dictContainer = document.createElement('div');
    dictContainer.className = 'dictionary-field';
    dictContainer.id = `dict-${fieldName}`;
    dictContainer.dataset.fieldName = fieldName;
    
    // Items container
    const itemsContainer = document.createElement('div');
    itemsContainer.className = 'dict-items';
    itemsContainer.id = `dict-items-${fieldName}`;
    dictContainer.appendChild(itemsContainer);
    
    // Initialize with default values if available
    let hasDefaults = false;
    if (fieldSchema.default && typeof fieldSchema.default === 'object') {
        console.log('[DICT] Adding default items:', Object.keys(fieldSchema.default).length);
        Object.entries(fieldSchema.default).forEach(([key, value]) => {
            addDictionaryItem(itemsContainer, fieldName, key, value, fieldSchema);
        });
        hasDefaults = true;
    }
    
    // If no defaults, add one empty item
    if (!hasDefaults) {
        addDictionaryItem(itemsContainer, fieldName, '', '', fieldSchema);
    }
    
    // Add/Remove buttons
    const controls = document.createElement('div');
    controls.className = 'dict-controls';
    controls.style.marginTop = '10px';
    
    const addBtn = document.createElement('button');
    addBtn.textContent = '+';
    addBtn.className = 'btn-add-item';
    addBtn.type = 'button';
    addBtn.onclick = (e) => {
        e.preventDefault();
        addDictionaryItem(itemsContainer, fieldName, '', '', fieldSchema);
    };
    
    const removeBtn = document.createElement('button');
    removeBtn.textContent = '-';
    removeBtn.className = 'btn-remove-item';
    removeBtn.type = 'button';
    removeBtn.onclick = (e) => {
        e.preventDefault();
        removeDictionaryItem(itemsContainer);
    };
    
    controls.appendChild(addBtn);
    controls.appendChild(removeBtn);
    dictContainer.appendChild(controls);
    
    container.appendChild(dictContainer);
}

function addDictionaryItem(container, fieldName, key, value, fieldSchema) {
    console.log('[DICT] Adding dictionary item:', key);
    
    const itemDiv = document.createElement('div');
    itemDiv.className = 'dict-item';
    itemDiv.style.display = 'flex';
    itemDiv.style.gap = '10px';
    itemDiv.style.marginBottom = '8px';
    itemDiv.style.alignItems = 'center';
    
    // Key input
    const keyInput = document.createElement('input');
    keyInput.type = 'text';
    keyInput.className = 'dict-key-input';
    keyInput.placeholder = 'Key';
    keyInput.value = key;
    keyInput.style.flex = '1';
    keyInput.style.minWidth = '150px';
    
    // Value input
    const valueInput = document.createElement('input');
    valueInput.type = 'text';
    valueInput.className = 'dict-value-input';
    valueInput.placeholder = 'Value';
    valueInput.value = value;
    valueInput.style.flex = '2';
    valueInput.style.minWidth = '200px';
    
    // Update form data on change
    const updateFormData = () => {
        const allItems = container.querySelectorAll('.dict-item');
        const dictData = {};
        allItems.forEach(item => {
            const k = item.querySelector('.dict-key-input').value;
            const v = item.querySelector('.dict-value-input').value;
            if (k) {  // Only include if key is not empty
                dictData[k] = v;
            }
        });
        AppState.addFormData(fieldName, dictData);
    };
    
    keyInput.addEventListener('change', updateFormData);
    valueInput.addEventListener('change', updateFormData);
    
    itemDiv.appendChild(keyInput);
    itemDiv.appendChild(valueInput);
    container.appendChild(itemDiv);
    
    // Update form data after adding
    updateFormData();
}

function removeDictionaryItem(container) {
    console.log('[DICT] Removing dictionary item');
    
    const items = container.querySelectorAll('.dict-item');
    if (items.length > 0) {
        items[items.length - 1].remove();
        
        // Update form data after removal
        const fieldName = container.id.replace('dict-items-', '');
        const allItems = container.querySelectorAll('.dict-item');
        const dictData = {};
        allItems.forEach(item => {
            const k = item.querySelector('.dict-key-input').value;
            const v = item.querySelector('.dict-value-input').value;
            if (k) {
                dictData[k] = v;
            }
        });
        AppState.addFormData(fieldName, dictData);
    }
}

function createSelectField(fieldName, fieldSchema) {
    const select = document.createElement('select');
    
    // Add default empty option
    const emptyOption = document.createElement('option');
    emptyOption.value = '';
    emptyOption.textContent = '-- Select --';
    select.appendChild(emptyOption);
    
    // Add enum options
    fieldSchema.enum.forEach(value => {
        const option = document.createElement('option');
        option.value = value;
        option.textContent = value;
        select.appendChild(option);
    });
    
    return select;
}

// ============================================================================
// User Interactions
// ============================================================================

async function selectSchema(schemaId) {
    try {
        AppState.selectedSchema = schemaId;
        await loadSchemaData(schemaId);
        AppState.clearFormData();
        AppState.setState('form-generator');
        showToast('success', 'Schema Selected', `Loaded ${AppState.selectedSchemaData.name}`);
    } catch (error) {
        console.error('Error selecting schema:', error);
        showToast('error', 'Error', 'Failed to load selected schema');
    }
}

// Quick schema selector from header dropdown
async function handleQuickSchemaSelect(schemaId) {
    if (!schemaId) return;
    await selectSchema(schemaId);
}

// Populate quick schema selector dropdown
function populateQuickSchemaSelector() {
    const select = document.getElementById('quick-schema-select');
    if (!select) return;
    
    // Clear existing options
    select.innerHTML = '<option value="">📋 Load Schema...</option>';
    
    // Add schema options
    AppState.schemas.forEach(schema => {
        const option = document.createElement('option');
        option.value = schema.id;
        option.textContent = `📄 ${schema.name}`;
        select.appendChild(option);
    });
    
    // Show the dropdown
    select.style.display = 'block';
}

function setupEventListeners() {
    console.log('[EVENTS] setupEventListeners started');
    
    // Main Menu Buttons
    const loadSchemaBtn = document.getElementById('load-schema-btn');
    if (loadSchemaBtn) {
        loadSchemaBtn.addEventListener('click', () => {
            console.log('[EVENTS] Load Schema button clicked');
            AppState.resetSelection();
            AppState.setState('schema-selector');
        });
        console.log('[EVENTS] Load Schema button listener attached');
    } else {
        console.warn('[EVENTS] Load Schema button not found');
    }
    
    const loadConfigBtn = document.getElementById('load-config-btn');
    if (loadConfigBtn) {
        loadConfigBtn.addEventListener('click', () => {
            console.log('[EVENTS] Load Configuration button clicked');
            AppState.resetSelection();
            AppState.setState('config-selector');
        });
        console.log('[EVENTS] Load Configuration button listener attached');
    } else {
        console.warn('[EVENTS] Load Configuration button not found');
    }
    
    // Back buttons
    const backFromSchemaBtn = document.getElementById('back-from-schema-btn');
    if (backFromSchemaBtn) {
        backFromSchemaBtn.addEventListener('click', () => {
            AppState.resetSelection();
            AppState.setState('main-menu');
        });
    }
    
    const backFromConfigBtn = document.getElementById('back-from-config-btn');
    if (backFromConfigBtn) {
        backFromConfigBtn.addEventListener('click', () => {
            AppState.resetSelection();
            AppState.setState('main-menu');
        });
    }
    
    const backFromFormBtn = document.getElementById('back-from-form-btn');
    if (backFromFormBtn) {
        backFromFormBtn.addEventListener('click', () => {
            AppState.resetSelection();
            AppState.setState('main-menu');
        });
    }
    
    // Schema Upload
    const schemaUploadInput = document.getElementById('schema-upload');
    if (schemaUploadInput) {
        schemaUploadInput.addEventListener('change', handleFileUpload);
    }
    
    // Schema Upload area drag and drop
    const uploadArea = document.getElementById('upload-area');
    if (uploadArea) {
        uploadArea.addEventListener('dragover', (e) => handleDragOver(e, 'upload-area'));
        uploadArea.addEventListener('dragleave', (e) => handleDragLeave(e, 'upload-area'));
        uploadArea.addEventListener('drop', handleFileDrop);
    }
    
    // Config Upload
    const configUploadInput = document.getElementById('config-upload');
    if (configUploadInput) {
        configUploadInput.addEventListener('change', handleConfigFileUpload);
    }
    
    // Config Upload area drag and drop
    const uploadConfigArea = document.getElementById('upload-config-area');
    if (uploadConfigArea) {
        uploadConfigArea.addEventListener('dragover', (e) => handleDragOver(e, 'upload-config-area'));
        uploadConfigArea.addEventListener('dragleave', (e) => handleDragLeave(e, 'upload-config-area'));
        uploadConfigArea.addEventListener('drop', handleConfigFileDrop);
    }
    
    // Form Save button
    const saveBtn = document.getElementById('save-btn');
    if (saveBtn) {
        saveBtn.addEventListener('click', saveConfiguration);
    }

    // Load Configuration in Form button (pre-fill form with existing config)
    const loadConfigInFormBtn = document.getElementById('load-config-in-form-btn');
    if (loadConfigInFormBtn) {
        loadConfigInFormBtn.addEventListener('click', () => {
            console.log('[EVENTS] Load Configuration in Form button clicked');
            // Open modal instead of navigating away
            renderConfigModal();
            const modal = document.getElementById('config-modal');
            if (modal) {
                modal.classList.remove('hidden');
            }
        });
        console.log('[EVENTS] Load Configuration in Form button listener attached');
    } else {
        console.warn('[EVENTS] Load Configuration in Form button not found');
    }
    
    // Modal close button
    const configModalClose = document.getElementById('config-modal-close');
    if (configModalClose) {
        configModalClose.addEventListener('click', () => {
            const modal = document.getElementById('config-modal');
            if (modal) {
                modal.classList.add('hidden');
            }
        });
    }
    
    // Modal overlay click to close
    const configModal = document.getElementById('config-modal');
    if (configModal) {
        const overlay = configModal.querySelector('.modal-overlay');
        if (overlay) {
            overlay.addEventListener('click', () => {
                configModal.classList.add('hidden');
            });
        }
    }
    
    // Config Upload in Modal
    const configUploadModalInput = document.getElementById('config-upload-modal');
    if (configUploadModalInput) {
        configUploadModalInput.addEventListener('change', handleConfigFileUploadModal);
    }
    
    // Config Upload modal area drag and drop
    const uploadConfigModalArea = document.getElementById('upload-config-modal-area');
    if (uploadConfigModalArea) {
        uploadConfigModalArea.addEventListener('dragover', (e) => handleDragOver(e, 'upload-config-modal-area'));
        uploadConfigModalArea.addEventListener('dragleave', (e) => handleDragLeave(e, 'upload-config-modal-area'));
        uploadConfigModalArea.addEventListener('drop', handleConfigFileDropModal);
    }
    
    // Form Reset button
    const resetBtn = document.getElementById('reset-form-btn');
    if (resetBtn) {
        resetBtn.addEventListener('click', () => {
            // Clear form and reload it
            AppState.clearFormData();
            renderFormGenerator();
        });
    }
}

function handleDragOver(e, areaId) {
    e.preventDefault();
    e.stopPropagation();
    document.getElementById(areaId).classList.add('dragover');
}

function handleDragLeave(e, areaId) {
    e.preventDefault();
    e.stopPropagation();
    document.getElementById(areaId).classList.remove('dragover');
}

function handleFileDrop(e) {
    e.preventDefault();
    e.stopPropagation();
    document.getElementById('upload-area').classList.remove('dragover');
    
    const files = e.dataTransfer.files;
    if (files.length > 0) {
        handleFileUpload({ target: { files } });
    }
}

function handleFileUpload(e) {
    console.log('[UPLOAD] handleFileUpload called');
    const file = e.target.files[0];
    if (!file) {
        console.log('[UPLOAD] No file selected');
        return;
    }
    
    console.log('[UPLOAD] File selected:', file.name, 'Type:', file.type);
    
    // Validate file type
    const validTypes = ['application/json', 'text/yaml', 'text/plain'];
    if (!validTypes.includes(file.type) && !file.name.match(/\.(json|yaml|yml)$/)) {
        console.error('[UPLOAD] Invalid file type:', file.type);
        showToast('error', 'Invalid File', 'Please upload a JSON or YAML file');
        return;
    }
    
    console.log('[UPLOAD] File type valid, reading...');
    
    // Read file
    const reader = new FileReader();
    reader.onload = (event) => {
        try {
            const content = event.target.result;
            console.log('[UPLOAD] File content length:', content.length);
            
            // Parse as JSON or YAML (for MVP, just JSON)
            const data = JSON.parse(content);
            console.log('[UPLOAD] File parsed successfully');
            
            // Detect if this is a SCHEMA or a CONFIG file
            // A schema typically has:
            // - "properties" at root level (JSON Schema)
            // - "$schema" keyword (JSON Schema meta-schema)
            // - "type": "object" with "properties"
            // A config typically has:
            // - "data" property (our config format)
            // - or just plain key-value data matching our schema structure
            
            const isSchema = !!(
                data.properties ||           // JSON Schema with properties
                data.$schema ||              // JSON Schema meta-schema
                (data.type === 'object' && data.properties) ||  // Explicit object schema
                data.title && (data.type || data.properties)    // Schema with title and type/properties
            );
            
            const isConfig = !!(
                data.data ||                 // Our config format with data wrapper
                (data.log || data.autoRules || data.parallelism)  // Known config keys
            );
            
            console.log('[UPLOAD] File type detection: isSchema=', isSchema, 'isConfig=', isConfig);
            
            if (isConfig && !isSchema) {
                // This is a CONFIG file - route to config handler
                console.log('[UPLOAD] Detected as CONFIG file, routing to config handler');
                
                // Add to AppState.configs
                const configId = data.id || `uploaded-${Date.now()}`;
                const configEntry = {
                    id: configId,
                    filename: file.name,
                    schemaName: data.schemaName || 'Unknown Schema',
                    schemaId: data.schemaId,
                    timestamp: new Date().toISOString(),
                    ...data
                };
                
                AppState.configs.push(configEntry);
                console.log('[CONFIG] Added config from schema upload area:', configEntry);
                
                // Try to match and load
                selectConfig(configId);
                showToast('success', 'Configuration Loaded', `Loaded configuration from ${file.name}`);
            } else if (isSchema) {
                // This is a SCHEMA file - process as schema
                console.log('[UPLOAD] Detected as SCHEMA file, processing as schema');
                
                const customSchema = {
                    id: 'custom-' + Date.now(),
                    name: data.title || file.name,
                    description: data.description || 'Custom uploaded schema',
                    fileFormat: 'json'
                };
                
                // Store full schema for form generation
                customSchema._fullSchema = data;
                
                console.log('[UPLOAD] Custom schema created, adding to AppState');
                AppState.schemas.push(customSchema);
                AppState.selectedSchemaData = customSchema;
                AppState.selectedSchema = customSchema.id;
                AppState.clearFormData();
                
                console.log('[UPLOAD] Transitioning to form-generator');
                AppState.setState('form-generator');
                
                showToast('success', 'Schema Loaded', `Loaded custom schema from ${file.name}`);
            } else {
                // Unknown file format
                showToast('error', 'Unknown Format', 'File is neither a schema nor a configuration');
            }
        } catch (error) {
            console.error('[UPLOAD] Error parsing file:', error);
            showToast('error', 'Parse Error', 'Failed to parse file: ' + error.message);
        }
    };
    
    reader.onerror = (error) => {
        console.error('[UPLOAD] FileReader error:', error);
        showToast('error', 'Read Error', 'Failed to read file: ' + error.message);
    };
    
    reader.readAsText(file);
}

function handleConfigFileDrop(e) {
    e.preventDefault();
    e.stopPropagation();
    document.getElementById('upload-config-area').classList.remove('dragover');
    
    const files = e.dataTransfer.files;
    if (files.length > 0) {
        handleConfigFileUpload({ target: { files } });
    }
}

function handleConfigFileDropModal(e) {
    e.preventDefault();
    e.stopPropagation();
    document.getElementById('upload-config-modal-area').classList.remove('dragover');
    
    const files = e.dataTransfer.files;
    if (files.length > 0) {
        handleConfigFileUploadModal({ target: { files } });
    }
}

function handleConfigFileUpload(e) {
    const file = e.target.files[0];
    if (!file) return;
    
    // Validate file type
    const validTypes = ['application/json', 'text/yaml', 'text/plain'];
    if (!validTypes.includes(file.type) && !file.name.match(/\.(json|yaml|yml)$/)) {
        showToast('error', 'Invalid File', 'Please upload a JSON or YAML configuration file');
        return;
    }
    
    // Read file
    const reader = new FileReader();
    reader.onload = (event) => {
        try {
            const content = event.target.result;
            // Parse as JSON
            const configData = JSON.parse(content);
            
            // Try to use the schema specified in config, or first available schema
            let schemaId = configData.schemaId || (AppState.schemas.length > 0 ? AppState.schemas[0].id : null);
            
            if (!schemaId) {
                showToast('error', 'No Schema', 'No schema available. Please load a schema first.');
                return;
            }
            
            // Load the schema first
            AppState.selectedSchema = schemaId;
            const schema = AppState.schemas.find(s => s.id === schemaId);
            if (schema) {
                AppState.selectedSchemaData = schema;
            }
            
            // Load config data
            AppState.selectedConfigData = configData;
            AppState.formData = configData.data || {};
            
            // Show form with config data populated
            AppState.setState('form-generator');
            
            showToast('success', 'Configuration Loaded', `Loaded configuration from ${file.name}`);
        } catch (error) {
            console.error('Error parsing config file:', error);
            showToast('error', 'Parse Error', 'Failed to parse configuration file: ' + error.message);
        }
    };
    
    reader.readAsText(file);
}

function handleConfigFileUploadModal(e) {
    const file = e.target.files[0];
    if (!file) return;
    
    // Validate file type
    const validTypes = ['application/json', 'text/yaml', 'text/plain'];
    if (!validTypes.includes(file.type) && !file.name.match(/\.(json|yaml|yml)$/)) {
        showToast('error', 'Invalid File', 'Please upload a JSON or YAML configuration file');
        return;
    }
    
    // Read file
    const reader = new FileReader();
    reader.onload = async (event) => {
        try {
            const content = event.target.result;
            // Parse as JSON
            const configData = JSON.parse(content);
            
            // Generate unique ID for this config
            const configId = configData.id || `uploaded-${Date.now()}`;
            
            // Add config to AppState.configs so selectConfig can find it
            const configEntry = {
                id: configId,
                filename: file.name,
                schemaName: configData.schemaName || 'Unknown Schema',
                schemaId: configData.schemaId,
                timestamp: new Date().toISOString(),
                ...configData  // Include all config data
            };
            
            AppState.configs.push(configEntry);
            console.log('[CONFIG] Added config to AppState.configs:', configEntry);
            
            // Close modal
            const modal = document.getElementById('config-modal');
            if (modal) {
                modal.classList.add('hidden');
            }
            
            // Load the config (if we're on form, will use same schema if match)
            await selectConfig(configId);
            
            showToast('success', 'Configuration Loaded', `Loaded configuration from ${file.name}`);
        } catch (error) {
            console.error('Error parsing config file:', error);
            showToast('error', 'Parse Error', 'Failed to parse configuration file: ' + error.message);
        }
    };
    
    reader.readAsText(file);
}

// ============================================================================
// Utilities
// ============================================================================

function updateServerInfo() {
    const status = document.getElementById('connection-status');
    if (status) {
        status.classList.remove('offline');
        status.classList.add('online');
        status.textContent = '● Online';
    }
}

function formatFieldName(fieldName) {
    return fieldName
        .replace(/([A-Z])/g, ' $1')
        .replace(/_/g, ' ')
        .replace(/^\s+/, '')
        .split(' ')
        .map(word => word.charAt(0).toUpperCase() + word.slice(1).toLowerCase())
        .join(' ');
}

function escapeHtml(text) {
    if (!text) return '';
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

/**
 * Show a confirmation dialog with multiple button options
 * @param {string} title - Dialog title
 * @param {string} message - Dialog message
 * @param {Array} buttons - Array of {label, action} objects
 */
function showConfirmationDialog(title, message, buttons) {
    // Create modal overlay
    const modal = document.createElement('div');
    modal.style.cssText = `
        position: fixed;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background: rgba(0, 0, 0, 0.5);
        display: flex;
        align-items: center;
        justify-content: center;
        z-index: 10000;
    `;
    
    // Create dialog
    const dialog = document.createElement('div');
    dialog.style.cssText = `
        background: white;
        border-radius: 8px;
        padding: 24px;
        max-width: 500px;
        box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.1);
        font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
    `;
    
    // Title
    const titleEl = document.createElement('h2');
    titleEl.style.cssText = 'margin: 0 0 12px 0; font-size: 1.25rem; color: #2d3748;';
    titleEl.textContent = title;
    dialog.appendChild(titleEl);
    
    // Message
    const messageEl = document.createElement('p');
    messageEl.style.cssText = 'margin: 0 0 20px 0; color: #718096; line-height: 1.5; white-space: pre-wrap;';
    messageEl.textContent = message;
    dialog.appendChild(messageEl);
    
    // Buttons container
    const buttonContainer = document.createElement('div');
    buttonContainer.style.cssText = 'display: flex; gap: 8px; justify-content: flex-end;';
    
    // Add buttons
    buttons.forEach(btn => {
        const button = document.createElement('button');
        button.textContent = btn.label;
        button.style.cssText = `
            padding: 8px 16px;
            border-radius: 4px;
            border: 1px solid #e2e8f0;
            background: #f7fafc;
            color: #2d3748;
            cursor: pointer;
            font-weight: 500;
            transition: all 0.2s;
        `;
        
        // Highlight primary action (usually first button)
        if (buttons.indexOf(btn) === 0) {
            button.style.cssText += `
                background: #667eea;
                color: white;
                border-color: #667eea;
            `;
        }
        
        button.onmouseover = () => {
            button.style.opacity = '0.8';
        };
        button.onmouseout = () => {
            button.style.opacity = '1';
        };
        
        button.addEventListener('click', () => {
            modal.remove();
            btn.action();
        });
        
        buttonContainer.appendChild(button);
    });
    
    dialog.appendChild(buttonContainer);
    modal.appendChild(dialog);
    document.body.appendChild(modal);
}

/**
 * Save configuration with file selection dialog
 * @param {Object} payload - Configuration payload to save
 * @param {boolean} isOverwrite - Whether this is an overwrite operation
 */
async function saveConfigWithFileDialog(payload, isOverwrite) {
    try {
        // Since browser cannot directly save files, we'll send to server
        // which will handle the file save and return download link
        const response = await fetch('/api/configs/save', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                ...payload,
                overwrite: isOverwrite
            })
        });
        
        if (!response.ok) {
            throw new Error('Failed to save configuration on server');
        }
        
        const result = await response.json();
        showToast('success', 'Saved', `Configuration saved to: ${result.filename || 'config.json'}`);
        
    } catch (error) {
        showError('Failed to save configuration: ' + error.message);
    }
}

/**
 * Save configuration directly to server with specific file path
 * @param {Object} payload - Configuration payload to save
 * @param {string} filepath - Target file path
 * @param {boolean} overwrite - Whether to overwrite existing file
 */
async function saveConfigToServer(payload, filepath, overwrite) {
    try {
        const response = await fetch('/api/configs', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                ...payload,
                filepath,
                overwrite
            })
        });
        
        if (!response.ok) {
            throw new Error('Failed to save configuration on server');
        }
        
        const result = await response.json();
        showToast('success', 'Saved', `Configuration saved successfully`);
        
    } catch (error) {
        showError('Failed to save configuration: ' + error.message);
    }
}

function showError(message) {
    document.getElementById('error-message').textContent = message;
}

function showToast(type, title, message) {
    const container = document.getElementById('toast-container');
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    
    const iconMap = {
        success: '✅',
        error: '❌',
        info: 'ℹ️'
    };
    
    toast.innerHTML = `
        <div class="toast-icon">${iconMap[type] || '•'}</div>
        <div class="toast-content">
            <div class="toast-title">${escapeHtml(title)}</div>
            <div class="toast-message">${escapeHtml(message)}</div>
        </div>
    `;
    
    container.appendChild(toast);
    
    // Auto-remove after 5 seconds
    setTimeout(() => {
        toast.style.animation = 'slideIn 300ms ease-out reverse';
        setTimeout(() => {
            toast.remove();
        }, 300);
    }, 5000);
}

// ============================================================================
// Error Handling
// ============================================================================

window.addEventListener('error', (event) => {
    console.error('Uncaught error:', event.error);
});

window.addEventListener('unhandledrejection', (event) => {
    console.error('Unhandled promise rejection:', event.reason);
});
