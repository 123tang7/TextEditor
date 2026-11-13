#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QGuiApplication>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTime>
#include <QTimer>
#include <QToolBar>

namespace {
constexpr int kAutoSaveIntervalMs = 30 * 1000;   // 自动保存周期：30 秒
const char *const kAppTitle = "简易文本编辑器";
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(QStringLiteral(":/icons/logo.svg")));
    m_textColor = ui->textEdit->palette().color(QPalette::Text);
    m_baseColor = ui->textEdit->palette().color(QPalette::Base);

    createStatusBarWidgets();
    createExtraWidgets();
    setupConnections();

    // 定时自动保存：只有文件已命名且内容被修改过才会真正写盘
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(kAutoSaveIntervalMs);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    m_autoSaveTimer->start();

    setCurrentFile(QString());
    ui->textEdit->setFocus();
    updateStatusBar();
    updateActionStates();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ===========================================================================
// 界面初始化
// ===========================================================================
void MainWindow::createStatusBarWidgets()
{
    m_positionLabel = new QLabel(this);
    m_charCountLabel = new QLabel(this);
    m_stateLabel = new QLabel(this);

    for (QLabel *label : {m_positionLabel, m_charCountLabel, m_stateLabel})
        label->setMinimumWidth(96);

    statusBar()->addPermanentWidget(m_positionLabel);
    statusBar()->addPermanentWidget(m_charCountLabel);
    statusBar()->addPermanentWidget(m_stateLabel);
    statusBar()->showMessage(tr("就绪"), 3000);
}

void MainWindow::createExtraWidgets()
{
    // 字号下拉框直接放在“格式工具栏”的最前面
    m_fontSizeCombo = new QComboBox(this);
    m_fontSizeCombo->setEditable(true);
    m_fontSizeCombo->setFixedWidth(70);
    m_fontSizeCombo->setToolTip(tr("字体大小（磅）"));
    const QList<int> sizes = {8, 9, 10, 11, 12, 14, 16, 18, 20, 24, 28, 32, 36, 48};
    for (int size : sizes)
        m_fontSizeCombo->addItem(QString::number(size));
    m_fontSizeCombo->setCurrentText(QString::number(ui->textEdit->font().pointSize()));
    ui->formatToolBar->insertWidget(ui->actionFont, m_fontSizeCombo);
}

void MainWindow::setupConnections()
{
    CodeEditor *editor = ui->textEdit;

    // ---- 文件 ----
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::close);

    // ---- 编辑：直接复用 QPlainTextEdit 自带的槽 ----
    connect(ui->actionUndo, &QAction::triggered, editor, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, editor, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, editor, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, editor, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, editor, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, editor, &QPlainTextEdit::selectAll);
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // ---- 格式 ----
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::chooseFont);
    connect(ui->actionFontColor, &QAction::triggered, this, &MainWindow::chooseFontColor);
    connect(ui->actionBackground, &QAction::triggered, this, &MainWindow::chooseBackgroundColor);
    connect(ui->actionWrap, &QAction::toggled, this, &MainWindow::toggleWordWrap);
    connect(ui->actionLineNumber, &QAction::toggled, this, &MainWindow::toggleLineNumber);
    connect(m_fontSizeCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::changeFontSize);

    // ---- 视图 ----
    connect(ui->actionToolBar, &QAction::toggled, this, &MainWindow::toggleToolBar);
    connect(ui->actionStatusBar, &QAction::toggled, this, &MainWindow::toggleStatusBar);

    // ---- 帮助 ----
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // ---- 状态同步 ----
    connect(editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::onTextChanged);
    connect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(editor, &QPlainTextEdit::selectionChanged, this, &MainWindow::updateStatusBar);
    connect(editor, &QPlainTextEdit::undoAvailable, ui->actionUndo, &QAction::setEnabled);
    connect(editor, &QPlainTextEdit::redoAvailable, ui->actionRedo, &QAction::setEnabled);

    // 剪贴板内容变化时刷新“粘贴”的可用状态
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updateActionStates);
}

