/**
 * @file TextField.cpp
 * @brief TextField widget implementation
 */

#include "KillerGK/widgets/TextField.hpp"
#include <algorithm>

namespace KillerGK {

// =============================================================================
// TextFieldData - Internal data structure
// =============================================================================

struct TextField::TextFieldData {
    std::string text;
    std::string placeholder;
    std::string label;
    
    bool isMultiline = false;
    int maxLength = 0;  // 0 = unlimited
    bool isPassword = false;
    char passwordChar = '*';
    bool isReadOnly = false;
    
    Widget* prefixWidget = nullptr;
    Widget* suffixWidget = nullptr;
    
    size_t cursorPos = 0;
    TextSelection selection;
    
    // Undo/Redo
    std::vector<UndoEntry> undoHistory;
    std::vector<UndoEntry> redoHistory;
    static constexpr size_t maxUndoHistory = 100;
    
    // Styling
    Color textColor = Color::Black;
    Color placeholderColor = Color(0.6f, 0.6f, 0.6f, 1.0f);
    Color selectionColor = Color(0.25f, 0.47f, 0.85f, 0.3f);
    Color cursorColor = Color::Black;
    
    // Callbacks
    std::function<void(const std::string&)> onChangeCallback;
    std::function<void(const std::string&)> onSubmitCallback;
    
