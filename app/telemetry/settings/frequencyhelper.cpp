#include "frequencyhelper.h"
#include "wifi_channel.h"

FrequencyHelper::FrequencyHelper(QObject *parent) : QObject{parent}
{

}

FrequencyHelper &FrequencyHelper::instance()
{
    static FrequencyHelper instance{};
    return instance;
}

QList<int> FrequencyHelper::get_frequencies(int filter)
{
    QList<int> ret;
    if(filter==0){
        auto tmp=openhd::get_openhd_channels_1_to_7();
        for(auto& channel:tmp){
            ret.push_back(channel.frequency);
        }
    }else if(filter==3){
        auto tmp=openhd::get_openhd_channels_licensed();
        for(auto& channel:tmp){
            ret.push_back(channel.frequency);
        }
    }else{
        const auto frequency_items=openhd::get_all_channels_2G_5G();
        for(auto& item:frequency_items){
            if(item.is_legal_at_least_one_country){
                if(filter==1){
                    if(item.frequency<3000){
                        ret.push_back(item.frequency);
                    }
                }else{
                    if(item.frequency>=5180 && item.frequency<=5865){
                        ret.push_back(item.frequency);
                    }
                }
            }
        }
    }
    return ret;
}

QList<int> FrequencyHelper::filter_frequencies_40mhz_ht40plus_only(QList<int> frequencies)
{
    std::vector<uint32_t> frequencies2;
    for(auto& freq:frequencies){
        frequencies2.push_back(freq);
    }
    QList<int> ret;
    auto channels=openhd::frequencies_to_channels(frequencies2);
    for(auto& channel:channels){
        if(channel.in_40Mhz_ht40_plus){
            ret.push_back(channel.frequency);
        }
    }
    return ret;
}

QList<int> FrequencyHelper::get_frequencies_all_40Mhz()
{
    QList<int> ret;
    const auto frequency_items=openhd::get_all_channels_2G_5G();
    for(auto& item:frequency_items){
        if(item.space==openhd::WifiSpace::G2_4){
            if(item.is_legal_at_least_one_country){
                ret.push_back(item.frequency);
            }
        }else{
            if(item.is_legal_at_least_one_country && item.in_40Mhz_ht40_plus){
                ret.push_back(item.frequency);
            }
        }
    }
    return ret;
}

bool FrequencyHelper::get_frequency_radar(int frequency_mhz)
{
    if(frequency_mhz>=5260 && frequency_mhz<=5680){
        return true;
    }
    return false;
}

int FrequencyHelper::get_frequency_openhd_race_band(int frequency_mhz)
{
    // 5700,5745,5785,5825,5865,5260,5280
    if(frequency_mhz==5700){
        return 1;
    }
    if(frequency_mhz==5745 ){
        return 2;
    }
    if(frequency_mhz==5785){
        return 3;
    }
    if(frequency_mhz==5825){
        return 4;
    }
    if(frequency_mhz==5865){
        return 5;
    }
    if(frequency_mhz==5260){
        return 6;
    }
    if(frequency_mhz==5280){
        return 7;
    }
    return -1;
}
int FrequencyHelper::get_frequency_openhd_licensed_band(int frequency_mhz)
{
    // Frequencies: 5080,5100,5120,5140,5160,5905,5925,5945,5965,5985,6005,6025,6045,6065,6085
    if(frequency_mhz==5080){
        return 1;
    }
    if(frequency_mhz==5100){
        return 2;
    }
    if(frequency_mhz==5120){
        return 3;
    }
    if(frequency_mhz==5140){
        return 4;
    }
    if(frequency_mhz==5160){
        return 5;
    }
    if(frequency_mhz==5905){
        return 6;
    }
    if(frequency_mhz==5925){
        return 7;
    }
    if(frequency_mhz==5945){
        return 8;
    }
    if(frequency_mhz==5965){
        return 9;
    }
    if(frequency_mhz==5985){
        return 10;
    }
    if(frequency_mhz==6005){
        return 11;
    }
    if(frequency_mhz==6025){
        return 12;
    }
    if(frequency_mhz==6045){
        return 13;
    }
    if(frequency_mhz==6065){
        return 14;
    }
    if(frequency_mhz==6085){
        return 15;
    }
    return -1;
}


int FrequencyHelper::get_frequency_channel_nr(int frequency_mhz)
{
    const auto frequency_item=openhd::channel_from_frequency(frequency_mhz);
    if(frequency_item.has_value()){
        return frequency_item.value().channel;
    }
    return -1;
}

bool FrequencyHelper::set_hw_supported_frequencies_threadsafe(const std::vector<uint16_t> supported_channels)
{
    std::lock_guard<std::mutex> lock(m_supported_channels_mutex);
    if(m_supported_channels!=supported_channels){
        m_supported_channels=supported_channels;
        return true;
    }
    return false;
}

bool FrequencyHelper::has_valid_supported_frequencies_data()
{
    std::lock_guard<std::mutex> lock(m_supported_channels_mutex);
    return !m_supported_channels.empty();
}

bool FrequencyHelper::hw_supports_frequency_threadsafe(int frequency_mhz)
{
    std::lock_guard<std::mutex> lock(m_supported_channels_mutex);
    for(const auto supported_freq: m_supported_channels){
        if(supported_freq==frequency_mhz)return true;
    }
    return false;
}


static std::string spaced_string(int number){
    std::stringstream ss;
    if(number<100)ss<<" ";
    if(number<10)ss<<" ";
    ss<<number;
    return ss.str();
}

QString FrequencyHelper::get_frequency_description(int frequency_mhz)
{
    const auto frequency_item=openhd::channel_from_frequency(frequency_mhz);
    std::stringstream ss;
    ss<<"["<<spaced_string(frequency_item->channel)<<"] ";
    ss<<frequency_mhz<<"Mhz ";
    return ss.str().c_str();
}

QList<int> FrequencyHelper::filter_frequencies(QList<int> frequencies, int filter_level)
{
    QList<int> ret;
    for(const auto& element:frequencies){
        if(filter_level==2){
            if(element> 3000){
                ret.push_back(element);
            }
        }else if(filter_level==1){
            if(element< 3000){
                ret.push_back(element);
            }
        }else{
            ret.push_back(element);
        }
    }
    return ret;
}