// ===========================================================================
// 文件操作
// ===========================================================================
void MainWindow::newFile()
{
    if (!maybeSave())
        return;

    ui->textEdit->clear();
    setCurrentFile(QString());
    updateStatusBar();
    updateActionStates();
    statusBar()->showMessage(tr("已新建空白文档"), 3000);
    ui->textEdit->setFocus();
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    const QString startDir = m_filePath.isEmpty() ? QDir::homePath()
                                                  : QFileInfo(m_filePath).absolutePath();
    const QString path = QFileDialog::getOpenFileName(
        this, tr("打开文件"), startDir,
        tr("文本文件 (*.txt *.log *.md *.cpp *.h);;所有文件 (*.*)"));
    if (path.isEmpty())
        return;

    QString error;
    if (!loadFile(path, &error)) {
        // 边界情况：文件不存在 / 无权限 / 编码异常
        QMessageBox::warning(this, tr("打开失败"),
                             tr("无法打开文件：\n%1\n\n原因：%2")
                                 .arg(QDir::toNativeSeparators(path), error));
        return;
    }

    setCurrentFile(path);
    updateStatusBar();
    updateActionStates();
    statusBar()->showMessage(tr("已打开 %1（%2 字符）")
                                 .arg(QFileInfo(path).fileName())
                                 .arg(ui->textEdit->document()->characterCount() - 1),
                             3000);
}

bool MainWindow::openFileFromPath(const QString &filePath)
{
    QString error;
    if (!loadFile(filePath, &error)) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("无法打开文件：\n%1\n\n原因：%2")
                                 .arg(QDir::toNativeSeparators(filePath), error));
        return false;
    }

    setCurrentFile(filePath);
    updateStatusBar();
    updateActionStates();
    statusBar()->showMessage(tr("已打开 %1").arg(QFileInfo(filePath).fileName()), 3000);
    return true;
}

bool MainWindow::saveFile()
{
    if (m_filePath.isEmpty())
        return saveFileAs();

    QString error;
    if (!writeFile(m_filePath, &error)) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("无法写入文件：\n%1\n\n原因：%2")
                                 .arg(QDir::toNativeSeparators(m_filePath), error));
        return false;
    }

    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
    updateActionStates();
    statusBar()->showMessage(tr("已保存到 %1").arg(QFileInfo(m_filePath).fileName()), 3000);
    return true;
}

bool MainWindow::saveFileAs()
{
    const QString suggested = m_filePath.isEmpty()
                                  ? QDir::homePath() + QStringLiteral("/未命名.txt")
                                  : m_filePath;
    QString path = QFileDialog::getSaveFileName(this, tr("另存为"), suggested,
                                                tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (path.isEmpty())
        return false;

    // 未写扩展名时自动补全 .txt
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".txt");

    QString error;
    if (!writeFile(path, &error)) {
        QMessageBox::warning(this, tr("另存为失败"),
                             tr("无法写入文件：\n%1\n\n原因：%2")
                                 .arg(QDir::toNativeSeparators(path), error));
        return false;
    }

    setCurrentFile(path);
    updateStatusBar();
    updateActionStates();
    statusBar()->showMessage(tr("已另存为 %1").arg(QFileInfo(path).fileName()), 3000);
    return true;
}

bool MainWindow::loadFile(const QString &filePath, QString *errorString)
{
    const QFileInfo info(filePath);
    if (!info.exists()) {
        if (errorString)
            *errorString = tr("文件不存在");
        return false;
    }
    if (info.isDir()) {
        if (errorString)
            *errorString = tr("目标是文件夹，不是文件");
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorString)
            *errorString = file.errorString();
        return false;
    }
    const QByteArray data = file.readAll();
    file.close();

    // 优先按 UTF-8 解码；出现替换字符说明不是 UTF-8，退回本地编码（中文系统为 GBK）
    QString text = QString::fromUtf8(data);
    if (text.contains(QChar::ReplacementCharacter))
        text = QString::fromLocal8Bit(data);

    ui->textEdit->setPlainText(text);
    ui->textEdit->document()->setModified(false);
    ui->textEdit->moveCursor(QTextCursor::Start);
    return true;
}

