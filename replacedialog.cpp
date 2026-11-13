#include "replacedialog.h"
#include "ui_replacedialog.h"

ReplaceDialog::ReplaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
    setModal(false);

    connect(ui->findNextButton, &QPushButton::clicked,
            this, &ReplaceDialog::findNextRequested);
    connect(ui->replaceButton, &QPushButton::clicked,
            this, &ReplaceDialog::replaceRequested);
    connect(ui->replaceAllButton, &QPushButton::clicked,
            this, &ReplaceDialog::replaceAllRequested);
    connect(ui->closeButton, &QPushButton::clicked, this, &ReplaceDialog::close);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

QString ReplaceDialog::searchText() const
{
    return ui->findLineEdit->text();
}

QString ReplaceDialog::replaceText() const
{
    return ui->replaceLineEdit->text();
}

bool ReplaceDialog::isCaseSensitive() const
{
    return ui->caseSensitiveCheck->isChecked();
}

bool ReplaceDialog::isWrapAround() const
{
    return ui->wrapCheck->isChecked();
}

void ReplaceDialog::setSearchText(const QString &text)
{
    ui->findLineEdit->setText(text);
}

void ReplaceDialog::focusSearchField()
{
    ui->findLineEdit->setFocus();
    ui->findLineEdit->selectAll();
}
