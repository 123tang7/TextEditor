#include "codeeditor.h"

#include <QFontMetrics>
#include <QPainter>
#include <QTextBlock>

// ---------------------------------------------------------------------------
// 行号栏：一个覆盖在编辑器左侧的透明子控件，只负责绘制行号
// ---------------------------------------------------------------------------
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor)
        : QWidget(editor)
        , m_codeEditor(editor)
    {
        setCursor(Qt::ArrowCursor);
    }

    QSize sizeHint() const override
    {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *m_codeEditor;
};

// ---------------------------------------------------------------------------
CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
{
    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const
{
    if (!m_lineNumberVisible)
        return 0;

    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // 数字宽度 + 左右各 6px 留白
    const int space = 6 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + 6;
    return space;
}

void CodeEditor::setLineNumberVisible(bool visible)
{
    if (m_lineNumberVisible == visible)
        return;

    m_lineNumberVisible = visible;
    m_lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth(0);
    viewport()->update();
}

void CodeEditor::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (!m_lineNumberVisible)
        return;

    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                       lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(232, 240, 254)); // 淡蓝色高亮当前行
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(246, 248, 250));

    // 右侧分隔竖线
    painter.setPen(QColor(218, 223, 230));
    painter.drawLine(m_lineNumberArea->width() - 1, event->rect().top(),
                     m_lineNumberArea->width() - 1, event->rect().bottom());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    const QColor normalColor(120, 130, 140);
    const QColor currentColor(45, 108, 223);

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const bool isCurrent = (blockNumber == textCursor().blockNumber());
            painter.setPen(isCurrent ? currentColor : normalColor);
            painter.drawText(0, top,
                             m_lineNumberArea->width() - 6,
                             fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