bool MainWindow::writeFile(const QString &filePath, QString *errorString)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorString)
            *errorString = file.errorString();
        return false;
    }

    const QByteArray data = ui->textEdit->toPlainText().toUtf8();
    if (file.write(data) != data.size()) {
        if (errorString)
            *errorString = file.errorString();
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool MainWindow::maybeSave()
{
    QTextDocument *doc = ui->textEdit->document();
    if (!doc->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, tr("简易文本编辑器"),
                             tr("文档“%1”已被修改。\n是否保存修改？").arg(documentDisplayName()),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                             QMessageBox::Save);

    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Discard:
        return true;
    default:
        return false;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// ===========================================================================
// 格式设置
// ===========================================================================
void MainWindow::chooseFont()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, ui->textEdit->font(), this, tr("选择字体"));
    if (!ok)
        return;

    ui->textEdit->setFont(font);
    m_fontSizeCombo->setCurrentText(QString::number(font.pointSize()));
    statusBar()->showMessage(tr("字体已设置为 %1 %2 磅").arg(font.family()).arg(font.pointSize()),
                             3000);
}

void MainWindow::changeFontSize(const QString &sizeText)
{
    bool ok = false;
    const int size = sizeText.toInt(&ok);
    if (!ok || size < 6 || size > 96)
        return;

    QFont font = ui->textEdit->font();
    if (font.pointSize() == size)
        return;

    font.setPointSize(size);
    ui->textEdit->setFont(font);
    statusBar()->showMessage(tr("字号已设置为 %1 磅").arg(size), 3000);
}

void MainWindow::chooseFontColor()
{
    const QColor color = QColorDialog::getColor(m_textColor, this, tr("选择字体颜色"),
                                                QColorDialog::DontUseNativeDialog);
    if (!color.isValid())
        return;

    m_textColor = color;
    QPalette palette = ui->textEdit->palette();
    palette.setColor(QPalette::Text, color);
    ui->textEdit->setPalette(palette);
    statusBar()->showMessage(tr("字体颜色已设置为 %1").arg(color.name()), 3000);
}

void MainWindow::chooseBackgroundColor()
{
    const QColor color = QColorDialog::getColor(m_baseColor, this, tr("选择背景颜色"),
                                                QColorDialog::DontUseNativeDialog);
    if (!color.isValid())
        return;

    m_baseColor = color;
    QPalette palette = ui->textEdit->palette();
    palette.setColor(QPalette::Base, color);
    ui->textEdit->setPalette(palette);
    statusBar()->showMessage(tr("背景颜色已设置为 %1").arg(color.name()), 3000);
}

void MainWindow::toggleWordWrap(bool enabled)
{
    ui->textEdit->setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth
                                          : QPlainTextEdit::NoWrap);
    statusBar()->showMessage(enabled ? tr("已开启自动换行") : tr("已关闭自动换行"), 3000);
}

void MainWindow::toggleLineNumber(bool enabled)
{
    ui->textEdit->setLineNumberVisible(enabled);
    statusBar()->showMessage(enabled ? tr("已显示行号") : tr("已隐藏行号"), 3000);
}

void MainWindow::toggleToolBar(bool visible)
{
    ui->fileToolBar->setVisible(visible);
    ui->editToolBar->setVisible(visible);
    ui->formatToolBar->setVisible(visible);
    statusBar()->showMessage(visible ? tr("已显示工具栏") : tr("已隐藏工具栏"), 3000);
}

void MainWindow::toggleStatusBar(bool visible)
{
    statusBar()->setVisible(visible);
}

