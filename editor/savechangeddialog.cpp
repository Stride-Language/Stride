#include "savechangeddialog.h"
#include "ui_savechangeddialog.h"

#include <QPushButton>

SaveChangedDialog::SaveChangedDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveChangedDialog)
{
    ui->setupUi(this);
    QPushButton *button = ui->buttonBox->button(QDialogButtonBox::Save);
    button->setDefault(true);
}

SaveChangedDialog::~SaveChangedDialog()
{
    delete ui;
}

void SaveChangedDialog::setListContents(QStringList fileNames)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(fileNames);
    ui->listWidget->selectAll();
}

QList<int> SaveChangedDialog::getSelected()
{
    QList<int> selected;
    if (result() == QDialog::Rejected) {
        return selected;
    }
    foreach(QListWidgetItem *item, ui->listWidget->selectedItems()) {
        selected.append(ui->listWidget->row(item));
    }
    return selected;
}