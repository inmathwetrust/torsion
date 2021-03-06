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

#include "ContactIDValidator.h"

static QRegularExpression regex(QStringLiteral("torsion:[a-z2-7]{16}"));

ContactIDValidator::ContactIDValidator(QObject *parent)
    : QRegularExpressionValidator(parent), m_uniqueIdentity(0)
{
    setRegularExpression(regex);
}

QValidator::State ContactIDValidator::validate(QString &text, int &pos) const
{
    Q_UNUSED(pos);
    fixup(text);
    if (text.isEmpty())
        return QValidator::Intermediate;

    QValidator::State re = QRegularExpressionValidator::validate(text, pos);
    if (re != QValidator::Acceptable)
        return re;

    ContactUser *u;
    if (m_uniqueIdentity && (u = m_uniqueIdentity->contacts.lookupHostname(text)))
    {
        emit contactExists(u);
        return QValidator::Intermediate;
    }

    return re;
}

void ContactIDValidator::fixup(QString &text) const
{
    text = text.trimmed().toLower();
}

bool ContactIDValidator::isValidID(const QString &text)
{
    return regex.match(text).hasMatch();
}

QString ContactIDValidator::hostnameFromID(const QString &ID)
{
    if (!isValidID(ID))
        return QString();

    return ID.mid(8) + QStringLiteral(".onion");
}

QString ContactIDValidator::idFromHostname(const QString &hostname)
{
    QString re = hostname;

    if (re.size() != 16)
    {
        if (re.size() == 22 && re.toLower().endsWith(QLatin1String(".onion")))
            re.chop(6);
        else
            return QString();
    }

    re.prepend(QStringLiteral("torsion:"));

    if (!isValidID(re))
        return QString();
    return re;
}