// ===========================================================================
// 查找 / 替换
// ===========================================================================
void MainWindow::showFindDialog()
{
    if (!m_findDialog) {
        m_findDialog = new FindDialog(this);
        connect(m_findDialog, &FindDialog::findNextRequested, this, &MainWindow::findNext);
    }

    // 若编辑器中已选中一段文本，自动作为查找内容
    const QString selected = ui->textEdit->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator) && selected.size() < 60)
        m_findDialog->setSearchText(selected);

    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
    m_findDialog->focusSearchField();
}

void MainWindow::showReplaceDialog()
{
    if (!m_replaceDialog) {
        m_replaceDialog = new ReplaceDialog(this);
        connect(m_replaceDialog, &ReplaceDialog::findNextRequested, this, &MainWindow::findNext);
        connect(m_replaceDialog, &ReplaceDialog::replaceRequested, this, &MainWindow::replaceCurrent);
        connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested, this, &MainWindow::replaceAll);
    }

    const QString selected = ui->textEdit->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator) && selected.size() < 60)
        m_replaceDialog->setSearchText(selected);

    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
    m_replaceDialog->focusSearchField();
}

bool MainWindow::findText(const QString &text, bool backward, bool caseSensitive,
                          bool wrapAround, bool *wrapped)
{
    if (wrapped)
        *wrapped = false;
    if (text.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (backward)
        flags |= QTextDocument::FindBackward;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    if (ui->textEdit->find(text, flags))
        return true;

    if (!wrapAround)
        return false;

    // 循环查找：把光标移到文档另一端后重新查找
    const QTextCursor original = ui->textEdit->textCursor();
    QTextCursor cursor = original;
    cursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
    ui->textEdit->setTextCursor(cursor);

    if (ui->textEdit->find(text, flags)) {
        if (wrapped)
            *wrapped = true;
        return true;
    }

    // 仍然找不到：恢复原来的光标位置，避免“未找到”时光标跳到文档另一端
    ui->textEdit->setTextCursor(original);
    return false;
}

void MainWindow::findNext()
{
    if (!m_findDialog || !m_findDialog->isVisible()) {
        showFindDialog();
        return;
    }

    const QString text = m_findDialog->searchText();
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("查找"), tr("请输入要查找的内容。"));
        return;
    }

    bool wrapped = false;
    if (findText(text, m_findDialog->isBackward(), m_findDialog->isCaseSensitive(),
                 m_findDialog->isWrapAround(), &wrapped)) {
        statusBar()->showMessage(wrapped ? tr("已从文档另一端继续查找") : tr("已找到匹配项"), 2500);
    } else {
        QMessageBox::information(this, tr("查找"), tr("未找到“%1”。").arg(text));
    }
}

void MainWindow::replaceCurrent()
{
    if (!m_replaceDialog)
        return;

    const QString text = m_replaceDialog->searchText();
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入要查找的内容。"));
        return;
    }

    const QTextCursor cursor = ui->textEdit->textCursor();
    const QString selected = cursor.selectedText();
    const bool selectedMatches =
        m_replaceDialog->isCaseSensitive()
            ? (selected == text)
            : (selected.compare(text, Qt::CaseInsensitive) == 0);

    // 当前没有选中要替换的内容时，先找到下一处并选中
    if (!selectedMatches) {
        bool wrapped = false;
        if (!findText(text, false, m_replaceDialog->isCaseSensitive(),
                      m_replaceDialog->isWrapAround(), &wrapped)) {
            QMessageBox::information(this, tr("替换"), tr("未找到“%1”。").arg(text));
        }
        return;
    }

    QTextCursor editor = ui->textEdit->textCursor();
    editor.insertText(m_replaceDialog->replaceText());
    ui->textEdit->setTextCursor(editor);
    statusBar()->showMessage(tr("已替换 1 处"), 2500);
    findNext();
}