    // Clipboard (simple in-memory implementation)
    static std::string clipboard;
};

std::string TextField::TextFieldData::clipboard;

// =============================================================================
// TextField Implementation
// =============================================================================

TextField::TextField() 
    : Widget()
    , m_textFieldData(std::make_shared<TextFieldData>()) 
{
    // Set default text field styling
    backgroundColor(Color::White);
    borderWidth(1.0f);
    borderColor(Color(0.8f, 0.8f, 0.8f, 1.0f));
    borderRadius(4.0f);
    padding(8.0f, 12.0f);
}

TextField TextField::create() {
    return TextField();
}

// Text Content
TextField& TextField::text(const std::string& text) {
    if (m_textFieldData->maxLength > 0 && 
        text.length() > static_cast<size_t>(m_textFieldData->maxLength)) {
        m_textFieldData->text = text.substr(0, m_textFieldData->maxLength);
    } else {
        m_textFieldData->text = text;
    }
    
    // Ensure cursor is within bounds
    if (m_textFieldData->cursorPos > m_textFieldData->text.length()) {
        m_textFieldData->cursorPos = m_textFieldData->text.length();
    }
    
    // Clear selection if it's out of bounds
    if (m_textFieldData->selection.start > m_textFieldData->text.length() ||
        m_textFieldData->selection.end > m_textFieldData->text.length()) {
        m_textFieldData->selection = TextSelection{};
    }
    
    if (m_textFieldData->onChangeCallback) {
        m_textFieldData->onChangeCallback(m_textFieldData->text);
    }
    
    return *this;
}

const std::string& TextField::getText() const {
    return m_textFieldData->text;
}

TextField& TextField::placeholder(const std::string& placeholder) {
    m_textFieldData->placeholder = placeholder;
    return *this;
}

const std::string& TextField::getPlaceholder() const {
    return m_textFieldData->placeholder;
}

TextField& TextField::label(const std::string& label) {
    m_textFieldData->label = label;
    return *this;
}

const std::string& TextField::getLabel() const {
    return m_textFieldData->label;
}

// Configuration
TextField& TextField::multiline(bool enabled) {
    m_textFieldData->isMultiline = enabled;
    return *this;
}

bool TextField::isMultiline() const {
    return m_textFieldData->isMultiline;
}

TextField& TextField::maxLength(int length) {
    m_textFieldData->maxLength = length;
    
    // Truncate existing text if needed
    if (length > 0 && m_textFieldData->text.length() > static_cast<size_t>(length)) {
        m_textFieldData->text = m_textFieldData->text.substr(0, length);
        if (m_textFieldData->cursorPos > m_textFieldData->text.length()) {
            m_textFieldData->cursorPos = m_textFieldData->text.length();
        }
    }
    
    return *this;
}

int TextField::getMaxLength() const {
    return m_textFieldData->maxLength;
}

TextField& TextField::password(bool isPassword) {
    m_textFieldData->isPassword = isPassword;
    return *this;
}

bool TextField::isPassword() const {
    return m_textFieldData->isPassword;
}

TextField& TextField::passwordChar(char maskChar) {
    m_textFieldData->passwordChar = maskChar;
    return *this;
}

char TextField::getPasswordChar() const {
    return m_textFieldData->passwordChar;
}

TextField& TextField::readOnly(bool readOnly) {
    m_textFieldData->isReadOnly = readOnly;
    return *this;
}

bool TextField::isReadOnly() const {
    return m_textFieldData->isReadOnly;
}

// Prefix/Suffix
TextField& TextField::prefix(Widget* widget) {
    m_textFieldData->prefixWidget = widget;
    return *this;
}

Widget* TextField::getPrefix() const {
    return m_textFieldData->prefixWidget;
}

TextField& TextField::suffix(Widget* widget) {
    m_textFieldData->suffixWidget = widget;
    return *this;
}

Widget* TextField::getSuffix() const {
    return m_textFieldData->suffixWidget;
}

// Cursor and Selection
TextField& TextField::cursorPosition(size_t pos) {
    m_textFieldData->cursorPos = std::min(pos, m_textFieldData->text.length());
    return *this;
}

size_t TextField::getCursorPosition() const {
    return m_textFieldData->cursorPos;
}

TextField& TextField::selection(size_t start, size_t end) {
    m_textFieldData->selection.start = std::min(start, m_textFieldData->text.length());
    m_textFieldData->selection.end = std::min(end, m_textFieldData->text.length());
    return *this;
}

const TextSelection& TextField::getSelection() const {
    return m_textFieldData->selection;
}

void TextField::selectAll() {
    m_textFieldData->selection.start = 0;
    m_textFieldData->selection.end = m_textFieldData->text.length();
}

void TextField::clearSelection() {
    m_textFieldData->selection = TextSelection{};
}

std::string TextField::getSelectedText() const {
    if (!m_textFieldData->selection.hasSelection()) {
        return "";
    }
    
    size_t start = m_textFieldData->selection.min();
    size_t length = m_textFieldData->selection.length();
    
    if (start >= m_textFieldData->text.length()) {
        return "";
    }
    
    return m_textFieldData->text.substr(start, length);
}

// Text Operations
void TextField::insertText(const std::string& insertText) {
    if (m_textFieldData->isReadOnly) {
        return;
    }
    
    saveUndoState();
    
    // Delete selected text first
    if (m_textFieldData->selection.hasSelection()) {
        size_t start = m_textFieldData->selection.min();
        size_t length = m_textFieldData->selection.length();
        m_textFieldData->text.erase(start, length);
        m_textFieldData->cursorPos = start;
        clearSelection();
    }
    
    // Check max length
    std::string textToInsert = insertText;
    if (m_textFieldData->maxLength > 0) {
        size_t remaining = m_textFieldData->maxLength - m_textFieldData->text.length();
        if (textToInsert.length() > remaining) {
            textToInsert = textToInsert.substr(0, remaining);
        }
    }
    
    // Insert text at cursor
    m_textFieldData->text.insert(m_textFieldData->cursorPos, textToInsert);
    m_textFieldData->cursorPos += textToInsert.length();
    
    // Clear redo history on new action
    m_textFieldData->redoHistory.clear();
    
    if (m_textFieldData->onChangeCallback) {
        m_textFieldData->onChangeCallback(m_textFieldData->text);
    }
}

void TextField::deleteText(bool forward) {
    if (m_textFieldData->isReadOnly) {
        return;
    }
    
    saveUndoState();
    
    if (m_textFieldData->selection.hasSelection()) {
        // Delete selected text
        size_t start = m_textFieldData->selection.min();
        size_t length = m_textFieldData->selection.length();
        m_textFieldData->text.erase(start, length);
        m_textFieldData->cursorPos = start;
        clearSelection();
    } else if (forward) {
        // Delete character after cursor
        if (m_textFieldData->cursorPos < m_textFieldData->text.length()) {
            m_textFieldData->text.erase(m_textFieldData->cursorPos, 1);
        }
    } else {
        // Delete character before cursor (backspace)
        if (m_textFieldData->cursorPos > 0) {
            m_textFieldData->cursorPos--;
            m_textFieldData->text.erase(m_textFieldData->cursorPos, 1);
        }
    }
    
    // Clear redo history on new action
    m_textFieldData->redoHistory.clear();
    
    if (m_textFieldData->onChangeCallback) {
        m_textFieldData->onChangeCallback(m_textFieldData->text);
    }
}

void TextField::copy() {
    if (m_textFieldData->selection.hasSelection()) {
        TextFieldData::clipboard = getSelectedText();
    }
}

void TextField::cut() {
    if (m_textFieldData->isReadOnly) {
        copy();
        return;
    }
    
    copy();
    if (m_textFieldData->selection.hasSelection()) {
        deleteText(true);
    }
}

void TextField::paste() {
    if (!TextFieldData::clipboard.empty()) {
        insertText(TextFieldData::clipboard);
    }
}

void TextField::undo() {
    if (m_textFieldData->undoHistory.empty()) {
        return;
    }
    
    // Save current state to redo
    UndoEntry currentState;
    currentState.text = m_textFieldData->text;
    currentState.cursorPos = m_textFieldData->cursorPos;
    currentState.selection = m_textFieldData->selection;
    m_textFieldData->redoHistory.push_back(currentState);
    
    // Restore previous state
    UndoEntry& prevState = m_textFieldData->undoHistory.back();
    m_textFieldData->text = prevState.text;
    m_textFieldData->cursorPos = prevState.cursorPos;
    m_textFieldData->selection = prevState.selection;
    m_textFieldData->undoHistory.pop_back();
    
    if (m_textFieldData->onChangeCallback) {
        m_textFieldData->onChangeCallback(m_textFieldData->text);
    }
}

void TextField::redo() {
    if (m_textFieldData->redoHistory.empty()) {
        return;
    }
    
    // Save current state to undo
    UndoEntry currentState;
    currentState.text = m_textFieldData->text;
    currentState.cursorPos = m_textFieldData->cursorPos;
    currentState.selection = m_textFieldData->selection;
    m_textFieldData->undoHistory.push_back(currentState);
    
    // Restore redo state
    UndoEntry& redoState = m_textFieldData->redoHistory.back();
    m_textFieldData->text = redoState.text;
    m_textFieldData->cursorPos = redoState.cursorPos;
    m_textFieldData->selection = redoState.selection;
    m_textFieldData->redoHistory.pop_back();
    
    if (m_textFieldData->onChangeCallback) {
        m_textFieldData->onChangeCallback(m_textFieldData->text);
    }
}

bool TextField::canUndo() const {
    return !m_textFieldData->undoHistory.empty();
}

bool TextField::canRedo() const {
    return !m_textFieldData->redoHistory.empty();
}

// Styling
TextField& TextField::textColor(const Color& color) {
    m_textFieldData->textColor = color;
    return *this;
}

const Color& TextField::getTextColor() const {
    return m_textFieldData->textColor;
}

TextField& TextField::placeholderColor(const Color& color) {
    m_textFieldData->placeholderColor = color;
    return *this;
}

const Color& TextField::getPlaceholderColor() const {
    return m_textFieldData->placeholderColor;
}

TextField& TextField::selectionColor(const Color& color) {
    m_textFieldData->selectionColor = color;
    return *this;
}

const Color& TextField::getSelectionColor() const {
    return m_textFieldData->selectionColor;
}

TextField& TextField::cursorColor(const Color& color) {
    m_textFieldData->cursorColor = color;
    return *this;
}

const Color& TextField::getCursorColor() const {
    return m_textFieldData->cursorColor;
}

// Event Callbacks
TextField& TextField::onChange(std::function<void(const std::string&)> callback) {
    m_textFieldData->onChangeCallback = std::move(callback);
    return *this;
}

TextField& TextField::onSubmit(std::function<void(const std::string&)> callback) {
    m_textFieldData->onSubmitCallback = std::move(callback);
    return *this;
}

// Display Text
std::string TextField::getDisplayText() const {
    if (m_textFieldData->isPassword) {
        return std::string(m_textFieldData->text.length(), m_textFieldData->passwordChar);
    }
    return m_textFieldData->text;
}

// Protected
void TextField::saveUndoState() {
    UndoEntry entry;
    entry.text = m_textFieldData->text;
    entry.cursorPos = m_textFieldData->cursorPos;
    entry.selection = m_textFieldData->selection;
    
    m_textFieldData->undoHistory.push_back(entry);
    
    // Limit history size
    if (m_textFieldData->undoHistory.size() > TextFieldData::maxUndoHistory) {
        m_textFieldData->undoHistory.erase(m_textFieldData->undoHistory.begin());
    }
}

} // namespace KillerGK
