/**
 * @file TextField.hpp
 * @brief TextField widget for KillerGK with Builder Pattern API
 */

#pragma once

#include "Widget.hpp"
#include "../core/Types.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace KillerGK {

/**
 * @struct TextSelection
 * @brief Represents text selection range
 */
struct TextSelection {
    size_t start = 0;
    size_t end = 0;
    
    [[nodiscard]] bool hasSelection() const { return start != end; }
    [[nodiscard]] size_t length() const { return end > start ? end - start : start - end; }
    [[nodiscard]] size_t min() const { return start < end ? start : end; }
    [[nodiscard]] size_t max() const { return start > end ? start : end; }
};

/**
 * @struct UndoEntry
 * @brief Entry in the undo/redo history
 */
struct UndoEntry {
    std::string text;
    size_t cursorPos = 0;
    TextSelection selection;
};

/**
 * @class TextField
 * @brief Text input widget with selection, copy/paste, and undo/redo support
 * 
 * Provides a full-featured text input with placeholder text, labels,
 * multiline support, password masking, and prefix/suffix widgets.
 * 
 * Example:
 * @code
 * auto textField = TextField::create()
 *     .placeholder("Enter your name")
 *     .label("Name")
 *     .maxLength(100)
 *     .onChange([](const std::string& text) { 
 *         std::cout << "Text changed: " << text; 
 *     });
 * @endcode
 */
class TextField : public Widget {
public:
    virtual ~TextField() = default;

    /**
     * @brief Create a new TextField instance
     * @return New TextField with default properties
     */
    static TextField create();

    // =========================================================================
    // Text Content
    // =========================================================================

    /**
     * @brief Set text content
     * @param text Text to set
     * @return Reference to this TextField for chaining
     */
    TextField& text(const std::string& text);

    /**
     * @brief Get current text content
     * @return Current text
     */
    [[nodiscard]] const std::string& getText() const;

    /**
     * @brief Set placeholder text
     * @param placeholder Placeholder text shown when empty
     * @return Reference to this TextField for chaining
     */
    TextField& placeholder(const std::string& placeholder);

    /**
     * @brief Get placeholder text
     * @return Current placeholder text
     */
    [[nodiscard]] const std::string& getPlaceholder() const;

    /**
     * @brief Set label text
     * @param label Label text shown above the field
     * @return Reference to this TextField for chaining
     */
    TextField& label(const std::string& label);

    /**
     * @brief Get label text
     * @return Current label text
     */
    [[nodiscard]] const std::string& getLabel() const;

    // =========================================================================
    // Configuration
    // =========================================================================

    /**
     * @brief Enable or disable multiline mode
     * @param enabled Whether multiline is enabled
     * @return Reference to this TextField for chaining
     */
    TextField& multiline(bool enabled);

    /**
     * @brief Check if multiline mode is enabled
     * @return true if multiline
     */
    [[nodiscard]] bool isMultiline() const;

    /**
     * @brief Set maximum text length
     * @param length Maximum number of characters (0 = unlimited)
     * @return Reference to this TextField for chaining
     */
    TextField& maxLength(int length);

    /**
     * @brief Get maximum text length
     * @return Maximum length (0 = unlimited)
     */
    [[nodiscard]] int getMaxLength() const;

    /**
     * @brief Enable or disable password mode
     * @param isPassword Whether to mask text
     * @return Reference to this TextField for chaining
     */
    TextField& password(bool isPassword);

    /**
     * @brief Check if password mode is enabled
     * @return true if password mode
     */
    [[nodiscard]] bool isPassword() const;

    /**
     * @brief Set password mask character
     * @param maskChar Character to use for masking
     * @return Reference to this TextField for chaining
     */
    TextField& passwordChar(char maskChar);

    /**
     * @brief Get password mask character
     * @return Current mask character
     */
    [[nodiscard]] char getPasswordChar() const;

    /**
     * @brief Enable or disable read-only mode
     * @param readOnly Whether field is read-only
     * @return Reference to this TextField for chaining
     */
    TextField& readOnly(bool readOnly);

    /**
     * @brief Check if read-only mode is enabled
     * @return true if read-only
     */
    [[nodiscard]] bool isReadOnly() const;

    // =========================================================================
    // Prefix/Suffix Widgets
    // =========================================================================

    /**
     * @brief Set prefix widget
     * @param widget Widget to display before text
     * @return Reference to this TextField for chaining
     */
    TextField& prefix(Widget* widget);