void MainWindow::replaceAll()
{
    if (!m_replaceDialog)
        return;

    const QString text = m_replaceDialog->searchText();
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入要查找的内容。"));
        return;
    }

    QTextDocument::FindFlags flags;
    if (m_replaceDialog->isCaseSensitive())
        flags |= QTextDocument::FindCaseSensitively;

    QTextDocument *doc = ui->textEdit->document();
    QTextCursor editBlock(doc);
    editBlock.beginEditBlock();     // 让“全部替换”成为一次可撤销的操作

    int count = 0;
    int from = 0;
    while (true) {
        QTextCursor found = doc->find(text, from, flags);
        if (found.isNull())
            break;
        found.insertText(m_replaceDialog->replaceText());
        from = found.position();
        ++count;
    }
    editBlock.endEditBlock();

    if (count > 0) {
        statusBar()->showMessage(tr("已替换 %1 处").arg(count), 3000);
        QMessageBox::information(this, tr("替换"), tr("共替换 %1 处。").arg(count));
    } else {
        QMessageBox::information(this, tr("替换"), tr("未找到“%1”。").arg(text));
    }
}

// ===========================================================================
// 其他
// ===========================================================================
void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onTextChanged()
{
    updateWindowTitle();
    updateActionStates();
    updateStatusBar();
}

void MainWindow::autoSave()
{
    if (m_filePath.isEmpty() || !ui->textEdit->document()->isModified())
        return;

    QString error;
    if (!writeFile(m_filePath, &error)) {
        statusBar()->showMessage(tr("自动保存失败：%1").arg(error), 5000);
        return;
    }

    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
    updateActionStates();

    const QString time = QTime::currentTime().toString(QStringLiteral("HH:mm:ss"));
    statusBar()->showMessage(tr("已自动保存（%1）").arg(time), 4000);
    if (m_stateLabel)
        m_stateLabel->setText(tr("自动保存于 %1").arg(time));
}

// ===========================================================================
// 状态与标题
// ===========================================================================
QString MainWindow::documentDisplayName() const
{
    return m_filePath.isEmpty() ? tr("未命名") : QFileInfo(m_filePath).fileName();
}

void MainWindow::setCurrentFile(const QString &filePath)
{
    m_filePath = filePath;
    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(tr("%1[*] - %2").arg(documentDisplayName(), tr("简易文本编辑器")));
    setWindowModified(ui->textEdit->document()->isModified());
}

void MainWindow::updateStatusBar()
{
    const QTextCursor cursor = ui->textEdit->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int column = cursor.positionInBlock() + 1;

    if (cursor.hasSelection()) {
        // selectedText() 中的段落分隔符 / 软换行符不计入可见字符数
        QString selected = cursor.selectedText();
        selected.remove(QChar::ParagraphSeparator);
        selected.remove(QChar::LineSeparator);
        m_positionLabel->setText(tr("行 %1，列 %2（选中 %3）")
                                     .arg(line).arg(column).arg(selected.size()));
    } else {
        m_positionLabel->setText(tr("行 %1，列 %2").arg(line).arg(column));
    }

    const int characters = qMax(0, ui->textEdit->document()->characterCount() - 1);
    m_charCountLabel->setText(tr("字符数：%1").arg(characters));
    m_stateLabel->setText(ui->textEdit->document()->isModified() ? tr("未保存") : tr("已保存"));
}

void MainWindow::updateActionStates()
{
    QTextDocument *doc = ui->textEdit->document();
    const bool hasSelection = ui->textEdit->textCursor().hasSelection();

    // 剪切 / 复制：只有存在选中内容时才可用
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);

    // 撤销 / 恢复：跟随编辑器的历史栈
    ui->actionUndo->setEnabled(doc->isUndoAvailable());
    ui->actionRedo->setEnabled(doc->isRedoAvailable());

    // 粘贴：剪贴板中有文本才可用
    const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData();
    ui->actionPaste->setEnabled(mimeData && mimeData->hasText());

    // 全选 / 查找 / 替换：文档非空才有意义
    const bool hasText = doc->characterCount() > 1;
    ui->actionSelectAll->setEnabled(hasText);
    ui->actionFind->setEnabled(hasText);
    ui->actionReplace->setEnabled(hasText);
}
