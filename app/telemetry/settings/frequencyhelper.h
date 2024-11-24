#ifndef FREQUENCYHELPER_H
#define FREQUENCYHELPER_H

#include "qlist.h"
#include <mutex>
#include <qobject.h>
#include <vector>



class FrequencyHelper : public QObject
{
    Q_OBJECT
public:
    explicit FrequencyHelper(QObject *parent = nullptr);
    static FrequencyHelper &instance();
    // Filter: 0 - OpenHD 1-7 only, 1= all 2.4G freq, 2 = all 5.8G freq
    Q_INVOKABLE QList<int> get_frequencies(int filter);
    Q_INVOKABLE QList<int> filter_frequencies_40mhz_ht40plus_only(QList<int>);

    Q_INVOKABLE QList<int> get_frequencies_all_40Mhz();

    Q_INVOKABLE bool get_frequency_radar(int frequency_mhz);
    Q_INVOKABLE int get_frequency_openhd_race_band(int frequency_mhz);
    Q_INVOKABLE int get_frequency_openhd_licensed_band(int frequency_mhz);
    Q_INVOKABLE int get_frequency_channel_nr(int frequency_mhz);
    // --------------
    Q_INVOKABLE bool hw_supports_frequency_threadsafe(int frequency_mhz);
    //
    Q_INVOKABLE QString get_frequency_description(int frequency_mhz);

    Q_INVOKABLE QList<int> filter_frequencies(QList<int> frequencies,int filter_level);
public:
    bool set_hw_supported_frequencies_threadsafe(const std::vector<uint16_t> supported_channels);
    bool has_valid_supported_frequencies_data();
private:
    // Written by telemetry, read by UI
    std::mutex m_supported_channels_mutex;
    std::vector<uint16_t> m_supported_channels;
};

#endif // FREQUENCYHELPER_H
