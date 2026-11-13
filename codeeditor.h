#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

/**
 * @brief 带行号栏的文本编辑器控件（由 QPlainTextEdit 派生）。
 *
 * 在 Qt Designer 中把 QPlainTextEdit 提升（promote）为本类即可直接使用，
 * 行号栏由内部子控件 LineNumberArea 负责绘制。
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    /** 行号栏宽度（像素），供 viewport 边距计算使用 */
    int lineNumberAreaWidth() const;

    /** 由行号栏调用，绘制行号 */
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    bool isLineNumberVisible() const { return m_lineNumberVisible; }

public slots:
    /** 显示 / 隐藏行号栏 */
    void setLineNumberVisible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
    QWidget *m_lineNumberArea;          // 行号栏子控件
    bool m_lineNumberVisible = true;    // 行号栏是否可见
};

#endif // CODEEDITOR_H
