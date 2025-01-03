#include "xparam.h"
#include "tutil/qopenhdmavlinkhelper.hpp"

#include <cstring>
#include <sstream>
#include <future>

#include <qdebug.h>
#include "../../MavlinkTelemetry.h"

XParam::XParam()
{
    m_timeout_thread=std::make_unique<std::thread>([this](){
        loop_timeout();
    });
}

XParam::~XParam()
{
    terminate();
}

void XParam::terminate()
{
    if(m_timeout_thread){
        m_timeout_thread_run=false;
        m_timeout_thread->join();
        m_timeout_thread=nullptr;
    }
}

XParam &XParam::instance()
{
    static XParam instance;
    return instance;
}

bool XParam::process_message(const mavlink_message_t &msg)
{
    if(msg.msgid==MAVLINK_MSG_ID_PARAM_EXT_ACK){
        mavlink_param_ext_ack_t ack;
        mavlink_msg_param_ext_ack_decode(&msg,&ack);
        return handle_param_ext_ack(ack,msg.sysid,msg.compid);
    }else if(msg.msgid==MAVLINK_MSG_ID_PARAM_EXT_VALUE){
        mavlink_param_ext_value_t response;
        mavlink_msg_param_ext_value_decode(&msg,&response);
        return handle_param_ext_value(response,msg.sysid,msg.compid);
    }
    return false;
}

bool XParam::try_set_param_async(const mavlink_param_ext_set_t cmd, SET_PARAM_RESULT_CB result_cb,PROGRESS_CB opt_progress_cb, std::chrono::milliseconds retransmit_delay, int n_wanted_retransmissions)
{
    assert(n_wanted_retransmissions>=1);
    assert(retransmit_delay.count()>=10);
    if(!result_cb){
        // the cb must not be nullptr
        qDebug()<<"No result cb,using dummy";
        auto dummy_cb=[](SetParamResult result){
            //qDebug()<<run_command_result_as_string(result).c_str();
        };
        result_cb=dummy_cb;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_running_commands.size()>=MAX_N_SIMULTANOEUS_COMMANDS){
        std::stringstream ss;
        ss<<"cannot enqueue, curr:"<<m_running_commands.size()<<" max:"<<MAX_N_SIMULTANOEUS_COMMANDS;
        qDebug()<<ss.str().c_str();
        return false;
    }
    RunningParamCmdSet running_cmd{cmd,result_cb,opt_progress_cb,n_wanted_retransmissions,retransmit_delay};
    m_running_commands.push_back(running_cmd);
    send_next_message_running_set(m_running_commands.back());
    return true;
}

bool XParam::try_set_param_blocking(const mavlink_param_ext_set_t cmd,std::chrono::milliseconds retransmit_delay, int n_wanted_retransmission)
{
    std::promise<bool> prom;
    auto fut = prom.get_future();
    auto cb=[&prom](SetParamResult result){
        prom.set_value(result.is_accepted());
    };
    if(!try_set_param_async(cmd,cb,nullptr,retransmit_delay,n_wanted_retransmission)){
        qDebug()<<"Cannot enqueue param ";
        return false;
    }
    return fut.get();
}

