/**
 * @file SyntaxHighlighter.hpp
 * @brief Syntax highlighting for code display
 * 
 * Provides token-based syntax highlighting for multiple
 * programming languages.
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <regex>

namespace KillerGK {

/**
 * @brief Token types for syntax highlighting
 */
enum class TokenType {
    Plain,          ///< Regular text
    Keyword,        ///< Language keyword
    Type,           ///< Type name
    String,         ///< String literal
    Number,         ///< Numeric literal
    Comment,        ///< Comment
    Operator,       ///< Operator
    Punctuation,    ///< Punctuation
    Function,       ///< Function name
    Variable,       ///< Variable name
    Constant,       ///< Constant value
    Preprocessor,   ///< Preprocessor directive
    Attribute,      ///< Attribute/annotation
    Tag,            ///< HTML/XML tag
    TagAttribute,   ///< HTML/XML attribute
    Regex,          ///< Regular expression
    Error           ///< Error/invalid token
};

/**
 * @brief A highlighted token
 */
struct Token {
    TokenType type = TokenType::Plain;
    std::string text;
    size_t start = 0;
    size_t length = 0;
    int line = 0;
    int column = 0;
};

/**
 * @brief Color scheme for syntax highlighting
 */
struct SyntaxColorScheme {
    Color plain = Color::White;
    Color keyword = Color(0.86f, 0.44f, 0.58f);      // Pink
    Color type = Color(0.4f, 0.85f, 0.94f);          // Cyan
    Color string = Color(0.6f, 0.8f, 0.4f);          // Green
    Color number = Color(0.68f, 0.51f, 0.98f);       // Purple
    Color comment = Color(0.5f, 0.5f, 0.5f);         // Gray
    Color operatorColor = Color(0.86f, 0.44f, 0.58f);
    Color punctuation = Color::White;
    Color function = Color(0.4f, 0.7f, 0.98f);       // Blue
    Color variable = Color::White;
    Color constant = Color(0.68f, 0.51f, 0.98f);
    Color preprocessor = Color(0.86f, 0.44f, 0.58f);
    Color attribute = Color(0.6f, 0.8f, 0.4f);
    Color tag = Color(0.86f, 0.44f, 0.58f);
    Color tagAttribute = Color(0.4f, 0.85f, 0.94f);
    Color regex = Color(0.6f, 0.8f, 0.4f);
    Color error = Color::Red;
    Color background = Color(0.12f, 0.12f, 0.12f);
    Color lineNumber = Color(0.5f, 0.5f, 0.5f);
    Color selection = Color(0.26f, 0.26f, 0.26f);
    Color currentLine = Color(0.16f, 0.16f, 0.16f);
    
    /**
     * @brief Get color for token type
     */
    [[nodiscard]] Color getColor(TokenType type) const;
    
    /**
     * @brief Create dark theme
     */
    static SyntaxColorScheme dark();
    
    /**
     * @brief Create light theme
     */
    static SyntaxColorScheme light();
    
    /**
     * @brief Create Monokai theme
     */
    static SyntaxColorScheme monokai();
};

/**
 * @brief Language definition for syntax highlighting
 */
struct LanguageDefinition {
    std::string name;
    std::vector<std::string> extensions;
    std::vector<std::string> keywords;
    std::vector<std::string> types;
    std::vector<std::string> constants;
    std::string lineComment;
    std::string blockCommentStart;
    std::string blockCommentEnd;
    std::string stringDelimiters;
    bool caseSensitive = true;
};

/**
 * @class SyntaxHighlighter
 * @brief Tokenizes and highlights source code
 */
class SyntaxHighlighter {
public:
    SyntaxHighlighter();
    ~SyntaxHighlighter();
    
    /**
     * @brief Set the language for highlighting
     * @param language Language name (e.g., "cpp", "python", "javascript")
     */
    void setLanguage(const std::string& language);
    
    /**
     * @brief Detect language from file extension
     * @param filename Filename or path
     * @return Detected language name
     */
    static std::string detectLanguage(const std::string& filename);
    
    /**
     * @brief Set color scheme
     * @param scheme Color scheme to use
     */
    void setColorScheme(const SyntaxColorScheme& scheme);
    
    /**
     * @brief Get current color scheme
     */
    [[nodiscard]] const SyntaxColorScheme& getColorScheme() const;
    
    /**
     * @brief Tokenize source code
     * @param code Source code to tokenize
     * @return Vector of tokens
     */
    std::vector<Token> tokenize(const std::string& code);
    
    /**
     * @brief Highlight a single line
     * @param line Line of code
     * @param lineNumber Line number (for context)
     * @return Vector of tokens for the line
     */
    std::vector<Token> highlightLine(const std::string& line, int lineNumber = 0);
    
    /**
     * @brief Get available languages
     * @return List of supported language names
     */
    static std::vector<std::string> getAvailableLanguages();
    
    /**
     * @brief Register a custom language
     * @param definition Language definition
     */
    static void registerLanguage(const LanguageDefinition& definition);
    
    /**
     * @brief Get language definition
     * @param language Language name
     * @return Language definition or nullptr
     */
    static const LanguageDefinition* getLanguageDefinition(const std::string& language);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    // Built-in language definitions
    static void initializeBuiltinLanguages();
    static std::unordered_map<std::string, LanguageDefinition> s_languages;
    static bool s_languagesInitialized;
};

} // namespace KillerGK
