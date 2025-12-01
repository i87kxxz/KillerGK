/**
 * @file RichText.hpp
 * @brief Rich text editing and formatting for KillerGK
 * 
 * Provides rich text support with formatting, embedded images,
 * and embedded widgets.
 */

#pragma once

#include "../core/Types.hpp"
#include "Font.hpp"
#include "TextRenderer.hpp"
#include <memory>
#include <string>
#include <vector>
#include <variant>

namespace KillerGK {

// Forward declarations
class Widget;

/**
 * @brief Text formatting attributes
 */
struct TextFormat {
    FontHandle font;
    float fontSize = 16.0f;
    Color color = Color::White;
    Color backgroundColor = Color::Transparent;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikethrough = false;
    bool superscript = false;
    bool subscript = false;
    std::string link;  ///< URL if this is a hyperlink
};

/**
 * @brief A span of formatted text
 */
struct TextSpan {
    std::string text;
    TextFormat format;
    size_t start = 0;   ///< Start position in document
    size_t length = 0;  ///< Length in characters
};

/**
 * @brief An embedded image in rich text
 */
struct EmbeddedImage {
    std::string path;
    float width = 0.0f;   ///< 0 = auto
    float height = 0.0f;  ///< 0 = auto
    std::string altText;
    size_t position = 0;  ///< Position in document
};

/**
 * @brief An embedded widget in rich text
 */
struct EmbeddedWidget {
    std::shared_ptr<Widget> widget;
    float width = 0.0f;
    float height = 0.0f;
    size_t position = 0;
};

/**
 * @brief Rich text element (text span, image, or widget)
 */
using RichTextElement = std::variant<TextSpan, EmbeddedImage, EmbeddedWidget>;

/**
 * @brief Paragraph alignment
 */
enum class ParagraphAlign {
    Left,
    Center,
    Right,
    Justify
};

/**
 * @brief A paragraph in rich text
 */
struct Paragraph {
    std::vector<RichTextElement> elements;
    ParagraphAlign alignment = ParagraphAlign::Left;
    float lineHeight = 1.2f;
    float marginTop = 0.0f;
    float marginBottom = 8.0f;
    float indentFirst = 0.0f;   ///< First line indent
    float indentLeft = 0.0f;    ///< Left indent
    float indentRight = 0.0f;   ///< Right indent
    
    // List properties
    bool isList = false;
    bool isOrdered = false;
    int listLevel = 0;
    int listIndex = 0;
};

/**
 * @brief Selection range in rich text
 */
struct TextSelection {
    size_t start = 0;
    size_t end = 0;
    
    [[nodiscard]] bool isEmpty() const { return start == end; }
    [[nodiscard]] size_t length() const { return (end > start) ? (end - start) : 0; }
    [[nodiscard]] size_t min() const { return std::min(start, end); }
    [[nodiscard]] size_t max() const { return std::max(start, end); }
};

/**
 * @class RichTextDocument
 * @brief A rich text document with formatting
 */
class RichTextDocument {
public:
    RichTextDocument() = default;
    
    /**
     * @brief Create from plain text
     * @param text Plain text content
     * @param defaultFormat Default text format
     */
    static RichTextDocument fromPlainText(const std::string& text,
                                           const TextFormat& defaultFormat = TextFormat{});
    
    /**
     * @brief Create from HTML
     * @param html HTML content
     * @return Parsed document
     */
    static RichTextDocument fromHTML(const std::string& html);
    
    /**
     * @brief Export to plain text
     * @return Plain text content
     */
    [[nodiscard]] std::string toPlainText() const;
    
    /**
     * @brief Export to HTML
     * @return HTML content
     */
    [[nodiscard]] std::string toHTML() const;
    
    // Content manipulation
    void insertText(size_t position, const std::string& text, 
                    const TextFormat& format = TextFormat{});
    void deleteText(size_t start, size_t length);
    void replaceText(size_t start, size_t length, const std::string& text,
                     const TextFormat& format = TextFormat{});
    
