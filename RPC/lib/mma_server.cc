#include "mma_server.h"

namespace proj4 {

    Status MmaServer::Allocate (ServerContext* context, const AllocateRequest* request, AllocateReply* reply) {
        size_t page_request = (int(request->size()) + int(PageSize) - 1) / int(PageSize);
        this->server_lock.lock();
        if ((this->max_vir_page_num <= this->total_vir_page_num+page_request) && (this->max_vir_page_num != 0)) {
            this->server_lock.unlock();
            return Status::CANCELLED;
        }
        else {
            total_vir_page_num += page_request;
            this->server_lock.unlock();
            int array_id = mma->Allocate(request->size());
            reply->set_array_id(array_id);
            return Status::OK;
        }
    }

    Status MmaServer::Release (ServerContext* context, const ReleaseRequest*   request, ReleaseReply* reply) {
        size_t page_used = mma->Release(request->array_id());
        this->server_lock.lock();
        total_vir_page_num += page_used;
        this->server_lock.unlock();
        return Status::OK;
    }

    Status MmaServer::ReadPage (ServerContext* context, const ReadPageRequest*  request, ReadPageReply* reply) {
        int value = mma->ReadPage(request->array_id(), request->virtual_page_id(), request->offset());
        reply->set_value(value);
        return Status::OK;
    }

    Status MmaServer::WritePage(ServerContext* context, const WritePageRequest* request, WritePageReply* reply) {
        mma->WritePage(request->array_id(), request->virtual_page_id(), request->offset(), request->value());
        return Status::OK;
    }

    // Declare the server itself here
    std::unique_ptr<Server> mma_server;

    void RunServerL(size_t phy_page_num, size_t max_vir_page_num) {
        std::string server_address("0.0.0.0:50051");
        MmaServer service(phy_page_num, max_vir_page_num);

        grpc::EnableDefaultHealthCheckService(true);
        grpc::reflection::InitProtoReflectionServerBuilderPlugin();
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        mma_server = std::unique_ptr<Server> (builder.BuildAndStart());
        std::cout << "Server listening on " << server_address << std::endl;
        mma_server->Wait();
    }

    void RunServerUL(size_t phy_page_num) {
        RunServerL(phy_page_num, 0);
    }

    void ShutdownServer() {
        std::cout << "Server shutdown " << std::endl;
        mma_server->Shutdown();
    }

} //namespace proj4