void XParam::try_get_param_all_async(const mavlink_param_ext_request_list_t cmd, GET_ALL_PARAM_RESULT_CB result_cb,PROGRESS_CB opt_progress_cb)
{
    if(!result_cb){
        // the cb must not be nullptr
        qDebug()<<"No result cb,using dummy";
        auto dummy_cb=[](GetAllParamResult result){
            //qDebug()<<run_command_result_as_string(result).c_str();
        };
        result_cb=dummy_cb;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    RunningParamCmdGetAll running{cmd,result_cb,opt_progress_cb,std::chrono::milliseconds(3000),std::chrono::milliseconds(500),std::chrono::steady_clock::now(),{},10,0};
    m_running_get_all.push_back(running);
    send_next_message_running_get_all(m_running_get_all.back());
}

std::optional<std::vector<mavlink_param_ext_value_t>> XParam::try_get_param_all_blocking(const int target_sysid, const int target_compid)
{
    std::promise<std::optional<std::vector<mavlink_param_ext_value_t>>> prom;
    auto fut = prom.get_future();
    auto cb=[&prom](GetAllParamResult result){
        if(result.success){
            prom.set_value(result.param_set);
        }else{
            prom.set_value(std::nullopt);
        }
    };
    const auto command=create_cmd_get_all(target_sysid,target_compid);
    try_get_param_all_async(command,cb);
    return fut.get();
}

// allowed: up to 16 bytes, either including or excluding the 0-terminator
static void set_param_id(char* dest,const std::string& source){
    if(source.length()>16){
        qWarning("Invalid param name %s",source.c_str());
    }
    const int len =  source.length()>=16 ? 16 :  source.length()+1;
    std::memcpy(dest, source.c_str(),len);
}
static std::string get_param_id(const char* param_id){
    // The param_id field of the MAVLink struct has length 16 and can not be null terminated.
    // Therefore, we make a 0 terminated copy first.
    char param_id_long_enough[16 + 1] = {};
    std::memcpy(param_id_long_enough, param_id, 16);
    return {param_id_long_enough};
}
static void set_param_value_int(char* param_value,int value){
    std::memset(param_value,0,128);
    int32_t tmp=value;
    std::memcpy(param_value, &tmp, sizeof(tmp));
}
static void set_param_value_string(char* param_value,const std::string& value){
    std::memset(param_value,0,128);
    const int len =  value.length()>=128 ? 128 :  value.length()+1;
    std::memcpy(param_value,value.c_str(),len);
}
static int get_param_value_int(const char* param_value){
    int32_t ret;
    std::memcpy(&ret,param_value,sizeof(ret));
    return ret;
}
static std::string get_param_value_string(const char* param_value){
    char param_value_long_enough[128 + 1] = {};
    std::memcpy(param_value_long_enough, param_value,128);
    return {param_value_long_enough};
}


mavlink_param_ext_set_t XParam::create_cmd_set_int(int target_sysid, int target_compid, std::string param_name, int value)
{
    mavlink_param_ext_set_t cmd{};
    cmd.target_system=target_sysid;
    cmd.target_component=target_compid;
    set_param_id(cmd.param_id,param_name);
    cmd.param_type=MAV_PARAM_EXT_TYPE_INT32;
    set_param_value_int(cmd.param_value,value);
    return cmd;
}

mavlink_param_ext_set_t XParam::create_cmd_set_string(int target_sysid, int target_compid, std::string param_id, std::string value)
{
    mavlink_param_ext_set_t cmd{};
    cmd.target_system=target_sysid;
    cmd.target_component=target_compid;
    set_param_id(cmd.param_id,param_id);
    cmd.param_type=MAV_PARAM_EXT_TYPE_CUSTOM;
    set_param_value_string(cmd.param_value,value);
    return cmd;
}

mavlink_param_ext_request_list_t XParam::create_cmd_get_all(int target_sysid, int target_compid)
{
    mavlink_param_ext_request_list_t cmd{};
    cmd.target_system=target_sysid;
    cmd.target_component=target_compid;
    return cmd;
}

std::vector<XParam::ParamVariant> XParam::parse_server_param_set(const std::vector<mavlink_param_ext_value_t> &param_set)
{
    std::vector<ParamVariant> ret;
    ret.reserve(param_set.size());
    for(int i=0;i<param_set.size();i++){
        const auto& param_ext_value=param_set[i];
        ParamVariant param_variant{get_param_id(param_ext_value.param_id),std::nullopt,std::nullopt};
        if(param_ext_value.param_type==MAV_PARAM_EXT_TYPE_INT32){
            param_variant.int_param=get_param_value_int(param_ext_value.param_value);
        }else if(param_ext_value.param_type==MAV_PARAM_EXT_TYPE_CUSTOM){
            param_variant.string_param=get_param_value_string(param_ext_value.param_value);
        }else{
            qDebug()<<"Unsupported param type:"<<(int)param_ext_value.param_type;
        }
        ret.push_back(param_variant);
    }
    return ret;
}


bool XParam::handle_param_ext_ack(const mavlink_param_ext_ack_t &ack,int sender_sysid,int sender_compid)
{
    auto opt_running_cmd_set=find_remove_running_command_threadsafe(ack,sender_sysid,sender_compid);
    if(opt_running_cmd_set.has_value()){
        //qDebug()<<"Got consumer for mavlink_param_ext_ack_t";
        auto running_cmd_set=opt_running_cmd_set.value();
        SetParamResult result{ack,running_cmd_set.n_transmissions};
        running_cmd_set.cb(result);
        return true;
    }
    //qDebug()<<"Got no consumer for mavlink_param_ext_ack_t";
    return false;
}

bool XParam::handle_param_ext_value(const mavlink_param_ext_value_t &response, int sender_sysid, int sender_compid)
{
    qDebug() << "Incoming PARAM_EXT_VALUE:";
    qDebug() << "  Param ID:" << get_param_id(response.param_id).c_str();
    qDebug() << "  Param Value:" << get_param_value_string(response.param_value).c_str();
    qDebug() << "  Param Type:" << response.param_type;
    qDebug() << "  Target SysID:" << sender_sysid;
    qDebug() << "  Target CompID:" << sender_compid;
    qDebug() << "  Param Count:" << response.param_count;
    qDebug() << "  Param Index:" << response.param_index;

    auto opt_finished = find_remove_running_command_get_all_threadsafe(response, sender_sysid, sender_compid);
    if (opt_finished.has_value()) {
        auto finished = opt_finished.value();
        std::vector<mavlink_param_ext_value_t> valid_param_set;
        for (auto &param : finished.server_param_set) {
            assert(param.has_value());
            valid_param_set.push_back(param.value());
        }
        GetAllParamResult result{true, valid_param_set};
        finished.cb(result);
        return true;
    }
    return true;
}

std::optional<XParam::RunningParamCmdSet> XParam::find_remove_running_command_threadsafe(const mavlink_param_ext_ack_t &ack,int sender_sysid,int sender_compid)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it=m_running_commands.begin(); it!=m_running_commands.end(); ++it){
        const auto& running=*it;
        const auto& cmd=running.cmd;
        const auto safe_cmd_param_id=get_param_id(cmd.param_id);
        const auto safe_ack_param_id=get_param_id(ack.param_id);
        if(safe_cmd_param_id==safe_ack_param_id && cmd.param_type==ack.param_type && cmd.target_system==sender_sysid && cmd.target_component==sender_compid){
            RunningParamCmdSet tmp=*it;
            m_running_commands.erase(it);
            return tmp;
        }else{
            //qDebug()<<"Self:"<<safe_cmd_param_id.c_str()<<" other:"<<safe_ack_param_id.c_str();
        }
    }
    return std::nullopt;
}

