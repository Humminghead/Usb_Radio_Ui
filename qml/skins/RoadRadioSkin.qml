import QtQuick

// Dark "Road Radio" skin: top ruler, big frequency, station + LIVE badge,
// animated dot-matrix equalizer and a preset carousel at the bottom.
SkinBase {
    id: skin

    // ---- palette ----
    readonly property color bg:      "#282439"
    readonly property color accent:  "#6a4cff"
    readonly property color marker:  "#f0384f"
    readonly property color dim:     "#8c88a0"
    readonly property color dotOff:  "#3a3550"
    readonly property color dotLow:  "#e49a5f"
    readonly property color dotHigh: "#ee5a78"

    // ---- presets (keep sorted by frequency) ----
    readonly property var presets: [
        { name: "Jamz",        f: 88.1  },
        { name: "Truth Radio", f: 91.5  },
        { name: "Road Radio",  f: 95.4  },
        { name: "Thill Trax",  f: 99.2  },
        { name: "Kabl Radio",  f: 103.5 },
        { name: "Night Drive", f: 106.7 }
    ]
    function presetAt(f) {
        for (let i = 0; i < presets.length; ++i)
            if (Math.abs(presets[i].f - f) < 0.05) return presets[i]
        return null
    }
    // fractional index of `f` inside the preset list (carousel slides with the tuner)
    function presetPosition(f) {
        const p = presets
        if (f <= p[0].f) return 0
        for (let i = 0; i < p.length - 1; ++i)
            if (f < p[i + 1].f) return i + (f - p[i].f) / (p[i + 1].f - p[i].f)
        return p.length - 1
    }
    readonly property var activePreset: presetAt(frequency)           // confirmed frequency
    readonly property real presetPos: presetPosition(displayFrequency) // visual position

    // ---- skin-local animation state (see SkinBase) ----
    property real displayFrequency: frequency
    property bool dragging: false
    property bool powerOn: uiActive
    property bool favorite: false

    Behavior on displayFrequency {
        enabled: !skin.dragging
        NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
    }
    onFrequencyChanged: { if (!skin.dragging) skin.displayFrequency = skin.frequency }
    onPlayingChanged:   { if (!skin.playing) eq.flatten() }

    Rectangle { anchors.fill: parent; color: skin.bg }

    // small drawn icons (inline component: must not reference ids of this file)
    component Glyph: Canvas {
        property string kind
        property color tint: "#8c88a0"
        property bool filled: false
        property bool muted: false
        onTintChanged: requestPaint()
        onFilledChanged: requestPaint()
        onMutedChanged: requestPaint()
        onPaint: {
            const c = getContext("2d")
            c.clearRect(0, 0, width, height)
            c.strokeStyle = tint; c.fillStyle = tint
            c.lineWidth = 1.6; c.lineCap = "round"; c.lineJoin = "round"
            const cx = width / 2, cy = height / 2
            if (kind === "left" || kind === "right") {
                const d = kind === "left" ? -1 : 1
                c.beginPath()
                c.moveTo(cx - d * 10, cy); c.lineTo(cx + d * 10, cy)
                c.moveTo(cx + d * 4, cy - 6); c.lineTo(cx + d * 10, cy); c.lineTo(cx + d * 4, cy + 6)
                c.stroke()
            } else if (kind === "heart") {
                c.beginPath()
                c.moveTo(cx, cy + 7)
                c.bezierCurveTo(cx - 13, cy - 2, cx - 6, cy - 11, cx, cy - 4)
                c.bezierCurveTo(cx + 6, cy - 11, cx + 13, cy - 2, cx, cy + 7)
                if (filled) c.fill(); else c.stroke()
            } else if (kind === "speaker") {
                c.beginPath()
                c.moveTo(cx - 8, cy - 3); c.lineTo(cx - 4, cy - 3); c.lineTo(cx + 1, cy - 7)
                c.lineTo(cx + 1, cy + 7); c.lineTo(cx - 4, cy + 3); c.lineTo(cx - 8, cy + 3)
                c.closePath(); c.stroke()
                if (muted) {
                    c.beginPath(); c.moveTo(cx + 5, cy - 4); c.lineTo(cx + 11, cy + 4)
                    c.moveTo(cx + 11, cy - 4); c.lineTo(cx + 5, cy + 4); c.stroke()
                } else {
                    c.beginPath(); c.arc(cx + 1, cy, 5, -0.9, 0.9); c.stroke()
                    c.beginPath(); c.arc(cx + 1, cy, 9, -0.9, 0.9); c.stroke()
                }
            }
        }
    }

    // ================= header =================
    Item {
        x: 20; y: 30; width: 70; height: 24
        Column {
            y: 5; spacing: 5
            Repeater { model: 3
                Rectangle { width: index === 1 ? 17 : 22; height: 2; radius: 1; color: "#d8d6e6" } }
        }
        Text { x: 36; anchors.verticalCenter: parent.verticalCenter
               text: "Radio"; color: "#f1f0f7"; font.pixelSize: 19 }
        MouseArea { anchors.fill: parent; anchors.margins: -8; onClicked: skin.settingsRequested() }
    }

    Rectangle {                                     // OFF / ON pill
        x: skin.width - width - 20; y: 30; width: 52; height: 24; radius: 12
        color: skin.powerOn ? "#3b327a" : "#1f1c30"
        Text { x: skin.powerOn ? 29 : 10; anchors.verticalCenter: parent.verticalCenter
               text: skin.powerOn ? "ON" : "OFF"; color: "#d0cde0"; font.pixelSize: 10 }
        Rectangle {
            width: 14; height: 14; radius: 7; color: skin.accent
            y: 5; x: skin.powerOn ? 5 : 33
            Behavior on x { NumberAnimation { duration: 150 } }
        }
        MouseArea { anchors.fill: parent; anchors.margins: -6
                    onClicked: { skin.powerOn = !skin.powerOn; skin.powerUpToggled() } }
    }

    // ================= ruler =================
    Item {
        id: ruler
        y: 84; width: parent.width; height: 64; clip: true
        readonly property real pxPerMHz: 66

        Repeater {
            model: 106                                   // 87.0 .. 108.0, step 0.2
            delegate: Item {
                readonly property real f: 87 + index * 0.2
                readonly property real k: Math.max(0, 1 - Math.abs(f - skin.displayFrequency) / 2.6)
                readonly property bool whole: index % 5 === 0
                x: ruler.width / 2 + (f - skin.displayFrequency) * ruler.pxPerMHz
                visible: x > -30 && x < ruler.width + 30
                Rectangle {
                    x: -1; y: 33; width: 2; height: 16; radius: 1
                    color: Qt.rgba(0.35 + 0.62 * parent.k, 0.33 + 0.64 * parent.k, 0.45 + 0.52 * parent.k, 1)
                }
                Text {
                    visible: parent.whole
                    x: -width / 2; y: 1
                    text: Math.round(parent.f)
                    font.pixelSize: 17 + 2 * parent.k
                    color: Qt.rgba(0.42 + 0.55 * parent.k, 0.41 + 0.56 * parent.k, 0.52 + 0.45 * parent.k, 1)
                }
            }
        }

        // edge fades
        Rectangle {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            width: 60
            gradient: Gradient { orientation: Gradient.Horizontal
                GradientStop { position: 0; color: skin.bg }
                GradientStop { position: 1; color: Qt.rgba(skin.bg.r, skin.bg.g, skin.bg.b, 0) } }
        }
        Rectangle {
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            width: 60
            gradient: Gradient { orientation: Gradient.Horizontal
                GradientStop { position: 0; color: Qt.rgba(skin.bg.r, skin.bg.g, skin.bg.b, 0) }
                GradientStop { position: 1; color: skin.bg } }
        }

        // fixed marker (never animates)
        Canvas {
            x: ruler.width / 2 - 8; y: 31; width: 16; height: 14
            onPaint: {
                const c = getContext("2d"); c.clearRect(0, 0, width, height)
                c.fillStyle = skin.marker; c.lineJoin = "round"; c.strokeStyle = skin.marker; c.lineWidth = 2
                c.beginPath(); c.moveTo(2, 2); c.lineTo(14, 2); c.lineTo(8, 12); c.closePath()
                c.fill(); c.stroke()
                c.fillStyle = "white"; c.beginPath(); c.arc(8, 5.5, 1.8, 0, 2 * Math.PI); c.fill()
            }
        }

        MouseArea {
            anchors.fill: parent
            property real startX
            property real startF
            onPressed: (m) => { skin.dragging = true; startX = m.x; startF = skin.displayFrequency }
            onPositionChanged: (m) => {
                const f = Math.round((startF - (m.x - startX) / ruler.pxPerMHz) * 10) / 10
                skin.displayFrequency = Math.max(skin.minFreq, Math.min(skin.maxFreq, f))
            }
            onReleased: {
                skin.dragging = false
                if (Math.abs(skin.displayFrequency - skin.frequency) > 0.01)
                    skin.tuneRequested(skin.displayFrequency)
            }
            onCanceled: { skin.dragging = false; skin.displayFrequency = skin.frequency }
        }
    }

    // ================= frequency + station =================
    Text {
        y: 160; width: parent.width; horizontalAlignment: Text.AlignHCenter
        text: skin.displayFrequency.toFixed(1)
        color: "white"; font.pixelSize: 44
    }

    Text {                                           // follows confirmed frequency
        y: 232; width: parent.width; horizontalAlignment: Text.AlignHCenter
        text: skin.activePreset ? skin.activePreset.name : skin.stationName
        color: "white"; font.pixelSize: 27
    }

    Row {
        y: 279; anchors.horizontalCenter: parent.horizontalCenter; spacing: 8
        Rectangle {
            visible: skin.connected
            width: liveText.width + 14; height: 17; radius: 5; color: skin.accent
            anchors.verticalCenter: parent.verticalCenter
            Text { id: liveText; anchors.centerIn: parent; text: "LIVE"
                   color: "white"; font.pixelSize: 9; font.bold: true }
        }
        Text { text: skin.connected ? skin.stationInfo : (skin.status || "NO DEVICE")
               color: skin.dim; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
    }

    // prev / next
    Glyph { kind: "left";  x: 18; y: 253; width: 40; height: 30
            MouseArea { anchors.fill: parent; anchors.margins: -10; onClicked: skin.stepRequested(-1) } }
    Glyph { kind: "right"; x: skin.width - 58; y: 253; width: 40; height: 30
            MouseArea { anchors.fill: parent; anchors.margins: -10; onClicked: skin.stepRequested(1) } }

    // round buttons: mute/play + favourite
    Row {
        y: 321; anchors.horizontalCenter: parent.horizontalCenter; spacing: 21
        Rectangle {
            width: 38; height: 38; radius: 19; color: "transparent"; border.width: 1; border.color: "#3d3956"
            Glyph { anchors.fill: parent; kind: "speaker"; tint: "#b9b5cc"; muted: !skin.playing }
            MouseArea { anchors.fill: parent; onClicked: skin.playToggled() }
        }
        Rectangle {
            width: 38; height: 38; radius: 19; color: "transparent"; border.width: 1; border.color: "#3d3956"
            Glyph { anchors.fill: parent; kind: "heart"; filled: skin.favorite
                    tint: skin.favorite ? skin.dotHigh : "#b9b5cc" }
            MouseArea { anchors.fill: parent; onClicked: skin.favorite = !skin.favorite }
        }
    }

    // ================= dot-matrix equalizer =================
    Canvas {
        id: eq
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom
                  leftMargin: 18; rightMargin: 18; bottomMargin: 66 }
        height: 132
        readonly property int cols: 25
        readonly property int rows: 10
        property var levels: []
        property real phase: 0

        function flatten() { levels = new Array(cols).fill(2); requestPaint() }
        function step() {
            const next = []
            for (let i = 0; i < cols; ++i) {
                const env = 0.55 + 0.45 * Math.sin(i * 0.55 + phase)
                const target = 2 + Math.random() * 7 * env
                next.push(Math.max(2, Math.min(rows, Math.round((levels[i] || 2) * 0.45 + target * 0.55))))
            }
            levels = next; phase += 0.25
            requestPaint()
        }
        Component.onCompleted: flatten()

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            const dx = width / cols, dy = height / rows, r = Math.min(dx, dy) * 0.26
            for (let c = 0; c < cols; ++c)
                for (let rw = 0; rw < rows; ++rw) {
                    ctx.fillStyle = rw >= levels[c] ? skin.dotOff : (rw < 4 ? skin.dotLow : skin.dotHigh)
                    ctx.beginPath()
                    ctx.arc((c + 0.5) * dx, height - (rw + 0.5) * dy, r, 0, 2 * Math.PI)
                    ctx.fill()
                }
        }
    }
    Timer { interval: 110; repeat: true; running: skin.playing && skin.visible; onTriggered: eq.step() }

    // ================= preset carousel =================
    Item {
        id: carousel
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; bottomMargin: 10 }
        height: 32; clip: true
        readonly property real spacing: 105

        Repeater {
            model: skin.presets
            delegate: Text {
                readonly property real d: index - skin.presetPos
                readonly property real k: Math.max(0, 1 - Math.abs(d))
                x: carousel.width / 2 + d * carousel.spacing - width / 2
                anchors.verticalCenter: parent.verticalCenter
                text: modelData.name.toUpperCase()
                font.pixelSize: 11 + 3 * k
                color: Qt.rgba(0.55 + 0.45 * k, 0.53 + 0.47 * k, 0.63 + 0.37 * k, 1)
                MouseArea { anchors.fill: parent; anchors.margins: -10
                            onClicked: skin.tuneRequested(modelData.f) }
            }
        }
    }

    // ================= "no device" veil =================
    Rectangle {
        anchors.fill: parent; visible: !skin.connected; color: "#a6282439"
        Text {
            id: veilText
            anchors.centerIn: parent; text: "Connect the device"
            color: "#e0dff0"; font.pixelSize: 24; font.weight: Font.DemiBold
            SequentialAnimation on opacity {
                loops: Animation.Infinite; running: !skin.connected
                NumberAnimation { from: 1.0; to: 0.45; duration: 1200; easing.type: Easing.InOutQuad }
                NumberAnimation { from: 0.45; to: 1.0; duration: 1200; easing.type: Easing.InOutQuad }
            }
        }
    }
}