    // Formatting
    void applyFormat(size_t start, size_t length, const TextFormat& format);
    void toggleBold(size_t start, size_t length);
    void toggleItalic(size_t start, size_t length);
    void toggleUnderline(size_t start, size_t length);
    void toggleStrikethrough(size_t start, size_t length);
    void setColor(size_t start, size_t length, const Color& color);
    void setFontSize(size_t start, size_t length, float size);
    void setFont(size_t start, size_t length, FontHandle font);
    void setLink(size_t start, size_t length, const std::string& url);
    
    // Paragraph formatting
    void setParagraphAlignment(size_t paragraphIndex, ParagraphAlign align);
    void setParagraphIndent(size_t paragraphIndex, float first, float left, float right);
    void setParagraphLineHeight(size_t paragraphIndex, float lineHeight);
    
    // Embedded content
    void insertImage(size_t position, const std::string& path, 
                     float width = 0, float height = 0);
    void insertWidget(size_t position, std::shared_ptr<Widget> widget,
                      float width = 0, float height = 0);
    
    // Queries
    [[nodiscard]] TextFormat getFormatAt(size_t position) const;
    [[nodiscard]] size_t getLength() const;
    [[nodiscard]] size_t getParagraphCount() const { return m_paragraphs.size(); }
    [[nodiscard]] const Paragraph& getParagraph(size_t index) const;
    [[nodiscard]] size_t getParagraphAtPosition(size_t position) const;
    
    // Undo/Redo
    void undo();
    void redo();
    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    void beginUndoGroup();
    void endUndoGroup();
    
private:
    std::vector<Paragraph> m_paragraphs;
    TextFormat m_defaultFormat;
    
    // Undo/redo state
    struct UndoState {
        std::vector<Paragraph> paragraphs;
    };
    std::vector<UndoState> m_undoStack;
    std::vector<UndoState> m_redoStack;
    bool m_inUndoGroup = false;
    
    void saveUndoState();
    void rebuildFromText();
};

/**
 * @class RichTextEditor
 * @brief Widget for editing rich text
 */
class RichTextEditor {
public:
    RichTextEditor();
    ~RichTextEditor();
    
    /**
     * @brief Set the document to edit
     * @param document Document to edit
     */
    void setDocument(std::shared_ptr<RichTextDocument> document);
    
    /**
     * @brief Get the current document
     * @return Current document
     */
    [[nodiscard]] std::shared_ptr<RichTextDocument> getDocument() const;
    
    /**
     * @brief Set the editing bounds
     * @param bounds Editing area bounds
     */
    void setBounds(const Rect& bounds);
    
    /**
     * @brief Get current selection
     * @return Current selection
     */
    [[nodiscard]] TextSelection getSelection() const;
    
    /**
     * @brief Set selection
     * @param selection New selection
     */
    void setSelection(const TextSelection& selection);
    
    /**
     * @brief Select all text
     */
    void selectAll();
    
    /**
     * @brief Get cursor position
     * @return Cursor position
     */
    [[nodiscard]] size_t getCursorPosition() const;
    
    /**
     * @brief Set cursor position
     * @param position New cursor position
     */
    void setCursorPosition(size_t position);
    
    // Editing operations
    void cut();
    void copy();
    void paste();
    void deleteSelection();
    
    // Input handling
    void handleKeyPress(uint32_t keyCode, bool shift, bool ctrl, bool alt);
    void handleTextInput(const std::string& text);
    void handleMouseDown(float x, float y, bool shift);
    void handleMouseMove(float x, float y);
    void handleMouseUp(float x, float y);
    
    // Rendering
    void render();
    
    // Callbacks
    using ChangeCallback = std::function<void()>;
    using SelectionCallback = std::function<void(const TextSelection&)>;
    using LinkCallback = std::function<void(const std::string&)>;
    
    void setOnChange(ChangeCallback callback);
    void setOnSelectionChange(SelectionCallback callback);
    void setOnLinkClick(LinkCallback callback);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace KillerGK
