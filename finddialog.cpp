#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    setModal(false);

    connect(ui->findNextButton, &QPushButton::clicked,
            this, &FindDialog::findNextRequested);
    connect(ui->closeButton, &QPushButton::clicked, this, &FindDialog::close);
}

FindDialog::~FindDialog()
{
    delete ui;
}

QString FindDialog::searchText() const
{
    return ui->findLineEdit->text();
}

bool FindDialog::isCaseSensitive() const
{
    return ui->caseSensitiveCheck->isChecked();
}

bool FindDialog::isBackward() const
{
    return ui->backwardCheck->isChecked();
}

bool FindDialog::isWrapAround() const
{
    return ui->wrapCheck->isChecked();
}

void FindDialog::setSearchText(const QString &text)
{
    ui->findLineEdit->setText(text);
}

void FindDialog::focusSearchField()
{
    ui->findLineEdit->setFocus();
    ui->findLineEdit->selectAll();
}