    /**
     * @brief Get prefix widget
     * @return Prefix widget or nullptr
     */
    [[nodiscard]] Widget* getPrefix() const;

    /**
     * @brief Set suffix widget
     * @param widget Widget to display after text
     * @return Reference to this TextField for chaining
     */
    TextField& suffix(Widget* widget);

    /**
     * @brief Get suffix widget
     * @return Suffix widget or nullptr
     */
    [[nodiscard]] Widget* getSuffix() const;

    // =========================================================================
    // Cursor and Selection
    // =========================================================================

    /**
     * @brief Set cursor position
     * @param pos Cursor position (character index)
     * @return Reference to this TextField for chaining
     */
    TextField& cursorPosition(size_t pos);

    /**
     * @brief Get cursor position
     * @return Current cursor position
     */
    [[nodiscard]] size_t getCursorPosition() const;

    /**
     * @brief Set text selection
     * @param start Selection start position
     * @param end Selection end position
     * @return Reference to this TextField for chaining
     */
    TextField& selection(size_t start, size_t end);

    /**
     * @brief Get current selection
     * @return Current selection range
     */
    [[nodiscard]] const TextSelection& getSelection() const;

    /**
     * @brief Select all text
     */
    void selectAll();

    /**
     * @brief Clear selection
     */
    void clearSelection();

    /**
     * @brief Get selected text
     * @return Selected text or empty string
     */
    [[nodiscard]] std::string getSelectedText() const;

    // =========================================================================
    // Text Operations
    // =========================================================================

    /**
     * @brief Insert text at cursor position
     * @param text Text to insert
     */
    void insertText(const std::string& text);

    /**
     * @brief Delete selected text or character at cursor
     * @param forward If true, delete forward; if false, delete backward
     */
    void deleteText(bool forward = true);

    /**
     * @brief Copy selected text to clipboard
     */
    void copy();

    /**
     * @brief Cut selected text to clipboard
     */
    void cut();

    /**
     * @brief Paste text from clipboard
     */
    void paste();

    /**
     * @brief Undo last operation
     */
    void undo();

    /**
     * @brief Redo last undone operation
     */
    void redo();

    /**
     * @brief Check if undo is available
     * @return true if can undo
     */
    [[nodiscard]] bool canUndo() const;

    /**
     * @brief Check if redo is available
     * @return true if can redo
     */
    [[nodiscard]] bool canRedo() const;

    // =========================================================================
    // Styling
    // =========================================================================

    /**
     * @brief Set text color
     * @param color Text color
     * @return Reference to this TextField for chaining
     */
    TextField& textColor(const Color& color);

    /**
     * @brief Get text color
     * @return Current text color
     */
    [[nodiscard]] const Color& getTextColor() const;

    /**
     * @brief Set placeholder text color
     * @param color Placeholder color
     * @return Reference to this TextField for chaining
     */
    TextField& placeholderColor(const Color& color);

    /**
     * @brief Get placeholder text color
     * @return Current placeholder color
     */
    [[nodiscard]] const Color& getPlaceholderColor() const;

    /**
     * @brief Set selection highlight color
     * @param color Selection color
     * @return Reference to this TextField for chaining
     */
    TextField& selectionColor(const Color& color);

    /**
     * @brief Get selection highlight color
     * @return Current selection color
     */
    [[nodiscard]] const Color& getSelectionColor() const;

    /**
     * @brief Set cursor color
     * @param color Cursor color
     * @return Reference to this TextField for chaining
     */
    TextField& cursorColor(const Color& color);

    /**
     * @brief Get cursor color
     * @return Current cursor color
     */
    [[nodiscard]] const Color& getCursorColor() const;

    // =========================================================================
    // Event Callbacks
    // =========================================================================

    /**
     * @brief Set callback for text changes
     * @param callback Function called when text changes
     * @return Reference to this TextField for chaining
     */
    TextField& onChange(std::function<void(const std::string&)> callback);

    /**
     * @brief Set callback for submit (Enter key)
     * @param callback Function called on submit
     * @return Reference to this TextField for chaining
     */
    TextField& onSubmit(std::function<void(const std::string&)> callback);

    // =========================================================================
    // Display Text
    // =========================================================================

    /**
     * @brief Get display text (masked if password mode)
     * @return Text to display
     */
    [[nodiscard]] std::string getDisplayText() const;

protected:
    TextField();

    /**
     * @brief Save current state to undo history
     */
    void saveUndoState();

    struct TextFieldData;
    std::shared_ptr<TextFieldData> m_textFieldData;
};

} // namespace KillerGK
