/**
 * @file SyntaxHighlighter.cpp
 * @brief Syntax highlighting implementation
 */

#include "KillerGK/text/SyntaxHighlighter.hpp"
#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace KillerGK {

// Static members
std::unordered_map<std::string, LanguageDefinition> SyntaxHighlighter::s_languages;
bool SyntaxHighlighter::s_languagesInitialized = false;

// ============================================================================
// SyntaxColorScheme Implementation
// ============================================================================

Color SyntaxColorScheme::getColor(TokenType tokenType) const {
    switch (tokenType) {
        case TokenType::Keyword: return keyword;
        case TokenType::Type: return type;
        case TokenType::String: return string;
        case TokenType::Number: return number;
        case TokenType::Comment: return comment;
        case TokenType::Operator: return operatorColor;
        case TokenType::Punctuation: return punctuation;
        case TokenType::Function: return function;
        case TokenType::Variable: return variable;
        case TokenType::Constant: return constant;
        case TokenType::Preprocessor: return preprocessor;
        case TokenType::Attribute: return attribute;
        case TokenType::Tag: return tag;
        case TokenType::TagAttribute: return tagAttribute;
        case TokenType::Regex: return regex;
        case TokenType::Error: return error;
        default: return plain;
    }
}

SyntaxColorScheme SyntaxColorScheme::dark() {
    return SyntaxColorScheme{};  // Default is dark
}

SyntaxColorScheme SyntaxColorScheme::light() {
    SyntaxColorScheme scheme;
    scheme.plain = Color(0.2f, 0.2f, 0.2f);
    scheme.keyword = Color(0.6f, 0.1f, 0.6f);
    scheme.type = Color(0.0f, 0.5f, 0.5f);
    scheme.string = Color(0.2f, 0.5f, 0.2f);
    scheme.number = Color(0.0f, 0.4f, 0.8f);
    scheme.comment = Color(0.5f, 0.5f, 0.5f);
    scheme.function = Color(0.0f, 0.4f, 0.8f);
    scheme.background = Color(1.0f, 1.0f, 1.0f);
    scheme.lineNumber = Color(0.6f, 0.6f, 0.6f);
    scheme.selection = Color(0.8f, 0.9f, 1.0f);
    scheme.currentLine = Color(0.95f, 0.95f, 0.95f);
    return scheme;
}

SyntaxColorScheme SyntaxColorScheme::monokai() {
    SyntaxColorScheme scheme;
    scheme.plain = Color(0.97f, 0.97f, 0.95f);
    scheme.keyword = Color(0.98f, 0.15f, 0.45f);
    scheme.type = Color(0.4f, 0.85f, 0.94f);
    scheme.string = Color(0.9f, 0.86f, 0.45f);
    scheme.number = Color(0.68f, 0.51f, 0.98f);
    scheme.comment = Color(0.46f, 0.44f, 0.37f);
    scheme.function = Color(0.65f, 0.89f, 0.18f);
    scheme.background = Color(0.15f, 0.16f, 0.13f);
    return scheme;
}

// ============================================================================
// Built-in Language Definitions
// ============================================================================

