/*
 * 2012-07-01 12:13 PM
 * San Jose, California USA
 *
 * qtpam - example to authenticate root in qtapp / FreeBSD
 *
 * Copyright (c) 2012, Waitman Gobble <waitman@waitman.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Documentation +.v.+
 *
 * this allows you setuid your Qt app to allow a non-root user to run it as root,
 * but require the user to authenticate as root to run. In order to authenticate
 * root using pam, the app must be setuid root (for example see "su")
 * $ ls -l /usr/bin/su
 * -r-sr-xr-x  1 root  wheel  17304 Jun 10 17:10 /usr/bin/su
 *
 * 1) chmod 4755 app_name
 * 2) chown root:wheel app_name
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialog.h"
#include <sys/param.h>
#include <sys/wait.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <security/pam_appl.h>
#include <QDebug>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    Authenticated = false;
    ui->setupUi(this);
    connect( ui->pushButton, SIGNAL(clicked()),
             this, SLOT(doAuthenticate())) ;

    connect( ui->pushButton_2, SIGNAL(clicked()),
             this, SLOT(doTouchTest())) ;

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::doTouchTest() {
    if (Authenticated) {
        QStringList arguments;
        QString foo = "/usr/bin/touch /root/touchtest";
        arguments << "-c" << foo;
        QProcess* process = new QProcess(this);
        process->start("/bin/sh", arguments);
        process->waitForFinished();
        qDebug() << "Touch Test Succeeded. Check to see that /root/touchtest exists.";
        return true;
    } else {
        qDebug() << "Not authenticated as root. Access Denied.";
        return false;
    }
}

bool MainWindow::doAuthenticate() {

    Authenticated = false;
    LoginDialog dialog(this);
    if (dialog.exec()) {
        pcode = dialog.getPass();

        static pam_handle_t *pamh;
        struct pam_conv pamc = { converse, this };
        char hostname[MAXHOSTNAMELEN];
        char *ruser;
        int retcode = 0;

        char user[] = "root";
        pam_start("su", user, &pamc, &pamh);

        gethostname(hostname, sizeof(hostname));
        if ((retcode = pam_set_item(pamh, PAM_RHOST, hostname)) != PAM_SUCCESS) {
            pcode = "";
            dialog.clearPass();
            qDebug() << "pam_set_item hostname failed. " << pam_strerror(pamh, retcode);
            return false;
        }

        ruser = getlogin();
        if ((retcode = pam_set_item(pamh, PAM_RUSER, ruser)) != PAM_SUCCESS) {
            pcode = "";
            dialog.clearPass();
            qDebug() << "pam_set_item remote user failed. " << pam_strerror(pamh, retcode);
            return false;
        }

        if ((retcode = pam_authenticate(pamh, 0)) != PAM_SUCCESS) {
            pcode = "";
            dialog.clearPass();
            qDebug() << "pam_authenticate failed. " << pam_strerror(pamh, retcode);
            return false;
        }

        Authenticated = true;

        qDebug() << "Authenticated as root. ";
        pcode = "";
        dialog.clearPass();
        return true;
    }

    qDebug() << "Not Authenticated as root.";
    Authenticated = false;
    return false;
}

int
converse(int n, const struct pam_message **msg,
    struct pam_response **resp, void *data)
{

    MainWindow* ob = static_cast<MainWindow*>(data);


    QString strp = ob->pcode;
    QByteArray ba = strp.toLatin1();
    char *pcodec = ba.data();

    struct pam_response *aresp;
    char buf[PAM_MAX_RESP_SIZE];
    int i;

    aresp = new pam_response;

    if (n <= 0 || n > PAM_MAX_NUM_MSG)
        return (PAM_CONV_ERR);
    for (i = 0; i < n; ++i) {
        aresp[i].resp_retcode = 0;
        aresp[i].resp = NULL;
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
            aresp[i].resp = strdup(pcodec);
            if (aresp[i].resp == NULL)
                goto fail;
            break;
        case PAM_PROMPT_ECHO_ON:
            fputs(msg[i]->msg, stderr);
            if (fgets(buf, sizeof buf, stdin) == NULL)
                goto fail;
            aresp[i].resp = strdup(buf);
            if (aresp[i].resp == NULL)
                goto fail;
            break;
        case PAM_ERROR_MSG:
            fputs(msg[i]->msg, stderr);
            if (strlen(msg[i]->msg) > 0 &&
                msg[i]->msg[strlen(msg[i]->msg) - 1] != '\n')
                fputc('\n', stderr);
            break;
        case PAM_TEXT_INFO:
            fputs(msg[i]->msg, stdout);
            if (strlen(msg[i]->msg) > 0 &&
                msg[i]->msg[strlen(msg[i]->msg) - 1] != '\n')
                fputc('\n', stdout);
            break;
        default:
            goto fail;
        }
    }
    *resp = aresp;
    return (PAM_SUCCESS);
 fail:
        for (i = 0; i < n; ++i) {
                if (aresp[i].resp != NULL) {
                        memset(aresp[i].resp, 0, strlen(aresp[i].resp));
                        free(aresp[i].resp);
                }
        }
        memset(aresp, 0, n * sizeof *aresp);
    *resp = NULL;
    return (PAM_CONV_ERR);
}
