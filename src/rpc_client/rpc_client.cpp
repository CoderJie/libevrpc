/***************************************************************************
 *
 * Copyright (c) 2015 aishuyu, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file rpc_client.cpp
 * @author aishuyu(asy5178@163.com)
 * @date 2015/09/10 20:08:23
 * @brief
 *
 **/

#include "rpc_client.h"

#include "client_rpc_controller.h"
#include "util/rpc_util.h"

namespace libevrpc {

using std::string;

RpcClient::RpcClient(const string& config_file) :
    rpc_channel_ptr_(NULL),
    rpc_controller_ptr_(NULL),
    rpc_heartbeat_ptr_(NULL),
    center_client_heartbeat_ptr_(NULL),
    config_parser_instance_(ConfigParser::GetInstance(config_file)){

    InitClient(config_file);
}

RpcClient::~RpcClient() {
    if (NULL != rpc_heartbeat_ptr_) {
        rpc_heartbeat_ptr_->Stop();
        rpc_heartbeat_ptr_->Wait();
        delete rpc_heartbeat_ptr_;
    }

    if (NULL != center_client_heartbeat_ptr_) {
        center_client_heartbeat_ptr_->Stop();
        center_client_heartbeat_ptr_->Wait();
        delete center_client_heartbeat_ptr_;
    }

    if (NULL != rpc_channel_ptr_) {
        delete rpc_channel_ptr_;
    }

    if (NULL != rpc_controller_ptr_) {
        delete rpc_controller_ptr_;
    }
}

bool RpcClient::InitClient(const string& config_file) {
    const char* rpc_server_addr = config_parser_instance_.IniGetString("rpc_server:addr", "127.0.0.1");
    const char* rpc_server_port = config_parser_instance_.IniGetString("rpc_server:port", "9998");
    const char* hb_server_port = config_parser_instance_.IniGetString("heartbeat:port", "9999");
    bool distributed_mode = config_parser_instance_.IniGetBool("rpc_server:distributed_mode", false);
    bool hb_open = config_parser_instance_.IniGetBool("heartbeat:open", true);
    int32_t rpc_connection_timeout = config_parser_instance_.IniGetInt("connection:timeout", 10);

    if (distributed_mode) {
        center_client_heartbeat_ptr_ = new CenterClientHeartbeat(config_file);
        center_client_heartbeat_ptr_->Start();
        rpc_channel_ptr_ = new Channel(center_client_heartbeat_ptr_, rpc_server_port);
    } else if (NULL != rpc_server_addr && NULL != rpc_server_port) {
        rpc_channel_ptr_ = new Channel(rpc_server_addr, rpc_server_port);
    } else {
        rpc_channel_ptr_ = new Channel("127.0.0.1", "9999");
        PrintErrorInfo("Attention! rpc client cann't read config file! Init with local server address and default port:8899! \n");
    }

    if (hb_open) {
        rpc_heartbeat_ptr_ = new RpcHeartbeatClient(rpc_server_addr, hb_server_port, rpc_connection_timeout);
        rpc_heartbeat_ptr_->Start();
    }

    rpc_controller_ptr_ = new ClientRpcController();
    SetRpcConnectionInfo(1000, 1);
    return true;
}

RpcController* RpcClient::Status() {
    return rpc_controller_ptr_;
}

bool RpcClient::IsCallOk() {
    if (NULL == rpc_controller_ptr_) {
        return true;
    }
    return !rpc_controller_ptr_->Failed();
}

string RpcClient::GetErrorInfo() const {
    if (NULL == rpc_controller_ptr_) {
        return "";
    }
    return rpc_controller_ptr_->ErrorText();
}

Channel* RpcClient::GetRpcChannel() {
    return rpc_channel_ptr_;
}

bool RpcClient::OpenRpcAsyncMode() {
    if (NULL == rpc_channel_ptr_) {
        PrintErrorInfo("Maybe YOU DIDNOT call InitClient first! open the async failed!");
        exit(0);
    }
    rpc_channel_ptr_->OpenRpcAsyncMode();
    return true;
}

bool RpcClient::GetAsyncResponse(const string& method_name, Message* response) {
    if (NULL == rpc_channel_ptr_) {
        return false;
    }
    return rpc_channel_ptr_->GetAsyncResponse(method_name, response);
}

bool RpcClient::SetRpcConnectionInfo(int32_t rpc_timeout, int32_t try_time) {
    if (NULL == rpc_channel_ptr_) {
        return false;
    }
    rpc_channel_ptr_->SetConnectionInfo(rpc_timeout, try_time);
    return true;
}


}  // end of namespace libevrpc












/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
