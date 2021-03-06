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
#include "ui/MainWindow.h"
#include "core/IdentityManager.h"
#include "tor/TorManager.h"
#include "tor/TorControl.h"
#include "utils/CryptoKey.h"
#include "utils/SecureRNG.h"
#include <QApplication>
#include <QSettings>
#include <QTime>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>
#include <QLocale>
#include <QLockFile>
#include <QStandardPaths>
#include <openssl/crypto.h>

AppSettings *config = 0;
static QLockFile *configLock = 0;

static bool initSettings(QString &errorMessage);
static void initTranslation();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationVersion(QLatin1String("1.0.0"));

    {
        QString error;
        if (!initSettings(error)) {
            QMessageBox::critical(0, QStringLiteral("Torsion Error"), error);
            return 1;
        }
    }

    initTranslation();

    /* Initialize OpenSSL's allocator */
    CRYPTO_malloc_init();

    /* Seed the OpenSSL RNG */
    if (!SecureRNG::seed())
        qFatal("Failed to initialize RNG");
    qsrand(SecureRNG::randomInt(UINT_MAX));

    /* Tor control manager */
    torControl = Tor::TorManager::instance()->control();
    Tor::TorManager::instance()->start();

    /* Identities */
    identityManager = new IdentityManager;

    /* Window */
    MainWindow w;

    int r = a.exec();
    delete configLock;
    return r;
}

static QString userConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

#ifdef Q_OS_MAC
static QString appBundlePath()
{
    QString path = QApplication::applicationDirPath();
    int p = path.lastIndexOf(QLatin1String(".app/"));
    if (p >= 0)
    {
        p = path.lastIndexOf(QLatin1Char('/'), p);
        path = path.left(p+1);
    }

    return path;
}
#endif

static bool initSettings(QString &errorMessage)
{
    /* If built in portable mode (default), configuration is stored in the 'config'
     * directory next to the binary. If not writable, launching fails.
     *
     * Portable OS X is an exception. In that case, configuration is stored in a
     * 'config.torsion' folder next to the application bundle, unless the application
     * path contains "/Applications", in which case non-portable mode is used.
     *
     * When not in portable mode, a platform-specific per-user config location is used.
     *
     * This behavior may be overriden by passing a folder path as the first argument.
     */

    qApp->setOrganizationName(QStringLiteral("Torsion"));

    QString configPath;
    QStringList args = qApp->arguments();
    if (args.size() > 1) {
        configPath = args[1];
    } else {
#ifndef TORSION_NO_PORTABLE
# ifdef Q_OS_MAC
        if (!qApp->applicationDirPath().contains(QStringLiteral("/Applications")))
            configPath = appBundlePath() + QStringLiteral("config.torsion");
# else
        configPath = qApp->applicationDirPath() + QStringLiteral("/config");
# endif
#endif
        if (configPath.isEmpty())
            configPath = userConfigPath();
    }

    QDir dir(configPath);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        errorMessage = QStringLiteral("Cannot create configuration directory");
        return false;
    }

    QFile dirf(configPath);
    static QFile::Permissions perm = QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser;
    static QFile::Permissions ignore = QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner;
    if ((dirf.permissions() & ~ignore) != perm) {
        qWarning() << "Correcting permissions on configuration directory";
        if (!dirf.setPermissions(perm))
            qWarning() << "Setting permissions failed";
    }

    configLock = new QLockFile(dir.filePath(QStringLiteral("lock")));
    configLock->setStaleLockTime(0);
    if (!configLock->tryLock()) {
        if (configLock->error() == QLockFile::LockFailedError)
            errorMessage = QStringLiteral("Torsion is already running");
        else
            errorMessage = QStringLiteral("Cannot write configuration files (failed to acquire lock)");
        return false;
    }

    config = new AppSettings(dir.filePath(QStringLiteral("Torsion.ini")), QSettings::IniFormat);
    if (!config->isWritable()) {
        errorMessage = QStringLiteral("Configuration file is not writable");
        return false;
    }

    QDir::setCurrent(dir.absolutePath());
    return true;
}

static void initTranslation()
{
    QTranslator *translator = new QTranslator;

    bool ok = false;
    QString appPath = qApp->applicationDirPath();
    QString resPath = QLatin1String(":/lang/");

    /* First, try to load the user's configured language */
    QString configLang = config->value(QLatin1String("core/language")).toString();
    if (!configLang.isEmpty())
    {
        QString filename = QLatin1String("torsion.") + configLang;
        QString separators = QLatin1String("_");

        /* Look in the application directory */
        ok = translator->load(filename, appPath, separators);
        /* Look in the resources */
        if (!ok)
            ok = translator->load(filename, resPath, separators);
    }

    /* Next, try to load the system locale language, and allow it to fall back to the english default */
    if (!ok)
    {
        QString filename = QLatin1String("torsion.") + QLocale::system().name();
        ok = translator->load(filename, appPath);
        if (!ok)
            ok = translator->load(filename, resPath);
    }

    if (ok)
        qApp->installTranslator(translator);
}