void SyntaxHighlighter::initializeBuiltinLanguages() {
    if (s_languagesInitialized) return;
    
    // C++
    LanguageDefinition cpp;
    cpp.name = "cpp";
    cpp.extensions = {".cpp", ".hpp", ".h", ".cc", ".cxx", ".hxx", ".c"};
    cpp.keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
        "break", "case", "catch", "class", "compl", "concept", "const", "consteval",
        "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return",
        "co_yield", "decltype", "default", "delete", "do", "dynamic_cast", "else",
        "enum", "explicit", "export", "extern", "false", "for", "friend", "goto",
        "if", "inline", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
        "nullptr", "operator", "or", "or_eq", "private", "protected", "public",
        "register", "reinterpret_cast", "requires", "return", "sizeof", "static",
        "static_assert", "static_cast", "struct", "switch", "template", "this",
        "thread_local", "throw", "true", "try", "typedef", "typeid", "typename",
        "union", "using", "virtual", "volatile", "while", "xor", "xor_eq"
    };
    cpp.types = {
        "bool", "char", "char8_t", "char16_t", "char32_t", "double", "float",
        "int", "long", "short", "signed", "unsigned", "void", "wchar_t",
        "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t",
        "uint32_t", "uint64_t", "size_t", "ptrdiff_t", "string", "vector",
        "map", "set", "unordered_map", "unordered_set", "array", "list",
        "deque", "queue", "stack", "pair", "tuple", "optional", "variant",
        "any", "shared_ptr", "unique_ptr", "weak_ptr"
    };
    cpp.constants = {"NULL", "nullptr", "true", "false"};
    cpp.lineComment = "//";
    cpp.blockCommentStart = "/*";
    cpp.blockCommentEnd = "*/";
    cpp.stringDelimiters = "\"'";
    s_languages["cpp"] = cpp;
    s_languages["c"] = cpp;
    
    // JavaScript
    LanguageDefinition js;
    js.name = "javascript";
    js.extensions = {".js", ".jsx", ".mjs", ".ts", ".tsx"};
    js.keywords = {
        "async", "await", "break", "case", "catch", "class", "const", "continue",
        "debugger", "default", "delete", "do", "else", "export", "extends",
        "finally", "for", "function", "if", "import", "in", "instanceof", "let",
        "new", "of", "return", "static", "super", "switch", "this", "throw",
        "try", "typeof", "var", "void", "while", "with", "yield"
    };
    js.types = {
        "Array", "Boolean", "Date", "Error", "Function", "Map", "Number",
        "Object", "Promise", "RegExp", "Set", "String", "Symbol", "WeakMap",
        "WeakSet"
    };
    js.constants = {"true", "false", "null", "undefined", "NaN", "Infinity"};
    js.lineComment = "//";
    js.blockCommentStart = "/*";
    js.blockCommentEnd = "*/";
    js.stringDelimiters = "\"'`";
    s_languages["javascript"] = js;
    s_languages["js"] = js;
    s_languages["typescript"] = js;
    s_languages["ts"] = js;
    
    // Python
    LanguageDefinition python;
    python.name = "python";
    python.extensions = {".py", ".pyw", ".pyi"};
    python.keywords = {
        "and", "as", "assert", "async", "await", "break", "class", "continue",
        "def", "del", "elif", "else", "except", "finally", "for", "from",
        "global", "if", "import", "in", "is", "lambda", "nonlocal", "not",
        "or", "pass", "raise", "return", "try", "while", "with", "yield"
    };
    python.types = {
        "bool", "bytes", "complex", "dict", "float", "frozenset", "int",
        "list", "object", "set", "str", "tuple", "type"
    };
    python.constants = {"True", "False", "None"};
    python.lineComment = "#";
    python.blockCommentStart = "\"\"\"";
    python.blockCommentEnd = "\"\"\"";
    python.stringDelimiters = "\"'";
    s_languages["python"] = python;
    s_languages["py"] = python;
    
    // HTML
    LanguageDefinition html;
    html.name = "html";
    html.extensions = {".html", ".htm", ".xhtml"};
    html.keywords = {};
    html.types = {};
    html.constants = {};
    html.lineComment = "";
    html.blockCommentStart = "<!--";
    html.blockCommentEnd = "-->";
    html.stringDelimiters = "\"'";
    s_languages["html"] = html;
    
    // JSON
    LanguageDefinition json;
    json.name = "json";
    json.extensions = {".json"};
    json.keywords = {};
    json.types = {};
    json.constants = {"true", "false", "null"};
    json.lineComment = "";
    json.blockCommentStart = "";
    json.blockCommentEnd = "";
    json.stringDelimiters = "\"";
    s_languages["json"] = json;
    
    s_languagesInitialized = true;
}

// ============================================================================
// SyntaxHighlighter Implementation
// ============================================================================

struct SyntaxHighlighter::Impl {
    std::string language;
    SyntaxColorScheme colorScheme;
    const LanguageDefinition* langDef = nullptr;
    std::unordered_set<std::string> keywordSet;
    std::unordered_set<std::string> typeSet;
    std::unordered_set<std::string> constantSet;
    
    // State for multi-line constructs
    bool inBlockComment = false;
    bool inString = false;
    char stringDelimiter = 0;
};

SyntaxHighlighter::SyntaxHighlighter() : m_impl(std::make_unique<Impl>()) {
    initializeBuiltinLanguages();
}

SyntaxHighlighter::~SyntaxHighlighter() = default;

void SyntaxHighlighter::setLanguage(const std::string& language) {
    m_impl->language = language;
    m_impl->langDef = getLanguageDefinition(language);
    
    m_impl->keywordSet.clear();
    m_impl->typeSet.clear();
    m_impl->constantSet.clear();
    
    if (m_impl->langDef) {
        for (const auto& kw : m_impl->langDef->keywords) {
            m_impl->keywordSet.insert(kw);
        }
        for (const auto& t : m_impl->langDef->types) {
            m_impl->typeSet.insert(t);
        }
        for (const auto& c : m_impl->langDef->constants) {
            m_impl->constantSet.insert(c);
        }
    }
}