std::optional<XParam::RunningParamCmdGetAll> XParam::find_remove_running_command_get_all_threadsafe(const mavlink_param_ext_value_t &response, int sender_sysid, int sender_compid)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it=m_running_get_all.begin(); it!=m_running_get_all.end(); ++it){
        RunningParamCmdGetAll& running=*it;
        if(running.base_cmd.target_system==sender_sysid && running.base_cmd.target_component==sender_compid){
            // The response fromt the server can be cosumed by a running "Get all params" action
            if(response.param_count<=0){
                qWarning()<<"Invalid param count";
                return std::nullopt;
            }
            if(response.param_index>=response.param_count){
                qWarning()<<"Invalid param index";
                return std::nullopt;
            }
            if(running.server_param_set.empty()){
                // Size is not yet known
                running.server_param_set.resize(response.param_count);
                running.server_param_set[response.param_index]=response;
                update_progress_get_all(running);
                const int missing=get_missing_count(running.server_param_set);
                if(missing==0){
                    qDebug()<<"No params missing, total:"<<running.server_param_set.size();
                    // We have all the params from this server
                    RunningParamCmdGetAll tmp_copy=running;
                    it=m_running_get_all.erase(it);
                    return tmp_copy;
                }
            }else{
                // Size is known, check if we already have this param
                if(running.server_param_set[response.param_index]!=std::nullopt){
                    // we already have this param - nothing to do, we could validate it though
                    return std::nullopt;
                }else{
                    running.server_param_set[response.param_index]=response;
                    update_progress_get_all(running);
                    const int missing=get_missing_count(running.server_param_set);
                    if(missing==0){
                        qDebug()<<"No params missing, total:"<<running.server_param_set.size();
                        // We have all the params from this server
                        RunningParamCmdGetAll tmp_copy=running;
                        it=m_running_get_all.erase(it);
                        return tmp_copy;
                    }
                }
            }
        }
    }
    return std::nullopt;
}

