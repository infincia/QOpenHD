import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import Qt.labs.settings 1.0

// This is a base implementation for all HUD widgets -
// e.g. it exposes the dragging around, clicking once and clicking long action(s)
// for clicking once, set hasWidgetDetail to true and provide the widgetDetailComponent
// same for long press
// Action widget: Opened when the user (short) clicks on the widget
// Detail widget: Opened when the user long clicks on the widget
BaseWidgetForm {
    id: widgetBase
    visible:false
    Drag.active: dragging
    Drag.dragType: Drag.Internal

    // Used to uniquely identify persistent settings for this widget. Should only contain lowercase letters and "_" (aka no blank space, extra characters and more)
    property string widgetIdentifier
    // Shown to the user as a more verbose description of what this widget does
    property string bw_verbose_name: "PLS FILL ME"

    // Show a "hand" icon to let the user know where to touch to drag the widget around.
    // Only used by "MAP" right now (to my knowledge)
    property bool useDragHandle: false
    // I think you can set this one to 2 for example if you want to align from the lower right corner
    property int defaultAlignment: 0
    property int defaultXOffset: 0
    property int defaultYOffset: 0
    property bool defaultHCenter: false
    property bool defaultVCenter: false

    property int detailPanelFontPixels: 13

    property bool dragging: false

    property bool isFullScreen: false

    property string alignmentIdentifier: "%1_align".arg(widgetIdentifier);
    property string xOffsetIdentifier: "%1_x_offset".arg(widgetIdentifier);
    property string yOffsetIdentifier: "%1_y_offset".arg(widgetIdentifier);
    property string hCenterIdentifier: "%1_h_center".arg(widgetIdentifier);
    property string vCenterIdentifier: "%1_v_center".arg(widgetIdentifier);
    property double oldOpacity: 100;

    // Feature persist scale begin --------------------------------------------------------------------------------------
    // Pretty much all widgets have a scale (previously often called size) setting
    // Properly resizing the widget elements itself is the responsibility of the upper implementation -
    // This base class implements the neccessary tools to resize, though. In short, we do
    // 1) Store the scale unique for this widget persistently
    // 2) expose its current value via a property variable
    // 3) have a simple util method to set the scale
    property string bw_scale_identifier: "%1_scale".arg(widgetIdentifier);
    // Default scale is 1, the value is persistent
    property double bw_current_scale : settings.value(bw_scale_identifier,1.0);
    // Updates the current base widget scale (unique per widgetIdentifier) and persist the value for later use
    function bw_set_current_scale(scale){
        if(scale <=0 || scale>=500){
            console.warn("perhaps invalid widget scale");
        }
        bw_current_scale=scale
        settings.setValue(bw_scale_identifier, bw_current_scale);
        settings.sync();
    }
    // Feature persist scale end   --------------------------------------------------------------------------------------

    // Feature persist opacity begin ------------------------------------------------------------------------------------
    property string bw_opacity_identifier: "%1_opacity".arg(widgetIdentifier);
    // Default opacity is 1, the value is persistent
    property double bw_current_opacity : settings.value(bw_opacity_identifier,1.0);
    // Updates the current base widget scale (unique per widgetIdentifier) and persist the value for later use
    function bw_set_current_opacity(opacity){
        if(opacity <=0 || opacity>1){
            console.warn("perhaps invalid widget opacity");
        }
        bw_current_opacity=opacity
        settings.setValue(bw_opacity_identifier, bw_current_opacity);
        settings.sync();
    }
    // Feature persist opacity end --------------------------------------------------------------------------------------

    // Feature persist 'qquickpainteditem_font_scale' begin  ------------------------------------------------------------
    // only for qquick painted items (few)
    property string bw_qquickpainteditem_font_scale_identifier: "%1_qquickpainteditem_font_scale".arg(widgetIdentifier);
    property double bw_qquickpainteditem_font_scale: settings.value(bw_qquickpainteditem_font_scale_identifier,1.0);
    function bw_set_qquickpainteditem_font_scale(scale){
        bw_qquickpainteditem_font_scale=scale;
        settings.setValue(bw_qquickpainteditem_font_scale_identifier,bw_qquickpainteditem_font_scale);
        settings.sync();
    }
    // Feature persist 'qquickpainteditem_font_scale' end  --------------------------------------------------------------


    // Feature: Show grid when dragging
    property bool m_show_grid_when_dragging: false
    onDraggingChanged: {
        if(dragging && m_show_grid_when_dragging){
            hudOverlayGrid.m_show_horizontal_center_indicator=true;
            hudOverlayGrid.m_show_vertical_center_indicator=true
        }else{
            hudOverlayGrid.m_show_horizontal_center_indicator=false;
            hudOverlayGrid.m_show_vertical_center_indicator=false;
        }
    }



    // Added by Consti10 -
    // disable dragging for this widget (dragging is enabled by default)
    // Needed for the HUD log messages element, since it sits above the secondary video and disabling its "mouse area"
    // Was the easiest fix
    property bool disable_dragging: false
    // ---------------------------------------------------------------------
    // Custom keyboard KeyNavigation
    // DIRTY
    function dirty_open_action_popup(){
        widgetAction.open()
    }
    function dirty_close_action_popup(){
        widgetAction.close()
    }

    function dirty_open_close_action_popup(){
        if(widgetAction.opened){
            widgetAction.close()
        }else{
            widgetAction.open()
        }
    }


    // Can be called by the imp. to manually close the action popup
    function bw_manually_close_action_popup(){
        widgetAction.close()
    }

    // react to the user opening the (currently focused) widget
    Keys.onPressed: (event)=> {
        // console.log("BaseWidget "+widgetIdentifier+" Key was pressed:"+event);
        if (event.key == Qt.Key_Return) {
            console.log("enter was pressed");
            event.accepted = true;
            dirty_open_action_popup()
        }else if(event.key == Qt.Key_Left){
            console.log("left was pressed")
            event.accepted=true;
            // TODO: Go to the next item
            event.accepted=true;
        }else if(event.key == Qt.Key_Right){
            console.log("right was pressed")
            event.accepted=true;
            // TODO: Go to the next item
        }
    }

    //layer.enabled: false

    /*Connections {
        target: link
        function onWidgetLocation(widgetName, alignment, xOffset, yOffset, hCenter, vCenter, full) {
            if (widgetIdentifier === widgetName) {
                setAlignment(alignment, xOffset, yOffset, hCenter, vCenter, false);
            }
        }
    }*/

    Component.onCompleted: {
        loadAlignment();
        saveAlignment();
        var _hCenter = settings.value(hCenterIdentifier, defaultHCenter)
        settings.setValue(hCenterIdentifier, _hCenter)
        var _vCenter = settings.value(vCenterIdentifier, defaultVCenter)
        settings.setValue(vCenterIdentifier, _vCenter)
        settings.sync();
    }

    SequentialAnimation {
        running: dragging

        onFinished: {
            if (dragging) {
                restart()
            }
        }
        alwaysRunToEnd: true

        animations: [
            RotationAnimator { target: dragHandle; from: 0; to: 2; duration: 200 * 0.4 },
            RotationAnimator { target: dragHandle; from: 2; to: 0; duration: 200 * 0.4 },
            RotationAnimator { target: dragHandle; from: 0; to: -2; duration: 200 * 0.4 },
            RotationAnimator { target: dragHandle; from: -2; to: 0; duration: 200 * 0.4 }
        ]
    }

    Timer {
        interval: 50;
        running: dragging;
        repeat: true
        onTriggered: {
            var point = calculateOffsets();
            var _xOffset = point.x;
            var _yOffset = point.y;

            var _hCenter = getHCenter();
            var _vCenter = getVCenter();
        }
    }

    function _onClicked(drag) {
        if (dragging) {
            drag.target = null
            widgetDetail.close()
            saveAlignment()
            loadAlignment()
            dragging = false
            globalDragLock = false
        } else if (hasWidgetPopup) {
            widgetPopup.open()
        } else if (hasWidgetAction) {
            if (widgetAction.visible) {
                widgetAction.close()
            }
            else if (widgetDetail.visible) {
                widgetDetail.close()
                dragging = false

                //widgetControls.close()

                saveAlignment()

                loadAlignment()
                globalDragLock = false
            }
            else {
                if (globalDragLock) {
                    return;
                }
                // A bit dirty - different action
                if(widgetIdentifier=="wb_link_rate_control_widget"){
                    sidebar.open_link_category();
                }else{
                    widgetAction.open()
                }
            }
        }
    }

    function _onPressAndHold(drag) {
        if (!dragging) {
            if (globalDragLock) {
                return;
            }
            globalDragLock = true
            dragging = true

            /*
                 * Unlock the element anchors so it can be dragged. They'll be enabled
                 * again when the widget is done being moved, if the selected alignment
                 * type requires them.
                 */
            resetAnchors()
            drag.target = widgetBase
            //widgetControls.open() ///-------------this is the arrow window
            //widgetDetail.open()

        } else {
            drag.target = null
            widgetDetail.close()
            saveAlignment()
            loadAlignment()
            dragging = false
            globalDragLock = false
        }
    }


    Text {
        id: dragHandle
        color: "white"
        text: "\uf256"
        font.family: "Font Awesome 5 Free"
        z: 2.0
        visible: useDragHandle && !isFullScreen
        enabled: useDragHandle && !isFullScreen
        width: 24
        height: 24
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        MouseArea {
            id: dragHandleMouseArea
            z: 3.0
            enabled: useDragHandle
            anchors.fill: parent
            onClicked: {
                _onClicked(drag)
            }
            onPressAndHold: {
                oldOpacity = dragHandle.opacity
                dragHandle.opacity = 100
                _onPressAndHold(drag)
            }
            onReleased: {
                dragHandle.opacity = oldOpacity
                _onClicked(drag)
            }
        }
    }


    MouseArea {
        id: dragArea

        enabled: ! disable_dragging;

        anchors.fill: parent

        onClicked: { _onClicked(drag) }

        onPressAndHold: { _onPressAndHold(drag) }

        onReleased: {
            if (dragging) widgetDetail.open();
        }

    }

    function calculateOffsets() {
        var _xOffset;
        var _yOffset;

        switch (alignmentType) {
        case 0:
            _xOffset = x;
            _yOffset = y;
            break;
        case 1:
            _xOffset = parent.width - (x + width);
            _yOffset = y;
            break;
        case 2:
            _xOffset = parent.width - (x + width);
            _yOffset = parent.height - (y + height);
            break;
        case 3:
            _xOffset = x;
            _yOffset = parent.height - (y + height);
            break;
        }
        if (_xOffset+width>parent.width) {
        _xOffset = parent.width-width}
        if (_xOffset<0){
        _xOffset = 0}
        if (_yOffset<0){
        _yOffset = 0}

        return Qt.point(_xOffset, _yOffset);
    }

    function saveAlignment() {
        var point = calculateOffsets();
        var _xOffset = point.x;
        var _yOffset = point.y;

        settings.setValue(alignmentIdentifier, alignmentType);
        settings.setValue(xOffsetIdentifier, _xOffset);
        settings.setValue(yOffsetIdentifier, _yOffset);
        settings.sync();

        var _hCenter = getHCenter();
        var _vCenter = getVCenter();

        //link.setWidgetLocation(widgetIdentifier, alignmentType, _xOffset, _yOffset, _hCenter, _vCenter)
    }

    /*
     * This is completely absurd and shouldn't be necessary, but here we are...
     *
     * For some reason, this returns strings on Linux and booleans on every other platform.
     */
    function getHCenter() {
        var _hCenter = settings.value(hCenterIdentifier, defaultHCenter);
        if (_hCenter === "true" || _hCenter === 1 || _hCenter === true) {
            _hCenter = true;
        } else {
            _hCenter = false;
        }
        return _hCenter;
    }

    function getVCenter() {
        var _vCenter = settings.value(vCenterIdentifier, defaultVCenter);
        if (_vCenter === "true" || _vCenter === 1 || _vCenter === true) {
            _vCenter = true;
        } else {
            _vCenter = false;
        }
        return _vCenter;
    }

    function loadAlignment() {
        alignmentType = settings.value(alignmentIdentifier, defaultAlignment);

        var _hCenter = getHCenter();
        var _vCenter = getVCenter();

        var xOffset = settings.value(xOffsetIdentifier, defaultXOffset);
        var yOffset = settings.value(yOffsetIdentifier, defaultYOffset);

        setAlignment(alignmentType, xOffset, yOffset, _hCenter, _vCenter, false);
    }

    function setAlignment(alignmentType, xOffset, yOffset, hCenter, vCenter, full) {
        /*
         * Anchors have to be cleared before any of them can be set again. This is documented
         * at https://doc.qt.io/qt-5/qtquick-positioning-anchors.html#changing-anchors
         */
        resetAnchors();
        isFullScreen = full;

        if (full) {
            anchors.top = parent.top;
            anchors.topMargin = yOffset;
            anchors.bottom = parent.bottom;
            anchors.bottomMargin = yOffset;
            anchors.left = parent.left;
            anchors.leftMargin = xOffset;
            anchors.right = parent.right;
            anchors.rightMargin = xOffset;
        } else if (alignmentType === 0) {
            if (vCenter) {
                anchors.verticalCenter = parent.verticalCenter;
            } else {
                anchors.top = parent.top;
                anchors.topMargin = yOffset;
            }
            if (hCenter) {
                anchors.horizontalCenter = parent.horizontalCenter;
            } else {
                anchors.left = parent.left;
                anchors.leftMargin = xOffset;
            }
        } else if (alignmentType === 1) {
            if (vCenter) {
                anchors.verticalCenter = parent.verticalCenter;
            } else {
                anchors.top = parent.top;
                anchors.topMargin = yOffset;
            }
            if (hCenter) {
                anchors.horizontalCenter = parent.horizontalCenter;
            } else {
                anchors.right = parent.right;
                anchors.rightMargin = xOffset;
            }
        } else if (alignmentType === 2) {
            if (vCenter) {
                anchors.verticalCenter = parent.verticalCenter;
            } else {
                anchors.bottom = parent.bottom;
                anchors.bottomMargin = yOffset;
            }
            if (hCenter) {
                anchors.horizontalCenter = parent.horizontalCenter;
            } else {
                anchors.right = parent.right;
                anchors.rightMargin = xOffset;
            }
        } else if (alignmentType === 3) {
            if (vCenter) {
                anchors.verticalCenter = parent.verticalCenter;
            } else {
                anchors.bottom = parent.bottom;
                anchors.bottomMargin = yOffset;
            }
            if (hCenter) {
                anchors.horizontalCenter = parent.horizontalCenter;
            } else {
                anchors.left = parent.left;
                anchors.leftMargin = xOffset;
            }
        }
    }


    function moveUp() { widgetBase.y -= 1; }
    function moveRight() { widgetBase.x += 1; }
    function moveLeft() { widgetBase.x -= 1; }
    function moveDown() { widgetBase.y += 1; }


    function resetAnchors() {
        widgetBase.anchors.top = undefined;
        widgetBase.anchors.topMargin = 0;
        widgetBase.anchors.right = undefined;
        widgetBase.anchors.rightMargin = 0;
        widgetBase.anchors.left = undefined;
        widgetBase.anchors.leftMargin = 0;
        widgetBase.anchors.bottom = undefined;
        widgetBase.anchors.bottomMargin = 0;
        widgetBase.anchors.verticalCenter = undefined;
        widgetBase.anchors.horizontalCenter = undefined;
    }
}