std::string SyntaxHighlighter::detectLanguage(const std::string& filename) {
    initializeBuiltinLanguages();
    
    // Find extension
    size_t dotPos = filename.rfind('.');
    if (dotPos == std::string::npos) {
        return "plain";
    }
    
    std::string ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    for (const auto& [name, def] : s_languages) {
        for (const auto& langExt : def.extensions) {
            if (ext == langExt) {
                return name;
            }
        }
    }
    
    return "plain";
}

void SyntaxHighlighter::setColorScheme(const SyntaxColorScheme& scheme) {
    m_impl->colorScheme = scheme;
}

const SyntaxColorScheme& SyntaxHighlighter::getColorScheme() const {
    return m_impl->colorScheme;
}

std::vector<std::string> SyntaxHighlighter::getAvailableLanguages() {
    initializeBuiltinLanguages();
    
    std::vector<std::string> languages;
    for (const auto& [name, def] : s_languages) {
        if (name == def.name) {  // Only add primary names
            languages.push_back(name);
        }
    }
    return languages;
}

void SyntaxHighlighter::registerLanguage(const LanguageDefinition& definition) {
    s_languages[definition.name] = definition;
    for (const auto& ext : definition.extensions) {
        // Also register by extension for quick lookup
    }
}

const LanguageDefinition* SyntaxHighlighter::getLanguageDefinition(const std::string& language) {
    initializeBuiltinLanguages();
    
    auto it = s_languages.find(language);
    return (it != s_languages.end()) ? &it->second : nullptr;
}

std::vector<Token> SyntaxHighlighter::tokenize(const std::string& code) {
    std::vector<Token> tokens;
    
    // Split into lines and tokenize each
    std::string line;
    int lineNumber = 0;
    size_t globalPos = 0;
    
    for (size_t i = 0; i <= code.size(); ++i) {
        if (i == code.size() || code[i] == '\n') {
            auto lineTokens = highlightLine(line, lineNumber);
            
            // Adjust positions to global
            for (auto& token : lineTokens) {
                token.start += globalPos;
                tokens.push_back(token);
            }
            
            globalPos = i + 1;
            lineNumber++;
            line.clear();
        } else {
            line += code[i];
        }
    }
    
    return tokens;
}

