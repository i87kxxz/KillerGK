/**
 * @file RichText.cpp
 * @brief Rich text editing and formatting implementation
 */

#include "KillerGK/text/RichText.hpp"
#include "KillerGK/text/TextRenderer.hpp"
#include <algorithm>
#include <sstream>
#include <regex>

namespace KillerGK {

// ============================================================================
// RichTextDocument Implementation
// ============================================================================

RichTextDocument RichTextDocument::fromPlainText(const std::string& text,
                                                  const TextFormat& defaultFormat) {
    RichTextDocument doc;
    doc.m_defaultFormat = defaultFormat;
    
    // Split into paragraphs by newlines
    std::istringstream stream(text);
    std::string line;
    
    while (std::getline(stream, line)) {
        Paragraph para;
        if (!line.empty()) {
            TextSpan span;
            span.text = line;
            span.format = defaultFormat;
            span.start = 0;
            span.length = line.size();
            para.elements.push_back(span);
        }
        doc.m_paragraphs.push_back(para);
    }
    
    // Ensure at least one paragraph
    if (doc.m_paragraphs.empty()) {
        doc.m_paragraphs.push_back(Paragraph{});
    }
    
    return doc;
}

RichTextDocument RichTextDocument::fromHTML(const std::string& html) {
    RichTextDocument doc;
    
    // Simple HTML parsing (basic tags only)
    std::string content = html;
    Paragraph currentPara;
    TextFormat currentFormat;
    
    // Remove HTML tags and extract text (simplified)
    std::regex tagRegex("<[^>]*>");
    std::string plainText = std::regex_replace(content, tagRegex, "");
    
    // For now, just create plain text document
    return fromPlainText(plainText);
}

std::string RichTextDocument::toPlainText() const {
    std::string result;
    
    for (size_t i = 0; i < m_paragraphs.size(); ++i) {
        const auto& para = m_paragraphs[i];
        
        for (const auto& element : para.elements) {
            if (std::holds_alternative<TextSpan>(element)) {
                result += std::get<TextSpan>(element).text;
            } else if (std::holds_alternative<EmbeddedImage>(element)) {
                result += "[image]";
            } else if (std::holds_alternative<EmbeddedWidget>(element)) {
                result += "[widget]";
            }
        }
        
        if (i + 1 < m_paragraphs.size()) {
            result += "\n";
        }
    }
    
    return result;
}

std::string RichTextDocument::toHTML() const {
    std::string html = "<!DOCTYPE html><html><body>";
    
    for (const auto& para : m_paragraphs) {
        html += "<p";
        
        // Add alignment style
        switch (para.alignment) {
            case ParagraphAlign::Center:
                html += " style=\"text-align:center\"";
                break;
            case ParagraphAlign::Right:
                html += " style=\"text-align:right\"";
                break;
            case ParagraphAlign::Justify:
                html += " style=\"text-align:justify\"";
                break;
            default:
                break;
        }
        html += ">";
        
        for (const auto& element : para.elements) {
            if (std::holds_alternative<TextSpan>(element)) {
                const auto& span = std::get<TextSpan>(element);
                std::string spanHtml = span.text;
                
                // Apply formatting tags
                if (span.format.bold) {
                    spanHtml = "<b>" + spanHtml + "</b>";
                }
                if (span.format.italic) {
                    spanHtml = "<i>" + spanHtml + "</i>";
                }
                if (span.format.underline) {
                    spanHtml = "<u>" + spanHtml + "</u>";
                }
                if (span.format.strikethrough) {
                    spanHtml = "<s>" + spanHtml + "</s>";
                }
                if (!span.format.link.empty()) {
                    spanHtml = "<a href=\"" + span.format.link + "\">" + spanHtml + "</a>";
                }
                
                html += spanHtml;
            } else if (std::holds_alternative<EmbeddedImage>(element)) {
                const auto& img = std::get<EmbeddedImage>(element);
                html += "<img src=\"" + img.path + "\"";
                if (img.width > 0) {
                    html += " width=\"" + std::to_string(static_cast<int>(img.width)) + "\"";
                }
                if (img.height > 0) {
                    html += " height=\"" + std::to_string(static_cast<int>(img.height)) + "\"";
                }
                html += "/>";
            }
        }
        
        html += "</p>";
    }
    
    html += "</body></html>";
    return html;
}

void RichTextDocument::saveUndoState() {
    if (m_inUndoGroup) return;
    
    UndoState state;
    state.paragraphs = m_paragraphs;
    m_undoStack.push_back(state);
    m_redoStack.clear();
    
    // Limit undo stack size
    if (m_undoStack.size() > 100) {
        m_undoStack.erase(m_undoStack.begin());
    }
}

void RichTextDocument::insertText(size_t position, const std::string& text,
                                   const TextFormat& format) {
    saveUndoState();
    
    // Find paragraph and position within it
    size_t currentPos = 0;
    for (auto& para : m_paragraphs) {
        size_t paraLength = 0;
        for (const auto& elem : para.elements) {
            if (std::holds_alternative<TextSpan>(elem)) {
                paraLength += std::get<TextSpan>(elem).text.size();
            } else {
                paraLength += 1;  // Embedded elements count as 1 character
            }
        }
        
        if (position <= currentPos + paraLength) {
            // Insert in this paragraph
            size_t localPos = position - currentPos;
            
            // Find the span to insert into
            size_t spanPos = 0;
            for (auto& elem : para.elements) {
                if (std::holds_alternative<TextSpan>(elem)) {
                    auto& span = std::get<TextSpan>(elem);
                    if (localPos <= spanPos + span.text.size()) {
                        // Insert here
                        size_t insertPos = localPos - spanPos;
                        span.text.insert(insertPos, text);
                        span.length = span.text.size();
                        return;
                    }
                    spanPos += span.text.size();
                } else {
                    spanPos += 1;
                }
            }
            
            // Append new span
            TextSpan newSpan;
            newSpan.text = text;
            newSpan.format = format;
            newSpan.length = text.size();
            para.elements.push_back(newSpan);
            return;
        }
        
        currentPos += paraLength + 1;  // +1 for newline
    }
    
    // Append to last paragraph
    if (!m_paragraphs.empty()) {
        TextSpan newSpan;
        newSpan.text = text;
        newSpan.format = format;
        newSpan.length = text.size();
        m_paragraphs.back().elements.push_back(newSpan);
    }
}

void RichTextDocument::deleteText(size_t start, size_t length) {
    if (length == 0) return;
    saveUndoState();
    
    // Simplified deletion - rebuild from plain text
    std::string text = toPlainText();
    if (start < text.size()) {
        text.erase(start, std::min(length, text.size() - start));
    }
    
    *this = fromPlainText(text, m_defaultFormat);
}

void RichTextDocument::replaceText(size_t start, size_t length, const std::string& text,
                                    const TextFormat& format) {
    deleteText(start, length);
    insertText(start, text, format);
}

void RichTextDocument::applyFormat(size_t start, size_t length, const TextFormat& format) {
    saveUndoState();
    
    // Simplified - just update format of spans in range
    size_t currentPos = 0;
    for (auto& para : m_paragraphs) {
        for (auto& elem : para.elements) {
            if (std::holds_alternative<TextSpan>(elem)) {
                auto& span = std::get<TextSpan>(elem);
                size_t spanEnd = currentPos + span.text.size();
                
                // Check if span overlaps with range
                if (currentPos < start + length && spanEnd > start) {
                    span.format = format;
                }
                
                currentPos += span.text.size();
            } else {
                currentPos += 1;
            }
        }
        currentPos += 1;  // Newline
    }
}

void RichTextDocument::toggleBold(size_t start, size_t length) {
    TextFormat format = getFormatAt(start);
    format.bold = !format.bold;
    applyFormat(start, length, format);
}

void RichTextDocument::toggleItalic(size_t start, size_t length) {
    TextFormat format = getFormatAt(start);
    format.italic = !format.italic;
    applyFormat(start, length, format);
}

void RichTextDocument::toggleUnderline(size_t start, size_t length) {
    TextFormat format = getFormatAt(start);
    format.underline = !format.underline;
    applyFormat(start, length, format);
}

void RichTextDocument::toggleStrikethrough(size_t start, size_t length) {
    TextFormat format = getFormatAt(start);
    format.strikethrough = !format.strikethrough;
    applyFormat(start, length, format);
}

void RichTextDocument::setColor(size_t start, size_t length, const Color& color) {
    TextFormat format = getFormatAt(start);
    format.color = color;
    applyFormat(start, length, format);
}

void RichTextDocument::setFontSize(size_t start, size_t length, float size) {
    TextFormat format = getFormatAt(start);
    format.fontSize = size;
    applyFormat(start, length, format);
}

void RichTextDocument::setFont(size_t start, size_t length, FontHandle font) {
    TextFormat format = getFormatAt(start);
    format.font = font;
    applyFormat(start, length, format);
}

void RichTextDocument::setLink(size_t start, size_t length, const std::string& url) {
    TextFormat format = getFormatAt(start);
    format.link = url;
    applyFormat(start, length, format);
}

void RichTextDocument::setParagraphAlignment(size_t paragraphIndex, ParagraphAlign align) {
    if (paragraphIndex < m_paragraphs.size()) {
        saveUndoState();
        m_paragraphs[paragraphIndex].alignment = align;
    }
}

void RichTextDocument::setParagraphIndent(size_t paragraphIndex, float first, 
                                           float left, float right) {
    if (paragraphIndex < m_paragraphs.size()) {
        saveUndoState();
        m_paragraphs[paragraphIndex].indentFirst = first;
        m_paragraphs[paragraphIndex].indentLeft = left;
        m_paragraphs[paragraphIndex].indentRight = right;
    }
}

void RichTextDocument::setParagraphLineHeight(size_t paragraphIndex, float lineHeight) {
    if (paragraphIndex < m_paragraphs.size()) {
        saveUndoState();
        m_paragraphs[paragraphIndex].lineHeight = lineHeight;
    }
}

void RichTextDocument::insertImage(size_t position, const std::string& path,
                                    float width, float height) {
    saveUndoState();
    
    EmbeddedImage img;
    img.path = path;
    img.width = width;
    img.height = height;
    img.position = position;
    
    // Find paragraph and insert
    if (!m_paragraphs.empty()) {
        m_paragraphs.back().elements.push_back(img);
    }
}

void RichTextDocument::insertWidget(size_t position, std::shared_ptr<Widget> widget,
                                     float width, float height) {
    saveUndoState();
    
    EmbeddedWidget w;
    w.widget = widget;
    w.width = width;
    w.height = height;
    w.position = position;
    
    if (!m_paragraphs.empty()) {
        m_paragraphs.back().elements.push_back(w);
    }
}

TextFormat RichTextDocument::getFormatAt(size_t position) const {
    size_t currentPos = 0;
    for (const auto& para : m_paragraphs) {
        for (const auto& elem : para.elements) {
            if (std::holds_alternative<TextSpan>(elem)) {
                const auto& span = std::get<TextSpan>(elem);
                if (position >= currentPos && position < currentPos + span.text.size()) {
                    return span.format;
                }
                currentPos += span.text.size();
            } else {
                currentPos += 1;
            }
        }
        currentPos += 1;
    }
    return m_defaultFormat;
}

size_t RichTextDocument::getLength() const {
    return toPlainText().size();
}

const Paragraph& RichTextDocument::getParagraph(size_t index) const {
    static Paragraph empty;
    return (index < m_paragraphs.size()) ? m_paragraphs[index] : empty;
}

size_t RichTextDocument::getParagraphAtPosition(size_t position) const {
    size_t currentPos = 0;
    for (size_t i = 0; i < m_paragraphs.size(); ++i) {
        size_t paraLength = 0;
        for (const auto& elem : m_paragraphs[i].elements) {
            if (std::holds_alternative<TextSpan>(elem)) {
                paraLength += std::get<TextSpan>(elem).text.size();
            } else {
                paraLength += 1;
            }
        }
        
        if (position <= currentPos + paraLength) {
            return i;
        }
        currentPos += paraLength + 1;
    }
    return m_paragraphs.empty() ? 0 : m_paragraphs.size() - 1;
}

void RichTextDocument::undo() {
    if (m_undoStack.empty()) return;
    
    UndoState current;
    current.paragraphs = m_paragraphs;
    m_redoStack.push_back(current);
    
    m_paragraphs = m_undoStack.back().paragraphs;
    m_undoStack.pop_back();
}

void RichTextDocument::redo() {
    if (m_redoStack.empty()) return;
    
    UndoState current;
    current.paragraphs = m_paragraphs;
    m_undoStack.push_back(current);
    
    m_paragraphs = m_redoStack.back().paragraphs;
    m_redoStack.pop_back();
}

bool RichTextDocument::canUndo() const {
    return !m_undoStack.empty();
}

bool RichTextDocument::canRedo() const {
    return !m_redoStack.empty();
}

void RichTextDocument::beginUndoGroup() {
    if (!m_inUndoGroup) {
        saveUndoState();
        m_inUndoGroup = true;
    }
}

void RichTextDocument::endUndoGroup() {
    m_inUndoGroup = false;
}

// ============================================================================
// RichTextEditor Implementation
// ============================================================================

struct RichTextEditor::Impl {
    std::shared_ptr<RichTextDocument> document;
    Rect bounds;
    TextSelection selection;
    size_t cursorPosition = 0;
    bool cursorVisible = true;
    bool selecting = false;
    
