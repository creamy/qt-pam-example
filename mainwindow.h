#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString pcode;
    bool Authenticated;

private:
    Ui::MainWindow *ui;

private slots:
    bool doAuthenticate();
    bool doTouchTest();
};

int converse(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data);

#endif // MAINWINDOW_H
