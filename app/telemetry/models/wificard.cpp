#include "wificard.h"

#include "../../logging/hudlogmessagesmodel.h"

WiFiCard::WiFiCard(bool is_air,int card_idx,QObject *parent)
    : QObject{parent},m_is_air_card(is_air),m_card_idx(card_idx)
{

}

WiFiCard &WiFiCard::instance_gnd(int index)
{
    if(index==0){
        static WiFiCard gnd0{false,0};
        return gnd0;
    }else if(index==1){
        static WiFiCard gnd1{false,1};
        return gnd1;
    }else if(index==2){
        static WiFiCard gnd2{false,2};
        return gnd2;
    }else if(index==3){
        static WiFiCard gnd3{false,3};
        return gnd3;
    }
    assert(false);
}

WiFiCard &WiFiCard::instance_air()
{
    static WiFiCard air{true,0};
    return air;
}

void WiFiCard::process_mavlink(const mavlink_openhd_stats_monitor_mode_wifi_card_t &msg)
{
    set_alive(true);
    set_curr_rx_rssi_dbm(msg.rx_rssi_1);
    set_n_received_packets(msg.count_p_received);
    set_packet_loss_perc(msg.curr_rx_packet_loss_perc);
    if(m_tx_power >0 && m_tx_power!=msg.tx_power){
        // TX power changed
        if(m_is_air_card){
            std::stringstream ss;
            ss<<"Air TX Power "<<(int)msg.tx_power;
            HUDLogMessagesModel::instance().add_message_info(ss.str().c_str());
        }else{
            // All gnd cards use the same tx power
            if(m_card_idx==0){
                std::stringstream ss;
                ss<<"GND TX Power "<<(int)msg.tx_power;
                HUDLogMessagesModel::instance().add_message_info(ss.str().c_str());
            }
        }
    }
    set_tx_power(msg.tx_power);
    const bool disconnected=false;
    if(disconnected){
        const auto elapsed=std::chrono::steady_clock::now()-m_last_disconnected_warning;
        if(elapsed>=CARD_DISCONNECTED_WARNING_INTERVAL){
            m_last_disconnected_warning=std::chrono::steady_clock::now();
            std::stringstream message;
            message<<"Card "<<(int)msg.card_index<<" disconnected";
            HUDLogMessagesModel::instance().add_message_warning(message.str().c_str());
        }
    }
}

int WiFiCard::helper_get_gnd_curr_best_rssi()
{
    int best_rssi=-127;
    for(int i=0;i<4;i++){
        auto& card=instance_gnd(i);
        if(card.alive() && card.m_curr_rx_rssi_dbm>best_rssi){
            best_rssi=card.m_curr_rx_rssi_dbm;
        }
    }
    return best_rssi;
}

