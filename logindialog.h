#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();
    QString getPass();
    void clearPass();
    
private:
    Ui::LoginDialog *ui;
    QString passcode;

private slots:
    void accept();
};

#endif // LOGINDIALOG_H
