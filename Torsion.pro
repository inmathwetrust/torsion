# Torsion - http://torsionim.org/
# Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following disclaimer
#      in the documentation and/or other materials provided with the
#      distribution.
#
#    * Neither the names of the copyright owners nor the names of its
#      contributors may be used to endorse or promote products derived from
#      this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

lessThan(QT_MAJOR_VERSION,5)|lessThan(QT_MINOR_VERSION,1) {
    error("Qt 5.1 or greater is required. You can build your own, or get the SDK at https://qt-project.org/downloads")
}

TARGET = Torsion
TEMPLATE = app
QT += core gui network quick widgets

VERSION = 1.0.0

# Pass DEFINES+=TORSION_NO_PORTABLE for a system-wide installation

CONFIG(release,debug|release):DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

contains(DEFINES, TORSION_NO_PORTABLE) {
    unix:!macx {
        target.path = /usr/bin
        shortcut.path = /usr/share/applications
        shortcut.files = src/Torsion.desktop
        INSTALLS += target shortcut

        exists(tor) {
            message(Adding bundled Tor to installations)
            bundletor.path = /usr/lib/torsion/tor/
            bundletor.files = tor/*
            INSTALLS += bundletor
            DEFINES += BUNDLED_TOR_PATH=\\\"/usr/lib/torsion/tor/\\\"
        }
    }
}

macx {
    CONFIG += bundle

    exists(tor) {
        # Copy the entire tor/ directory, which should contain tor/tor (the binary itself)
        QMAKE_POST_LINK += cp -R $${_PRO_FILE_PWD_}/tor $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/;
    }
}

CONFIG += debug_and_release

# Create a pdb for release builds as well, to enable debugging
win32-msvc2008|win32-msvc2010 {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF,ICF
}

INCLUDEPATH += src

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += libcrypto # Using libcrypto instead of openssl to avoid needlessly linking libssl
}
win32 {
    isEmpty(OPENSSLDIR):error(You must pass OPENSSLDIR=path/to/openssl to qmake on this platform)
    INCLUDEPATH += $${OPENSSLDIR}/include
    LIBS += -L$${OPENSSLDIR}/lib -llibeay32

    # required by openssl
    LIBS += -lUser32 -lGdi32 -ladvapi32
}
macx:LIBS += -lcrypto

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += src/main.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ContactsModel.cpp \
    src/tor/TorControl.cpp \
    src/tor/TorControlSocket.cpp \
    src/tor/TorControlCommand.cpp \
    src/tor/ProtocolInfoCommand.cpp \
    src/tor/AuthenticateCommand.cpp \
    src/tor/SetConfCommand.cpp \
    src/utils/StringUtil.cpp \
    src/core/ContactsManager.cpp \
    src/core/ContactUser.cpp \
    src/protocol/ProtocolCommand.cpp \
    src/protocol/PingCommand.cpp \
    src/protocol/IncomingSocket.cpp \
    src/protocol/ChatMessageCommand.cpp \
    src/protocol/CommandHandler.cpp \
    src/protocol/CommandDataParser.cpp \
    src/tor/GetConfCommand.cpp \
    src/tor/HiddenService.cpp \
    src/protocol/ProtocolSocket.cpp \
    src/utils/CryptoKey.cpp \
    src/utils/SecureRNG.cpp \
    src/protocol/ContactRequestClient.cpp \
    src/protocol/ContactRequestServer.cpp \
    src/core/OutgoingContactRequest.cpp \
    src/core/IncomingRequestManager.cpp \
    src/core/ContactIDValidator.cpp \
    src/protocol/GetSecretCommand.cpp \
    src/core/UserIdentity.cpp \
    src/core/IdentityManager.cpp \
    src/utils/AppSettings.cpp \
    src/ui/AvatarImageProvider.cpp \
    src/ui/ConversationModel.cpp \
    src/tor/TorProcess.cpp \
    src/tor/TorManager.cpp \
    src/tor/TorSocket.cpp \
    src/protocol/OutgoingContactSocket.cpp

HEADERS += src/ui/MainWindow.h \
    src/ui/ContactsModel.h \
    src/tor/TorControl.h \
    src/tor/TorControlSocket.h \
    src/tor/TorControlCommand.h \
    src/tor/ProtocolInfoCommand.h \
    src/tor/AuthenticateCommand.h \
    src/tor/SetConfCommand.h \
    src/utils/StringUtil.h \
    src/core/ContactsManager.h \
    src/core/ContactUser.h \
    src/protocol/ProtocolCommand.h \
    src/protocol/PingCommand.h \
    src/protocol/IncomingSocket.h \
    src/main.h \
    src/protocol/ChatMessageCommand.h \
    src/protocol/CommandHandler.h \
    src/protocol/CommandDataParser.h \
    src/tor/GetConfCommand.h \
    src/tor/HiddenService.h \
    src/protocol/ProtocolSocket.h \
    src/utils/CryptoKey.h \
    src/utils/SecureRNG.h \
    src/protocol/ContactRequestClient.h \
    src/protocol/ContactRequestServer.h \
    src/core/OutgoingContactRequest.h \
    src/core/IncomingRequestManager.h \
    src/core/ContactIDValidator.h \
    src/protocol/GetSecretCommand.h \
    src/core/UserIdentity.h \
    src/core/IdentityManager.h \
    src/utils/AppSettings.h \
    src/ui/AvatarImageProvider.h \
    src/ui/ConversationModel.h \
    src/tor/TorProcess.h \
    src/tor/TorProcess_p.h \
    src/tor/TorManager.h \
    src/tor/TorSocket.h \
    src/protocol/OutgoingContactSocket.h

RESOURCES += translation/embedded.qrc \
    src/ui/qml/qml.qrc

OTHER_FILES += src/ui/qml/*