void XParam::send_next_message_running_set(RunningParamCmdSet &cmd)
{
    send_param_ext_set(cmd.cmd);
    cmd.last_transmission=std::chrono::steady_clock::now();
    cmd.n_transmissions++;
}

void XParam::send_next_message_running_get_all(RunningParamCmdGetAll& running_cmd)
{
    if(running_cmd.server_param_set.size()==0){
        qDebug()<<"Size not yet known";
        // No message ever received yet, size not yet known
        send_param_ext_request_list(running_cmd.base_cmd);
    }else{
        // We know how many params the server provides, request until we have all of them
        assert(running_cmd.server_param_set.size()>0);
        const int n_missing=get_missing_count(running_cmd.server_param_set);
        qDebug()<<"Still missing:"<<n_missing<<" total:"<<running_cmd.server_param_set.size();
        if(n_missing>2 && n_missing >= running_cmd.server_param_set.size()/2){
            // A lot are stil missing, request them all again
            send_param_ext_request_list(running_cmd.base_cmd);
        }else{
            // Request up to X specific missing cmds at a time
            for(int i=0;i<10 && i<running_cmd.server_param_set.size();i++){
                mavlink_param_ext_request_read_t cmd{};
                cmd.param_index=i;
                cmd.target_system=running_cmd.base_cmd.target_system;
                cmd.target_component=running_cmd.base_cmd.target_component;
                send_param_ext_request_read(cmd);
            }
        }
    }
    running_cmd.last_transmission=std::chrono::steady_clock::now();
    running_cmd.n_transmissions++;
}

void XParam::send_param_ext_set(const mavlink_param_ext_set_t &cmd) {
    const auto self_sysid = QOpenHDMavlinkHelper::get_own_sys_id();
    const auto self_compid = QOpenHDMavlinkHelper::get_own_comp_id();
    mavlink_message_t msg{};
    mavlink_msg_param_ext_set_encode(self_sysid, self_compid, &msg, &cmd);

    // Display the MAVLink message details directly in this function
    qDebug() << "MAVLink Message:";
    qDebug() << "  Msg ID:" << msg.msgid;
    qDebug() << "  System ID:" << msg.sysid;
    qDebug() << "  Component ID:" << msg.compid;
    qDebug() << "  Command: PARAM_EXT_SET";
    qDebug() << "    Param ID:" << cmd.param_id;
    qDebug() << "    Param Value:" << cmd.param_value;
    qDebug() << "    Param Type:" << cmd.param_type;

    MavlinkTelemetry::instance().sendMessage(msg);
}

void XParam::send_param_ext_request_list(const mavlink_param_ext_request_list_t& cmd)
{
    const auto self_sysid=QOpenHDMavlinkHelper::get_own_sys_id();
    const auto self_compid=QOpenHDMavlinkHelper::get_own_comp_id();
    mavlink_message_t msg{};
    mavlink_msg_param_ext_request_list_encode(self_sysid,self_compid,&msg,&cmd);
    MavlinkTelemetry::instance().sendMessage(msg);
}

