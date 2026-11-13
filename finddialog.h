#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class FindDialog;
}
QT_END_NAMESPACE

/**
 * @brief 查找对话框（非模态）。
 *
 * 对话框只负责收集查找条件并发出请求信号，真正的查找逻辑放在 MainWindow 中，
 * 这样可以保证对话框打开时仍能操作主窗口的编辑器。
 */
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog() override;

    QString searchText() const;
    bool isCaseSensitive() const;
    bool isBackward() const;
    bool isWrapAround() const;

    void setSearchText(const QString &text);
    void focusSearchField();

signals:
    void findNextRequested();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
