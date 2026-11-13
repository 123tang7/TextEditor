#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QColor>
#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QComboBox;
class QLabel;
class QTimer;
class FindDialog;
class ReplaceDialog;

/**
 * @brief 简易文本编辑器主窗口。
 *
 * 菜单、工具栏与动作全部在 Qt Designer 中完成，
 * 本类只负责业务逻辑：文件读写、格式设置、查找替换、状态同步等。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /** 打开指定路径的文件（供命令行参数或外部调用使用） */
    bool openFileFromPath(const QString &filePath);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // ---- 文件 ----
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();

    // ---- 格式 ----
    void chooseFont();
    void chooseFontColor();
    void chooseBackgroundColor();
    void changeFontSize(const QString &sizeText);
    void toggleWordWrap(bool enabled);
    void toggleLineNumber(bool enabled);

    // ---- 视图 ----
    void toggleToolBar(bool visible);
    void toggleStatusBar(bool visible);

    // ---- 查找 / 替换 ----
    void showFindDialog();
    void showReplaceDialog();
    void findNext();
    void replaceCurrent();
    void replaceAll();

    // ---- 其他 ----
    void showAboutDialog();
    void onTextChanged();
    void autoSave();

private:
    void createExtraWidgets();
    void createStatusBarWidgets();
    void setupConnections();

    void setCurrentFile(const QString &filePath);
    void updateWindowTitle();
    void updateStatusBar();
    void updateActionStates();

    bool maybeSave();
    bool loadFile(const QString &filePath, QString *errorString);
    bool writeFile(const QString &filePath, QString *errorString);
    bool findText(const QString &text, bool backward, bool caseSensitive,
                  bool wrapAround, bool *wrapped = nullptr);
    QString documentDisplayName() const;

    Ui::MainWindow *ui;

    QComboBox *m_fontSizeCombo = nullptr;   // 工具栏上的字号下拉框
    QLabel *m_positionLabel = nullptr;      // 状态栏：行列号
    QLabel *m_charCountLabel = nullptr;     // 状态栏：字符数
    QLabel *m_stateLabel = nullptr;         // 状态栏：保存状态
    QTimer *m_autoSaveTimer = nullptr;      // 定时自动保存
    FindDialog *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;

    QString m_filePath;                     // 当前文件路径，空表示“未命名”
    QColor m_textColor;                     // 当前字体颜色
    QColor m_baseColor;                     // 当前背景颜色
};

#endif // MAINWINDOW_H
