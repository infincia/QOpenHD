import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import OpenHD 1.0

import Qt.labs.settings 1.0

import "./widgets"
import "./widgets/map"
import "../resources" as Resources
import "./elements"
import "./sidebar"

import "../video"

import "sidebar"

Item {
    id: hudOverlayGrid
    focus: true
    // cross to indicate horizontal / vertical center
    // used by BaseWidget
    property bool m_show_vertical_center_indicator: false
    property bool m_show_horizontal_center_indicator: false

    //scale: settings.dev_some_scale

    //signal settingsButtonClicked
    property int m_highlight_index : 0
    property int m_MAX_ITEM_INDEX: 15
    property bool m_keyboard_navigation_active: false
    onM_keyboard_navigation_activeChanged: {
        if(m_keyboard_navigation_active){
            _qopenhd.show_toast("JOYSTICK NAVIGATION ENABLED");
        }else{
            _qopenhd.show_toast("JOYSTICK NAVIGATION DISABLED");
        }
    }

    property bool globalDragLock: false

    // Called by the config popup when closed to give focus back
    function regain_focus(){
        hudOverlayGrid.focus = true;
        // Receive key events
        hudOverlayGrid.enabled =true;
        hudOverlayGrid.visible =true;
        // argh
        actual_hud_elements.visible=true;
    }
    // Opens the config popup and gives it focus
    function open_config_popup(){
        // The HUD becomes not interactable while configpopup is opened
        hudOverlayGrid.enabled = false;
        actual_hud_elements.visible=false;
        settings_panel.openSettings();
    }

    function start_keyboard_navigation(){
        // Enable keyboard navigation (highlight), then return
        m_keyboard_navigation_active=true
        // Reset index
        m_highlight_index=0;
        highlight_specific_item(m_highlight_index)
        // Now the user can move around
    }
    function stop_keyboard_navigation(){
        m_keyboard_navigation_active=false
        m_highlight_index=0;
        unhighlight_all_items();
        close_popup_all_items();
    }

    function dummy_joystick_enter(){
        if(!m_keyboard_navigation_active){
            // Enable keyboard navigation (highlight), then return
            start_keyboard_navigation();
            // Now the user can move around
            return;
        }
        if(m_highlight_index==0){
            open_config_popup();
        }else{
            // Open the popup of the currently "highlighted" widget
            open_close_configpopup_for_current_item()

        }
    }
    function dummy_joystick_left(){
        if(!m_keyboard_navigation_active){
            // Enable keyboard navigation (highlight), then return
            start_keyboard_navigation();
            // Now the user can move around
            return;
        }
        close_popup_all_items();
        if(m_highlight_index==0){
            stop_keyboard_navigation()
            return;
        }
        goto_previous_visible_item();
        highlight_specific_item(m_highlight_index);
    }
    function dummy_joystick_right(){
        if(!m_keyboard_navigation_active){
            // Enable keyboard navigation (highlight), then return
            start_keyboard_navigation();
            // Now the user can move around
            return;
        }
        close_popup_all_items()
        goto_next_visible_item();
        highlight_specific_item(m_highlight_index);
    }
    function dummy_joystick_up(){
        if(m_keyboard_navigation_active){
            stop_keyboard_navigation();
        }
    }
    function dummy_joystick_down(){
        if(m_keyboard_navigation_active){
            stop_keyboard_navigation();
        }
    }

    // Goes to the next item in order, but skipping invisible widgets
    function goto_next_visible_item(){
        m_highlight_index++;
        if(m_highlight_index>m_MAX_ITEM_INDEX){
            m_highlight_index=0;
            return;
        }
        if(dirty_get_item_by_index(m_highlight_index).visible){
            return;
        }
        goto_next_visible_item()
    }
    // Goes to the previous item in order, but skipping invisible widgets
    function goto_previous_visible_item(){
        m_highlight_index--;
        if(m_highlight_index<0){
            m_highlight_index=0;
            return;
        }
        if(dirty_get_item_by_index(m_highlight_index).visible){
            return;
        }
        goto_previous_visible_item()
    }

    function dirty_get_item_by_index(index){
        var ret=downlink
        if(index==0)return settingsButtonHighlight;
        if(index==1)return downlink;
        if(index==2)return wBLinkRateControlWidget;
        if(index==3)return qRenderStatsWidget;
        if(index==4)return bitrate1;
        if(index==5)return bitrate2;
        if(index==6)return air_status;
        if(index==7)return ground_status;
        if(index==8)return uplink;
        if(index==9)return air_battery;
        if(index==10)return flight_mode;
        if(index==11)return throttleWidget;
        if(index==12)return missionWidget;
        if(index==13)return gps;
        if(index==14)return home_distance;
        //if(index==13)return uplink;
        console.log("Invalid index");
        return ret;
    }

    function unhighlight_all_items(){
        // Un-highlight the settings button
        settingsButtonHighlight.visible=false;
        // Un-highlight all BaseWidget items
        for(let i = 1; i <= m_MAX_ITEM_INDEX; i++){
            dirty_get_item_by_index(i).m_special_highlight=false;
        }
    }
    function close_popup_all_items(){
        for(let i = 1; i <= m_MAX_ITEM_INDEX; i++){
            dirty_get_item_by_index(i).dirty_close_action_popup()
        }
    }

    function highlight_specific_item(index_to_highlight){
        unhighlight_all_items()
        if(index_to_highlight==0){
            settingsButtonHighlight.visible=true;
            return;
        }
        // highlight the item to highlight
        dirty_get_item_by_index(m_highlight_index).m_special_highlight=true;
    }

    function open_close_configpopup_for_current_item(){
        var tmp=dirty_get_item_by_index(m_highlight_index)
        // Is of type BaseWidget
        tmp.dirty_open_close_action_popup()
    }

    Keys.onPressed: (event)=> {
                        // console.log("HUDOverlayGrid::Key was pressed:"+event);
                        if(event.key==Qt.Key_Left || event.key == Qt.Key_Right || event.key == Qt.Key_Up || event.key == Qt.Key_Down){
                            // If the user presses any navigation key, we open up the sidebar and hand over the inputs to it
                            if(!sidebar.m_extra_is_visible){
                                sidebar.open_and_take_control(true);
                                event.accepted=true;
                            }
                        }
                        /*if (event.key == Qt.Key_Return) {
                            //console.log("enter was pressed");
                            event.accepted = true;
                            dummy_joystick_enter()
                        }else if(event.key == Qt.Key_Left){
                            //console.log("left was pressed")
                            event.accepted=true;
                            dummy_joystick_left();
                        }else if(event.key == Qt.Key_Right){
                            //console.log("right was pressed")
                            event.accepted=true;
                            dummy_joystick_right();
                        }else if(event.key == Qt.Key_Up){
                            //console.log("up was pressed")
                            event.accepted=true;
                            dummy_joystick_up()
                        }else if(event.key == Qt.Key_Down){
                            //console.log("down was pressed")
                            event.accepted=true;
                            dummy_joystick_down()
                        }*/
                    }

    Image {
        id: settingsButton
        width: 48
        height: 48
        fillMode: Image.PreserveAspectFit
        z: 2.2

        source: "../resources/ic128.png"
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.top: parent.top
        anchors.topMargin: 0
        // If the sidebar is activated, do not show the (button/image) that opens the advanced menu
        // since on devices with a funky ratio (e.g. extra wide, like most modern android phones)
        // the back button of the sidebar and this button conflict
        visible: !sidebar.m_extra_is_visible

        MouseArea {
            id: settingsButtonMouseArea
            anchors.fill: parent
            onClicked: {
                open_config_popup()
            }
        }
        Rectangle{
            id: settingsButtonHighlight
            border.width: 5
            border.color: "green"
            anchors.fill: parent
            visible: false
            color: "transparent"
        }
    }

    // Dirty, in between a HUD element and not a HUD element
    // placed here because of the focus issue
    SecondaryVideoGStreamer{
        id: secondary_video
    }

    Item{
        id: actual_hud_elements
        width: parent.width
        height: parent.height
        visible: !quickPanel.visible
        MouseArea{
            anchors.fill: parent
            onClicked: {
                sidebar.notify_sidebar_user_clicked_outside();
            }
            enabled: sidebar.visible
        }

        // By default on top row
        // --------------------------------------------------------------------------
        LinkDownRSSIWidget {
            id: downlink
        }
        WBLinkRateControlWidget{
            id: wBLinkRateControlWidget
        }
        // exp
        QRenderStatsWidget {
            id: qRenderStatsWidget
        }
        VideoBitrateWidgetPrimary {
            id: bitrate1
        }
        VideoBitrateWidgetSecondary {
            id: bitrate2
        }
        SOCStatusWidgetAir {
            id: air_status
        }
        SOCStatusWidgetGround {
            id: ground_status
        }
        LinkUpRSSIWidget {
            id: uplink
        }
        // ----------------------------------------------------------------------------
        // TODO SORT ME

        // + 0% cpu
        GroundPowerWidget {
            id: groundPowerWidget
        }

        // + 0% cpu
        AirBatteryWidget {
            id: air_battery
        }

        // + 0% cpu
        FlightModeWidget {
            id: flight_mode
        }

        // + 0% cpu
        GPSWidget {
            id: gps
        }

        // + 0% cpu
        HomeDistanceWidget {
            id: home_distance
        }

        // + 0% cpu
        FlightTimeWidget {
            id: flight_timer
        }

        // + 0% cpu
        FlightDistanceWidget {
            id: flight_distance
        }

        // + 0% cpu
        FlightMahWidget {
            id: flight_mah
        }

        // + 0% cpu
        FlightMahKmWidget {
            id: flight_mah_km
        }

        // + 0% cpu
        ImuTempWidget {
            id: imu_temp
        }

        // + 0% cpu
        PressTempWidget {
            id: press_temp
        }

        PressTempWidget2 {
            id: press_temp2
        }
        
        PressTempWidget3 {
            id: press_temp3
        }

        RCRssiWidget {
            id: rc_rssi_widget
        }

        AirspeedTempWidget {
            id: airspeed_temp
        }

        BatteryTempWidget {
            id: battery_temp
        }

        // + 0% cpu
        EscTempWidget {
            id: esc_temp
        }

        // + 12% cpu
        HorizonWidget {
            id: horizonWidget
        }

        PerformanceHorizonWidget2{
            id: performanceHorizonWidget
        }

        // + 7% cpu
        FpvWidget {
            id: fpvWidget
        }

        // + 4% cpu
        AltitudeWidget {
            id: altitudeWidget
        }

        // + 0% cpu
        AltitudeSecondWidget {
            id: altitudesecondWidget
        }

        // + 17% cpu
        SpeedWidget {
            id: speedWidget
        }

        SpeedSecondWidget {
            id: speedSecondWidget
        }

        // +3% cpu
        HeadingWidget {
            id: headingWidget
        }

        // + 0% cpu
        ArrowWidget {
            id: arrowWidget
        }

        // + 0% cpu
        ThrottleWidget {
            id: throttleWidget
            scale: 0.7
        }

        // + 0% cpu
        ControlWidget {
            id: controlWidget
            //   scale: 0.7
        }

        // + 0% cpu
        GPIOWidget {
            id: gpioWidget
        }

        // + 3% cpu
        VibrationWidget {
            id: vibrationWidget
        }

        VerticalSpeedSimpleWidget{
            id: vssimpleWidget
        }
        VerticalSpeedGaugeWidget{
            id: vsgaugewidget
        }

        // + 0% cpu
        WindWidget {
            id: windWidget
        }

        // + 3% cpu
        RollWidget {
            id: rollWidget
        }

        MissionWidget {
            id: missionWidget
        }

        AoaWidget {
            id: aoaWidget
        }

        MapWidget {
            id: mapWidget
        }

        ExampleWidget {
            id: exampleWidget
        }

        DistanceSensorWidget{
            id: distancesensorwidget
        }

        UAVTimeWiget{
            id: uavtimewidget
        }

        SideBarMain{
            id: sidebar
        }

        MessageHUD {
            id: messageHUD
        }

        X20OverheatWidget{

        }
    }

    // Extra element - allows customizing the OSD color(s) and more
    // TODO QT 6
    OSDCustomizer {
        id: osdCustomizer
        anchors.centerIn: parent
        visible: false
        z: 5.0
    }
    TapHandler {
        acceptedButtons: Qt.AllButtons
        onTapped: {
        }
        onLongPressed: {
            osdCustomizer.visible = true
        }
        grabPermissions: PointerHandler.CanTakeOverFromAnything
    }

    Label{
        text: "JOSTICK NAVIGATION ENABLED"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        visible: m_keyboard_navigation_active
        // style
        color: settings.color_text
        horizontalAlignment: Text.AlignVCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 20
        wrapMode: Text.NoWrap
        style: Text.Outline
        styleColor: settings.color_glow
        height: 50
    }
    // Shows center while dragging widgets
    Rectangle{
        width: 1
        height: parent.height
        color: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        visible: m_show_horizontal_center_indicator
    }
    Rectangle{
        width: parent.width
        height: 1
        color: "white"
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        visible: m_show_vertical_center_indicator
    }

    DevStreamingInfo{

    }
}



