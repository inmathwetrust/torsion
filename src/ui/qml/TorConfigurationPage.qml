import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Column {
    id: setup
    spacing: 8

    property alias proxyType: proxyTypeField.selectedType
    property alias proxyAddress: proxyAddressField.text
    property alias proxyPort: proxyPortField.text
    property alias proxyUsername: proxyUsernameField.text
    property alias proxyPassword: proxyPasswordField.text
    property alias allowedPorts: allowedPortsField.text
    property alias bridges: bridgesField.text

    function reset() {
        proxyTypeField.currentIndex = 0
        proxyAddress = ''
        proxyPort = ''
        proxyUsername = ''
        proxyPassword = ''
        allowedPorts = ''
        bridges = ''
    }

    function save() {
        // null value is reset
        var conf = {
            'Socks4Proxy': null, 'Socks5Proxy': null, 'Socks5ProxyUsername': null,
            'Socks5ProxyPassword': null, 'HTTPProxy': null, 'HTTPProxyAuthenticator': null,
            'FirewallPorts': null, 'FascistFirewall': null, 'Bridge': null, 'UseBridges': null,
            'DisableNetwork': '0'
        }

        if (proxyType === "socks4") {
            conf['Socks4Proxy'] = proxyAddress + ":" + proxyPort
        } else if (proxyType === "socks5") {
            conf['Socks5Proxy'] = proxyAddress + ":" + proxyPort
            if (proxyUsername.length > 0)
                conf['Socks5ProxyUsername'] = proxyUsername
            if (proxyPassword.length > 0)
                conf['Socks5ProxyPassword'] = proxyPassword
        } else if (proxyType === "http") {
            conf['HTTPProxy'] = proxyAddress + ":" + proxyPort
            if (proxyUsername.length > 0 || proxyPassword.length > 0)
                conf['HTTPProxyAuthenticator'] = proxyUsername + ":" + proxyPassword
        }

        if (allowedPorts.length > 0) {
            conf['FirewallPorts'] = allowedPorts
            conf['FascistFirewall'] = "1"
        }

        if (bridges.length > 0) {
            conf['Bridge'] = bridges.split('\n')
            conf['UseBridges'] = "1"
        }

        var command = torControl.setConfiguration(conf)
        command.finished.connect(function() {
            if (command.successful) {
                torControl.saveConfiguration()
                window.openBootstrap()
            } else
                console.log("SETCONF error:", command.errorMessage)
        })
    }

    Label {
        width: parent.width
        text: "Does this computer need a proxy to access the internet?"
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: setup.width

        GridLayout {
            anchors.fill: parent
            columns: 2

            Label {
                text: "Proxy type:"
                color: proxyPalette.text
            }
            ComboBox {
                id: proxyTypeField
                model: ListModel {
                    ListElement { text: "None"; type: "" }
                    ListElement { text: "SOCKS 4"; type: "socks4" }
                    ListElement { text: "SOCKS 5"; type: "socks5" }
                    ListElement { text: "HTTP"; type: "http" }
                }
                textRole: "text"
                property string selectedType: currentIndex >= 0 ? model.get(currentIndex).type : ""

                SystemPalette {
                    id: proxyPalette
                    colorGroup: setup.proxyType == "" ? SystemPalette.Disabled : SystemPalette.Active
                }
            }

            Label {
                text: "Address:"
                color: proxyPalette.text
            }
            RowLayout {
                Layout.fillWidth: true
                TextField {
                    id: proxyAddressField
                    Layout.fillWidth: true
                    enabled: setup.proxyType
                    placeholderText: "IP address or hostname"
                }
                Label {
                    text: "Port:"
                    color: proxyPalette.text
                }
                TextField {
                    id: proxyPortField
                    Layout.preferredWidth: 50
                    enabled: setup.proxyType
                }
            }

            Label {
                text: "Username:"
                color: proxyPalette.text
            }
            RowLayout {
                Layout.fillWidth: true

                TextField {
                    id: proxyUsernameField
                    Layout.fillWidth: true
                    enabled: setup.proxyType
                    placeholderText: "Optional"
                }
                Label {
                    text: "Password:"
                    color: proxyPalette.text
                }
                TextField {
                    id: proxyPasswordField
                    Layout.fillWidth: true
                    enabled: setup.proxyType
                    placeholderText: "Optional"
                }
            }
        }
    }

    Item { height: 4; width: 1 }

    Label {
        width: parent.width
        text: "Does this computer's Internet connection go through a firewall " +
              "that only allows connections to certain ports?"
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: parent.width
        // Workaround OS X visual bug
        height: Math.max(implicitHeight, 40)
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Allowed ports:"
            }
            TextField {
                id: allowedPortsField
                Layout.fillWidth: true
            }
            Label {
                text: "Example: 80,443"
                SystemPalette { id: disabledPalette; colorGroup: SystemPalette.Disabled }
                color: disabledPalette.text
            }
        }
    }

    Item { height: 4; width: 1 }

    Label {
        width: parent.width
        text: "If this computer's Internet connection is censored, you will need " +
              "to obtain and use bridge relays."
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: parent.width
        ColumnLayout {
            anchors.fill: parent
            Label {
                text: "Enter one or more bridge relays (one per line):"
            }
            TextArea {
                id: bridgesField
                Layout.fillWidth: true
                Layout.preferredHeight: allowedPortsField.height * 2
                tabChangesFocus: true
            }
        }
    }

    RowLayout {
        width: parent.width

        Button {
            text: "Back"
            onClicked: window.back()
        }

        Item { height: 1; Layout.fillWidth: true }

        Button {
            text: "Connect"
            isDefault: true
            onClicked: {
                setup.save()
            }
        }
    }
}
