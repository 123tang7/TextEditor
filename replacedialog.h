#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class ReplaceDialog;
}
QT_END_NAMESPACE

/**
 * @brief 替换对话框（非模态）。
 */
class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog() override;

    QString searchText() const;
    QString replaceText() const;
    bool isCaseSensitive() const;
    bool isWrapAround() const;

    void setSearchText(const QString &text);
    void focusSearchField();

signals:
    void findNextRequested();
    void replaceRequested();
    void replaceAllRequested();

private:
    Ui::ReplaceDialog *ui;
};

#endif // REPLACEDIALOG_H
