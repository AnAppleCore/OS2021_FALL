#ifndef MMA_SERVER_H
#define MMA_SERVER_H

#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <mutex>

#include <grpc++/grpc++.h>
#include <grpc++/ext/proto_server_reflection_plugin.h>
#include <grpc++/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "proto/mma.grpc.pb.h"
#else
#include "mma.grpc.pb.h"
#endif

#include "memory_manager.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using mma::MMAService;
using mma::AllocateRequest;
using mma::AllocateReply;
using mma::ReleaseRequest;
using mma::ReleaseReply;
using mma::ReadPageRequest;
using mma::ReadPageReply;
using mma::WritePageRequest;
using mma::WritePageReply;



// Logic and data behind the server's behavior.

namespace proj4 {

class MmaServer final : public MMAService::Service {
public:
    //MmaServer(size_t phy_page_num) : mma(new MemoryManager(phy_page_num){}
    MmaServer(size_t phy_page_num, size_t max_vir_page_num) {
        this->mma = new MemoryManager(phy_page_num);
        this->max_vir_page_num = max_vir_page_num;
    }
    ~MmaServer() {delete this->mma;}

    Status Allocate (ServerContext* context, const AllocateRequest*  request, AllocateReply*  reply) override;
    Status Release  (ServerContext* context, const ReleaseRequest*   request, ReleaseReply*   reply) override;
    Status ReadPage (ServerContext* context, const ReadPageRequest*  request, ReadPageReply*  reply) override;
    Status WritePage(ServerContext* context, const WritePageRequest* request, WritePageReply* reply) override;

private:
    MemoryManager* mma;
    size_t max_vir_page_num = 0;
    size_t total_vir_page_num = 0;
    std::mutex server_lock;
};

// setup a server with UnLimited virtual memory space
void RunServerUL(size_t phy_page_num);

// setup a server with Limited virtual memory space
void RunServerL(size_t phy_page_num, size_t max_vir_page_num);

// shutdown the server setup by RunServerUL or RunServerL
void ShutdownServer();

} //namespace proj4

#endif