    ChangeCallback onChange;
    SelectionCallback onSelectionChange;
    LinkCallback onLinkClick;
};

RichTextEditor::RichTextEditor() : m_impl(std::make_unique<Impl>()) {
    m_impl->document = std::make_shared<RichTextDocument>();
}

RichTextEditor::~RichTextEditor() = default;

void RichTextEditor::setDocument(std::shared_ptr<RichTextDocument> document) {
    m_impl->document = document;
    m_impl->cursorPosition = 0;
    m_impl->selection = TextSelection{};
}

std::shared_ptr<RichTextDocument> RichTextEditor::getDocument() const {
    return m_impl->document;
}

void RichTextEditor::setBounds(const Rect& bounds) {
    m_impl->bounds = bounds;
}

TextSelection RichTextEditor::getSelection() const {
    return m_impl->selection;
}

void RichTextEditor::setSelection(const TextSelection& selection) {
    m_impl->selection = selection;
    m_impl->cursorPosition = selection.end;
    
    if (m_impl->onSelectionChange) {
        m_impl->onSelectionChange(selection);
    }
}

void RichTextEditor::selectAll() {
    if (m_impl->document) {
        m_impl->selection.start = 0;
        m_impl->selection.end = m_impl->document->getLength();
        m_impl->cursorPosition = m_impl->selection.end;
        
        if (m_impl->onSelectionChange) {
            m_impl->onSelectionChange(m_impl->selection);
        }
    }
}

size_t RichTextEditor::getCursorPosition() const {
    return m_impl->cursorPosition;
}

void RichTextEditor::setCursorPosition(size_t position) {
    m_impl->cursorPosition = position;
    m_impl->selection.start = position;
    m_impl->selection.end = position;
}

void RichTextEditor::cut() {
    copy();
    deleteSelection();
}

void RichTextEditor::copy() {
    // Copy to clipboard would be implemented here
    // For now, this is a placeholder
}

void RichTextEditor::paste() {
    // Paste from clipboard would be implemented here
    // For now, this is a placeholder
}

void RichTextEditor::deleteSelection() {
    if (!m_impl->document || m_impl->selection.isEmpty()) return;
    
    m_impl->document->deleteText(m_impl->selection.min(), m_impl->selection.length());
    m_impl->cursorPosition = m_impl->selection.min();
    m_impl->selection.start = m_impl->cursorPosition;
    m_impl->selection.end = m_impl->cursorPosition;
    
    if (m_impl->onChange) {
        m_impl->onChange();
    }
}

void RichTextEditor::handleKeyPress(uint32_t keyCode, bool shift, bool ctrl, bool alt) {
    if (!m_impl->document) return;
    
    (void)alt;  // Unused for now
    
    // Handle keyboard shortcuts
    if (ctrl) {
        switch (keyCode) {
            case 'A': selectAll(); return;
            case 'C': copy(); return;
            case 'X': cut(); return;
            case 'V': paste(); return;
            case 'Z': m_impl->document->undo(); break;
            case 'Y': m_impl->document->redo(); break;
            case 'B': 
                if (!m_impl->selection.isEmpty()) {
                    m_impl->document->toggleBold(m_impl->selection.min(), 
                                                  m_impl->selection.length());
                }
                break;
            case 'I':
                if (!m_impl->selection.isEmpty()) {
                    m_impl->document->toggleItalic(m_impl->selection.min(),
                                                    m_impl->selection.length());
                }
                break;
            case 'U':
                if (!m_impl->selection.isEmpty()) {
                    m_impl->document->toggleUnderline(m_impl->selection.min(),
                                                       m_impl->selection.length());
                }
                break;
        }
        return;
    }
    
    size_t docLength = m_impl->document->getLength();
    
    // Navigation keys
    switch (keyCode) {
        case 0x25:  // Left arrow
            if (m_impl->cursorPosition > 0) {
                m_impl->cursorPosition--;
            }
            if (!shift) {
                m_impl->selection.start = m_impl->cursorPosition;
            }
            m_impl->selection.end = m_impl->cursorPosition;
            break;
            
        case 0x27:  // Right arrow
            if (m_impl->cursorPosition < docLength) {
                m_impl->cursorPosition++;
            }
            if (!shift) {
                m_impl->selection.start = m_impl->cursorPosition;
            }
            m_impl->selection.end = m_impl->cursorPosition;
            break;
            
        case 0x24:  // Home
            m_impl->cursorPosition = 0;
            if (!shift) {
                m_impl->selection.start = 0;
            }
            m_impl->selection.end = 0;
            break;
            
        case 0x23:  // End
            m_impl->cursorPosition = docLength;
            if (!shift) {
                m_impl->selection.start = docLength;
            }
            m_impl->selection.end = docLength;
            break;
            
        case 0x08:  // Backspace
            if (!m_impl->selection.isEmpty()) {
                deleteSelection();
            } else if (m_impl->cursorPosition > 0) {
                m_impl->document->deleteText(m_impl->cursorPosition - 1, 1);
                m_impl->cursorPosition--;
                m_impl->selection.start = m_impl->cursorPosition;
                m_impl->selection.end = m_impl->cursorPosition;
            }
            if (m_impl->onChange) m_impl->onChange();
            break;
            
        case 0x2E:  // Delete
            if (!m_impl->selection.isEmpty()) {
                deleteSelection();
            } else if (m_impl->cursorPosition < docLength) {
                m_impl->document->deleteText(m_impl->cursorPosition, 1);
            }
            if (m_impl->onChange) m_impl->onChange();
            break;
            
        case 0x0D:  // Enter
            handleTextInput("\n");
            break;
    }
    
    if (m_impl->onSelectionChange) {
        m_impl->onSelectionChange(m_impl->selection);
    }
}

void RichTextEditor::handleTextInput(const std::string& text) {
    if (!m_impl->document || text.empty()) return;
    
    // Delete selection first if any
    if (!m_impl->selection.isEmpty()) {
        deleteSelection();
    }
    
    // Insert text at cursor
    m_impl->document->insertText(m_impl->cursorPosition, text);
    m_impl->cursorPosition += text.size();
    m_impl->selection.start = m_impl->cursorPosition;
    m_impl->selection.end = m_impl->cursorPosition;
    
    if (m_impl->onChange) {
        m_impl->onChange();
    }
}

void RichTextEditor::handleMouseDown(float x, float y, bool shift) {
    // Convert position to character index
    // This would use TextRenderer to find the character at position
    (void)x;
    (void)y;
    (void)shift;
    
    m_impl->selecting = true;
}

void RichTextEditor::handleMouseMove(float x, float y) {
    if (!m_impl->selecting) return;
    
    // Update selection end
    (void)x;
    (void)y;
}

void RichTextEditor::handleMouseUp(float x, float y) {
    (void)x;
    (void)y;
    m_impl->selecting = false;
}

void RichTextEditor::render() {
    if (!m_impl->document) return;
    
    // Render each paragraph
    float y = m_impl->bounds.y;
    
    for (size_t i = 0; i < m_impl->document->getParagraphCount(); ++i) {
        const auto& para = m_impl->document->getParagraph(i);
        
        // Build text for paragraph
        std::string paraText;
        for (const auto& elem : para.elements) {
            if (std::holds_alternative<TextSpan>(elem)) {
                paraText += std::get<TextSpan>(elem).text;
            }
        }
        
        // Render using TextRenderer
        TextStyle style;
        style.align = static_cast<TextAlign>(para.alignment);
        style.lineHeight = para.lineHeight;
        
        Rect paraBounds(m_impl->bounds.x + para.indentLeft, y,
                        m_impl->bounds.width - para.indentLeft - para.indentRight,
                        m_impl->bounds.height - (y - m_impl->bounds.y));
        
        TextRenderer::instance().renderTextInBounds(paraText, paraBounds, style);
        
        // Move to next paragraph
        Size textSize = TextRenderer::instance().measureText(paraText, style);
        y += textSize.height + para.marginBottom;
    }
}

void RichTextEditor::setOnChange(ChangeCallback callback) {
    m_impl->onChange = callback;
}

void RichTextEditor::setOnSelectionChange(SelectionCallback callback) {
    m_impl->onSelectionChange = callback;
}

void RichTextEditor::setOnLinkClick(LinkCallback callback) {
    m_impl->onLinkClick = callback;
}

} // namespace KillerGK
