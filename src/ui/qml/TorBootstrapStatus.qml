import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Column {
    id: page
    spacing: 8

    property var bootstrap: torControl.bootstrapStatus
    onBootstrapChanged: {
        if (bootstrap['tag'] === "done")
            window.networkReady()
    }

    Label {
        text: "Connecting to the Tor network..."
        font.bold: true
    }

    ProgressBar {
        width: parent.width
        maximumValue: 100
        indeterminate: bootstrap.progress === undefined
        value: bootstrap.progress === undefined ? 0 : bootstrap.progress
    }

    Label {
        text: (bootstrap['warning'] !== undefined ) ? bootstrap['warning'] : bootstrap['summary']
    }

    TorLogDisplay {
        id: logDisplay
        width: parent.width
        height: 0
        visible: height > 0

        Behavior on height {
            SmoothedAnimation {
                easing.type: Easing.InOutQuad
                velocity: 1500
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
            text: logDisplay.height ? "Hide details" : "Show details"
            onClicked: {
                if (logDisplay.height)
                    logDisplay.height = 0
                else
                    logDisplay.height = 300
            }
        }

        Item { height: 1; Layout.fillWidth: true }

        Button {
            text: "Done"
            isDefault: true
            enabled: bootstrap.tag === "done"
            onClicked: window.visible = false
        }
    }
}

