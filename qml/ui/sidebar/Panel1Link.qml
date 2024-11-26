import QtQuick 2.0

SideBarBasePanel{
    override_title: "Link"


    function takeover_control(){
        scan_frequency_element.takeover_control();
    }

    Column {
        anchors.top: parent.top
        anchors.topMargin: secondaryUiHeight/8
        spacing: 5

        /*EditFrequencyElement{
            id: edit_frequency_element
            onGoto_previous: {
                sidebar.regain_control_on_sidebar_stack()
            }
            onGoto_next: {
                edit_channel_width_element.takeover_control()
            }
        }

        EditChannelWidthElement{
            id: edit_channel_width_element
            onGoto_previous: {
                edit_frequency_element.takeover_control();
            }
            onGoto_next: {
                edit_rate_element.takeover_control()
            }
        }

        EditRateElement{
            id: edit_rate_element
            onGoto_previous: {
               edit_channel_width_element.takeover_control()
            }
            onGoto_next: {
                sidebar.regain_control_on_sidebar_stack()
            }
        }*/
        MavlinkChoiceElement2{
            id:scan_frequency_element
            m_title: "Scan for Air"
            m_param_id: mPARAM_ID_FREQUENCY_SCAN
            m_settings_model: _ohdSystemGroundSettings
            onGoto_previous: {
                sidebar.regain_control_on_sidebar_stack();
            }
            onGoto_next: {
                edit_frequency_element.takeover_control()
            }
        }
        MavlinkChoiceElement2{
            id:edit_frequency_element
            m_title: "Frequency"
            m_param_id: mPARAM_ID_FREQUENCY
            m_settings_model: _ohdSystemAirSettingsModel
            onGoto_previous: {
                scan_frequency_element.takeover_control()
            }
            onGoto_next: {
                edit_channel_width_element.takeover_control()
            }
        }
        MavlinkChoiceElement2{
            id: edit_channel_width_element
            m_title: "Channel Width"
            m_param_id: mPARAM_ID_CHANNEL_WIDTH
            m_settings_model: _ohdSystemAirSettingsModel
            onGoto_previous: {
                edit_frequency_element.takeover_control();
            }
            onGoto_next: {
                edit_rate_element.takeover_control()
            }
        }
        MavlinkChoiceElement2{
            id: edit_rate_element
            m_title: "Bandwith"
            m_param_id: mPARAM_ID_RATE
            m_settings_model: _ohdSystemAirSettingsModel
            onGoto_previous: {
               edit_channel_width_element.takeover_control()
            }
            onGoto_next: {
                sidebar.regain_control_on_sidebar_stack()
            }
        }

    }
}