std::vector<Token> SyntaxHighlighter::highlightLine(const std::string& line, int lineNumber) {
    std::vector<Token> tokens;
    
    if (line.empty()) {
        return tokens;
    }
    
    const LanguageDefinition* lang = m_impl->langDef;
    size_t i = 0;
    
    while (i < line.size()) {
        // Skip whitespace
        if (std::isspace(static_cast<unsigned char>(line[i]))) {
            size_t start = i;
            while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) {
                i++;
            }
            Token token;
            token.type = TokenType::Plain;
            token.text = line.substr(start, i - start);
            token.start = start;
            token.length = i - start;
            token.line = lineNumber;
            token.column = static_cast<int>(start);
            tokens.push_back(token);
            continue;
        }
        
        // Check for block comment continuation
        if (m_impl->inBlockComment && lang) {
            size_t endPos = line.find(lang->blockCommentEnd, i);
            Token token;
            token.type = TokenType::Comment;
            token.line = lineNumber;
            token.column = static_cast<int>(i);
            
            if (endPos != std::string::npos) {
                token.text = line.substr(i, endPos + lang->blockCommentEnd.size() - i);
                token.start = i;
                token.length = token.text.size();
                i = endPos + lang->blockCommentEnd.size();
                m_impl->inBlockComment = false;
            } else {
                token.text = line.substr(i);
                token.start = i;
                token.length = token.text.size();
                i = line.size();
            }
            tokens.push_back(token);
            continue;
        }
        
        // Check for line comment
        if (lang && !lang->lineComment.empty() &&
            line.compare(i, lang->lineComment.size(), lang->lineComment) == 0) {
            Token token;
            token.type = TokenType::Comment;
            token.text = line.substr(i);
            token.start = i;
            token.length = token.text.size();
            token.line = lineNumber;
            token.column = static_cast<int>(i);
            tokens.push_back(token);
            break;
        }
        
        // Check for block comment start
        if (lang && !lang->blockCommentStart.empty() &&
            line.compare(i, lang->blockCommentStart.size(), lang->blockCommentStart) == 0) {
            size_t endPos = line.find(lang->blockCommentEnd, i + lang->blockCommentStart.size());
            Token token;
            token.type = TokenType::Comment;
            token.line = lineNumber;
            token.column = static_cast<int>(i);
            
            if (endPos != std::string::npos) {
                token.text = line.substr(i, endPos + lang->blockCommentEnd.size() - i);
                token.start = i;
                token.length = token.text.size();
                i = endPos + lang->blockCommentEnd.size();
            } else {
                token.text = line.substr(i);
                token.start = i;
                token.length = token.text.size();
                i = line.size();
                m_impl->inBlockComment = true;
            }
            tokens.push_back(token);
            continue;
        }
        
        // Check for string
        if (lang && lang->stringDelimiters.find(line[i]) != std::string::npos) {
            char delim = line[i];
            size_t start = i;
            i++;
            
            while (i < line.size()) {
                if (line[i] == '\\' && i + 1 < line.size()) {
                    i += 2;  // Skip escape sequence
                } else if (line[i] == delim) {
                    i++;
                    break;
                } else {
                    i++;
                }
            }
            
            Token token;
            token.type = TokenType::String;
            token.text = line.substr(start, i - start);
            token.start = start;
            token.length = i - start;
            token.line = lineNumber;
            token.column = static_cast<int>(start);
            tokens.push_back(token);
            continue;
        }
        
        // Check for number
        if (std::isdigit(static_cast<unsigned char>(line[i])) ||
            (line[i] == '.' && i + 1 < line.size() && 
             std::isdigit(static_cast<unsigned char>(line[i + 1])))) {
            size_t start = i;
            
            // Handle hex, octal, binary
            if (line[i] == '0' && i + 1 < line.size()) {
                if (line[i + 1] == 'x' || line[i + 1] == 'X') {
                    i += 2;
                    while (i < line.size() && std::isxdigit(static_cast<unsigned char>(line[i]))) {
                        i++;
                    }
                } else if (line[i + 1] == 'b' || line[i + 1] == 'B') {
                    i += 2;
                    while (i < line.size() && (line[i] == '0' || line[i] == '1')) {
                        i++;
                    }
                }
            }
            
            // Regular number
            while (i < line.size() && (std::isdigit(static_cast<unsigned char>(line[i])) ||
                                        line[i] == '.' || line[i] == 'e' || line[i] == 'E' ||
                                        line[i] == '+' || line[i] == '-' ||
                                        line[i] == 'f' || line[i] == 'F' ||
                                        line[i] == 'l' || line[i] == 'L' ||
                                        line[i] == 'u' || line[i] == 'U')) {
                i++;
            }
            
            Token token;
            token.type = TokenType::Number;
            token.text = line.substr(start, i - start);
            token.start = start;
            token.length = i - start;
            token.line = lineNumber;
            token.column = static_cast<int>(start);
            tokens.push_back(token);
            continue;
        }
        
        // Check for identifier/keyword
        if (std::isalpha(static_cast<unsigned char>(line[i])) || line[i] == '_') {
            size_t start = i;
            while (i < line.size() && (std::isalnum(static_cast<unsigned char>(line[i])) || 
                                        line[i] == '_')) {
                i++;
            }
            
            std::string word = line.substr(start, i - start);
            Token token;
            token.text = word;
            token.start = start;
            token.length = i - start;
            token.line = lineNumber;
            token.column = static_cast<int>(start);
            
            // Determine token type
            if (m_impl->keywordSet.count(word)) {
                token.type = TokenType::Keyword;
            } else if (m_impl->typeSet.count(word)) {
                token.type = TokenType::Type;
            } else if (m_impl->constantSet.count(word)) {
                token.type = TokenType::Constant;
            } else if (i < line.size() && line[i] == '(') {
                token.type = TokenType::Function;
            } else {
                token.type = TokenType::Variable;
            }
            
            tokens.push_back(token);
            continue;
        }
        
        // Operators and punctuation
        Token token;
        token.start = i;
        token.line = lineNumber;
        token.column = static_cast<int>(i);
        
        // Multi-character operators
        if (i + 1 < line.size()) {
            std::string twoChar = line.substr(i, 2);
            if (twoChar == "==" || twoChar == "!=" || twoChar == "<=" || 
                twoChar == ">=" || twoChar == "&&" || twoChar == "||" ||
                twoChar == "++" || twoChar == "--" || twoChar == "+=" ||
                twoChar == "-=" || twoChar == "*=" || twoChar == "/=" ||
                twoChar == "<<" || twoChar == ">>" || twoChar == "->" ||
                twoChar == "::" || twoChar == "=>") {
                token.type = TokenType::Operator;
                token.text = twoChar;
                token.length = 2;
                tokens.push_back(token);
                i += 2;
                continue;
            }
        }
        
        // Single character
        char c = line[i];
        token.text = std::string(1, c);
        token.length = 1;
        
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
            c == '=' || c == '<' || c == '>' || c == '!' || c == '&' ||
            c == '|' || c == '^' || c == '~' || c == '?') {
            token.type = TokenType::Operator;
        } else {
            token.type = TokenType::Punctuation;
        }
        
        tokens.push_back(token);
        i++;
    }
    
    return tokens;
}

} // namespace KillerGK
