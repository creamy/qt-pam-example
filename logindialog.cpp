#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    passcode = "";
}

QString LoginDialog::getPass() {
    return passcode;
}

void LoginDialog::accept() {
    passcode = ui->lineEdit->text();
    QDialog::accept();
}

void LoginDialog::clearPass() {
    passcode = "";
    ui->lineEdit->text() = "";
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

