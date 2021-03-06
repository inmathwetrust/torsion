/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "main.h"
#include "MainWindow.h"
#include "core/UserIdentity.h"
#include "core/IncomingRequestManager.h"
#include "core/OutgoingContactRequest.h"
#include "core/IdentityManager.h"
#include "core/ContactIDValidator.h"
#include "tor/TorControl.h"
#include "tor/TorManager.h"
#include "tor/TorProcess.h"
#include "ui/AvatarImageProvider.h"
#include "ContactsModel.h"
#include "ui/ConversationModel.h"
#include <QtQml>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QMessageBox>
#include <QPushButton>

MainWindow *uiMain = 0;

MainWindow::MainWindow(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(!uiMain);
    uiMain = this;

    qml = new QQmlApplicationEngine(this);

    qmlRegisterUncreatableType<ContactUser>("org.torsionim.torsion", 1, 0, "ContactUser", QString());
    qmlRegisterUncreatableType<UserIdentity>("org.torsionim.torsion", 1, 0, "UserIdentity", QString());
    qmlRegisterUncreatableType<ContactsManager>("org.torsionim.torsion", 1, 0, "ContactsManager", QString());
    qmlRegisterUncreatableType<IncomingRequestManager>("org.torsionim.torsion", 1, 0, "IncomingRequestManager", QString());
    qmlRegisterUncreatableType<IncomingContactRequest>("org.torsionim.torsion", 1, 0, "IncomingContactRequest", QString());
    qmlRegisterUncreatableType<OutgoingContactRequest>("org.torsionim.torsion", 1, 0, "OutgoingContactRequest", QString());
    qmlRegisterUncreatableType<Tor::TorControl>("org.torsionim.torsion", 1, 0, "TorControl", QString());
    qmlRegisterUncreatableType<Tor::TorProcess>("org.torsionim.torsion", 1, 0, "TorProcess", QString());
    qmlRegisterType<ConversationModel>("org.torsionim.torsion", 1, 0, "ConversationModel");
    qmlRegisterType<ContactsModel>("org.torsionim.torsion", 1, 0, "ContactsModel");
    qmlRegisterType<ContactIDValidator>("org.torsionim.torsion", 1, 0, "ContactIDValidator");

    Q_ASSERT(!identityManager->identities().isEmpty());
    qml->rootContext()->setContextProperty(QLatin1String("userIdentity"), identityManager->identities()[0]);
    qml->rootContext()->setContextProperty(QLatin1String("torControl"), torControl);
    qml->rootContext()->setContextProperty(QLatin1String("torInstance"), Tor::TorManager::instance());
    qml->rootContext()->setContextProperty(QLatin1String("uiMain"), this);

    qml->addImageProvider(QLatin1String("avatar"), new AvatarImageProvider);
    qml->load(QUrl(QLatin1String("qrc:/ui/main.qml")));
}

MainWindow::~MainWindow()
{
}

QString MainWindow::version() const
{
    return qApp->applicationVersion();
}

QString MainWindow::aboutText() const
{
    QFile file(QStringLiteral(":/text/LICENSE"));
    file.open(QIODevice::ReadOnly);
    QString text = QString::fromUtf8(file.readAll());
    return text;
}

/* QMessageBox implementation for Qt <5.2 */
bool MainWindow::showRemoveContactDialog(ContactUser *user)
{
    QMessageBox::StandardButton btn = QMessageBox::question(0,
        QString::fromLatin1("Remove %1").arg(user->nickname()),
        QString::fromLatin1("Do you want to permanently remove %1?").arg(user->nickname()));
    return btn == QMessageBox::Yes;
}
