import QtQuick
import QtQuick.Controls
import QtQuick.Effects

SkinBase {
    id: skin
    readonly property color accent: "#d400c8"

    // skin-local animation state (see SkinBase / spec 5.5)
    property real displayFrequency: frequency
    property bool dragging: false
    property bool powerEnabled: false

    Behavior on displayFrequency {
        enabled: !skin.dragging
        NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
    }

    onFrequencyChanged: {
        if (!skin.dragging)
            skin.displayFrequency = skin.frequency
    }

    onPowerEnabledChanged: {
        ///\todo
        console.log("Power state changed")
    }
// Item{
//     id: rootBackground;

    Rectangle { anchors.fill: parent; color: "black" }

    // --- big frequency + reflection ---
    Text {
        id: big
        y: 40; width: parent.width
        horizontalAlignment: Text.AlignHCenter
        text: skin.displayFrequency.toFixed(1)
        color: "#f2f2f2"; font.pixelSize: 84; font.weight: Font.Light
    }
    // mirror image: the number flipped around its baseline, fading out downwards
    Item {
        id: mirror
        y: big.y + big.baselineOffset          // mirror axis = baseline of the big text
        width: parent.width; height: 64; clip: true

        Text {
            y: big.baselineOffset - big.height // flipped copy ends exactly at the axis
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: big.text; font: big.font; color: big.color; opacity: 0.45
            transform: Scale { yScale: -1; origin.y: big.height / 2 }
        }
        // fade: transparent at the baseline -> black at the bottom
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00000000" }
                GradientStop { position: 0.85; color: "#ff000000" }
            }
        }
    }

    // --- tuner scale ---
    Item {
        id: tuner
        y: 190; width: parent.width; height: 120; clip: true
        readonly property real pxPerMHz: 120

        Repeater {
            model: Math.round((skin.maxFreq - skin.minFreq) * 10) + 1
            delegate: Item {
                readonly property real f: skin.minFreq + index / 10
                readonly property bool whole: index % 10 === 5     // x.0 MHz
                readonly property bool half:  index % 5 === 0      // x.5 MHz
                x: tuner.width / 2 + (f - skin.displayFrequency) * tuner.pxPerMHz
                visible: x > -20 && x < tuner.width + 20
                Rectangle {
                    y: 70 - height / 2; width: 2
                    height: parent.whole ? 56 : parent.half ? 40 : 28
                    color: parent.whole ? "#e0e0e0" : "#9a9a9a"
                    opacity: parent.whole || parent.half ? 1 : 0.7
                }
                Text {
                    visible: parent.whole
                    x: -width / 2; y: -4
                    text: Math.round(parent.f); color: "#b0b0b0"; font.pixelSize: 24
                }
            }
        }

        // side shadows: scale fades into black at both edges
        Rectangle {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            width: 90
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#ff000000" }
                GradientStop { position: 1.0; color: "#00000000" }
            }
        }
        Rectangle {
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            width: 90
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#00000000" }
                GradientStop { position: 1.0; color: "#ff000000" }
            }
        }

        // needle: fixed, never animates
        Rectangle { x: tuner.width / 2 - 1; y: 0; width: 2; height: tuner.height; color: skin.accent }

        MouseArea {
            anchors.fill: parent
            property real startX
            property real startF
            onPressed: (m) => {
                skin.dragging = true
                startX = m.x
                startF = skin.displayFrequency
            }
            onPositionChanged: (m) => {
                const f = Math.round((startF - (m.x - startX) / tuner.pxPerMHz) * 10) / 10
                const c = Math.max(skin.minFreq, Math.min(skin.maxFreq, f))
                skin.displayFrequency = c                      // immediate, Behavior disabled
                if (Math.abs(c - skin.frequency) > 0.01)
                    skin.tuneRequested(c)
            }
            // dragging = false FIRST, then glide back to the confirmed value
            onReleased: { skin.dragging = false; skin.displayFrequency = skin.frequency }
            onCanceled: { skin.dragging = false; skin.displayFrequency = skin.frequency }
        }
    }

    // --- station text: follows confirmed `frequency` ---
    Column {
        y: 345; width: parent.width; spacing: 2
        Text { width: parent.width; horizontalAlignment: Text.AlignHCenter
               text: skin.stationName; color: "white"; font.pixelSize: 26; font.bold: true }
        Text { width: parent.width; horizontalAlignment: Text.AlignHCenter
               text: skin.connected ? skin.stationInfo : (skin.status || "NO DEVICE")
               color: "#e0e0e0"; font.pixelSize: 18 }
    }

    // --- transport ---
    Row {
        y: 455; anchors.horizontalCenter: parent.horizontalCenter; spacing: 70
        Image { source: "icons/arrow_btn_l.png"; width: 54; height: 54;
               MouseArea { anchors.fill: parent; anchors.margins: -15; onClicked: skin.stepRequested(-1) } }
        Image { source:  skin.powerEnabled ? "icons/power_round_on.png" : "icons/power_round_off.png";
               MouseArea { anchors.fill: parent; anchors.margins: -15; onClicked: skin.powerUpToggled() } }
        Image { source: "icons/arrow_btn_r.png"; width: 54; height: 54;
               MouseArea { anchors.fill: parent; anchors.margins: -15; onClicked: skin.stepRequested(1) } }
    }

    // --- bottom bar ---
    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 56; color: "#141414"
        Row {
            anchors.fill: parent
            Repeater {
                model: [
                    { g: "☰", act: "connect" }, { g: "+", act: "" },
                    { g: "🎧", act: "" },       { g: "≡", act: "skin" }
                ]
                delegate: Item {
                    width: skin.width / 4; height: 56
                    Text { anchors.centerIn: parent; text: modelData.g; font.pixelSize: 24
                           color: modelData.act === "connect" && skin.connected ? skin.accent : "#ddd" }
                    MouseArea { anchors.fill: parent
                        onClicked: {
                            if (modelData.act === "skin") skin.nextSkinRequested()
                            else if (modelData.act === "connect") skin.connectRequested()
                        } }
                }
            }
        }
    }

    // Transparent, smooth glass layer (Glassmorphism / Frosted Glass)
    Item {
        id: glassRoot
        width: parent.width
        height: parent.height
        visible: !skin.connected

        // Capture the background content beneath this panel
        ShaderEffectSource {
            id: backgroundSource
            sourceItem: skin//rootBackground // Reference to your underlying visual content/item
            sourceRect: Qt.rect(glassRoot.x, glassRoot.y, glassRoot.width, glassRoot.height)
            anchors.fill: parent
        }

        // Apply background blur and glass styling
        MultiEffect {
            id: glassEffect
            source: backgroundSource
            anchors.fill: parent
            blurEnabled: true
            blur: 1.0
            blurMax: 32

            // Optional tint to simulate glass color / frost
            colorization: 0.1
            colorizationColor: "#20ffffff"
        }

        // Optional specular/glossy border and inner highlight
        Rectangle {
            id: glassBorder
            visible: false
            anchors.fill: parent
            color: "transparent"
            radius: 16
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.25) // Subtle light top/edge
        }

        // Container for the text and its animated effects
        Item {
            id: textContainer
            anchors.centerIn: parent
            width: baseText.width
            height: baseText.height
            opacity: 0.0 // Hidden initially

            // Clean QML alternative to a Timer: Sequential sequence for Entrance
            SequentialAnimation {
                running: true // Starts immediately when component loads

                // Step A: Pause for second before doing anything
                PauseAnimation {
                    duration: 1000
                }

                // Step B: Smoothly transition opacity from 0.0 to 1.0 over 2 seconds
                NumberAnimation {
                    target: textContainer
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                    duration: 2000
                    easing.type: Easing.OutQuad
                }

                // Step C: Seamlessly trigger the continuous idle breathing scale loop
                PropertyAction {
                    target: breathingAnimation
                    property: "running"
                    value: true
                }
            }

            // Base Sharp Text
            Text {
                id: baseText
                text: "Smooth Glass Layer"
                font.pixelSize: 26
                font.weight: Font.DemiBold
                color: "#e0e0e0"

                layer.enabled: true
                layer.effect: MultiEffect {
                    shadowEnabled: true
                    shadowColor: Qt.rgba(0, 0, 0, 0.5)
                    shadowBlur: 0.3
                    shadowVerticalOffset: 2
                }
            }
        }

        // Pure QML Text Masking Overlay (No external shader files required)
        Item {
            anchors.fill: parent
            layer.enabled: true
            // This ensures the animation only draws inside the text boundaries
            layer.effect: MultiEffect {
                maskEnabled: true
                maskSource: ShaderEffectSource {
                    sourceItem: baseText
                }
            }

            // The Shimmer Gleam Strip
            Rectangle {
                id: shimmerGleam
                width: 80
                height: glassText.height * 2
                y: -glassText.height * 0.5
                rotation: 20

                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: "transparent"
                    }
                    GradientStop {
                        position: 0.5
                        color: Qt.rgba(1, 1, 1, 0.7)
                    }
                    GradientStop {
                        position: 1.0
                        color: "transparent"
                    }
                }

                // Corrected, sequential sweep loop animation
                SequentialAnimation {
                    running: true
                    loops: Animation.Infinite

                    NumberAnimation {
                        target: shimmerGleam
                        property: "x"
                        from: -120
                        to: glassText.width + 120
                        duration: 2000
                        easing.type: Easing.InOutQuad
                    }

                    PauseAnimation {
                        duration: 1500
                    }
                }
            }
        }
    }

    // Idle breathing/scale animation for the whole text layout
    SequentialAnimation {
        running: true
        loops: Animation.Infinite

        PropertyAnimation {
            target: glassText
            property: "scale"
            from: 1.0
            to: 1.04
            duration: 2500
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            target: glassText
            property: "scale"
            from: 1.04
            to: 1.0
            duration: 2500
            easing.type: Easing.InOutQuad
        }
    }
}
