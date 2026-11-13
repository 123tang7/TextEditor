#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QApplication>
#include <QPixmap>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // 使用应用程序图标填充左上角的 logo
    const QIcon appIcon = QApplication::windowIcon();
    if (!appIcon.isNull())
        ui->logoLabel->setPixmap(appIcon.pixmap(64, 64));

    connect(ui->okButton, &QPushButton::clicked, this, &QDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