void XParam::send_param_ext_request_read(const mavlink_param_ext_request_read_t &cmd)
{
    const auto self_sysid=QOpenHDMavlinkHelper::get_own_sys_id();
    const auto self_compid=QOpenHDMavlinkHelper::get_own_comp_id();
    mavlink_message_t msg{};
    mavlink_msg_param_ext_request_read_encode(self_sysid,self_compid,&msg,&cmd);
    MavlinkTelemetry::instance().sendMessage(msg);
}

int XParam::get_missing_count(const std::vector<std::optional<mavlink_param_ext_value_t> > &server_param_set)
{
    int ret=0;
    for(const auto& param:server_param_set){
        if(!param.has_value())ret++;
    }
    return ret;
}

void XParam::update_progress_set(const RunningParamCmdSet &cmd,bool done)
{
    if(cmd.opt_progress_cb){
        if(done){
            cmd.opt_progress_cb(100.0);
        }else{
            const float progress=(float)cmd.n_transmissions/(float)cmd.n_wanted_retransmissions*100.0;
            cmd.opt_progress_cb(progress);
        }
    }
}

void XParam::update_progress_get_all(const RunningParamCmdGetAll &cmd)
{
    if(cmd.opt_progress_cb){
        float progress=0;
        if(cmd.server_param_set.size()>0){
            const int missing=get_missing_count(cmd.server_param_set);
            if(missing<=0){
                progress=100;
            }else{
                progress= (static_cast<float>(cmd.server_param_set.size()-missing) / static_cast<float>(cmd.server_param_set.size())*100.0f);
            }
        }
        cmd.opt_progress_cb(progress);
    }
}


void XParam::loop_timeout()
{
    while(m_timeout_thread_run){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        check_timeout_param_get_all();
        check_timeout_param_set();
        /*std::lock_guard<std::mutex> lock(m_mutex);
        const bool busy=!(m_running_commands.empty()&&m_running_get_all.empty());
        XParamUI::instance().set_is_busy(busy);*/
    }
}

void XParam::check_timeout_param_set()
{
    // Optimization: Call cb without lock being hold
    std::vector<std::pair<RunningParamCmdSet,SetParamResult>> timed_out_commands{};
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it=m_running_commands.begin(); it!=m_running_commands.end(); ++it){
            RunningParamCmdSet& running_cmd=*it;
            const auto elapsed=std::chrono::steady_clock::now()-running_cmd.last_transmission;
            if(elapsed>running_cmd.retransmit_delay){
                qDebug()<<"Param cmd set timeout";
                if(running_cmd.n_transmissions<running_cmd.n_wanted_retransmissions){
                    qDebug()<<"Param cmd retransmit "<<running_cmd.n_transmissions;
                    send_next_message_running_set(running_cmd);
                }else{
                    qDebug()<<"Timeout after "<<running_cmd.n_transmissions<<" transmissions";
                    SetParamResult result{std::nullopt,running_cmd.n_transmissions};
                    timed_out_commands.push_back(std::make_pair(running_cmd,result));
                    it=m_running_commands.erase(it);
                }
            }
        }
    }
    for(auto& timed_out:timed_out_commands){
        timed_out.first.cb(timed_out.second);
    }
}

void XParam::check_timeout_param_get_all()
{
    std::vector<std::pair<RunningParamCmdGetAll,GetAllParamResult>> timed_out_commands{};
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it=m_running_get_all.begin(); it!=m_running_get_all.end(); ++it){
            RunningParamCmdGetAll& running_cmd=*it;
            const auto elapsed=std::chrono::steady_clock::now()-running_cmd.last_transmission;
            if(elapsed>running_cmd.retransmit_delay){
                qDebug()<<"Param get all timeout";
                if(running_cmd.n_transmissions<running_cmd.n_wanted_retransmissions){
                    send_next_message_running_get_all(running_cmd);
                }else{
                    qDebug()<<"Timeout after "<<running_cmd.n_transmissions<<" transmissions";
                    GetAllParamResult result{false,{}};
                    timed_out_commands.push_back(std::make_pair(running_cmd,result));
                    it=m_running_get_all.erase(it);
                }
            }
        }
    }
    for(auto& timed_out:timed_out_commands){
        timed_out.first.cb(timed_out.second);
    }
}
