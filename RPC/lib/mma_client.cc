#include "mma_client.h"
#include "array_list.h"
#include <thread>

namespace proj4 {

    ArrayList* MmaClient::Allocate(size_t sz) {

        while (true) {
            ClientContext context;
            AllocateRequest request;
            AllocateReply reply;
            request.set_size(sz);
            Status status = stub_->Allocate(&context, request, &reply);

            if (status.ok()) {
                return new ArrayList(sz, this, reply.array_id());
            } else {
                // wait untill allocation succeed
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }

    void MmaClient::Free(ArrayList* arr) {
        ClientContext context;
        ReleaseRequest request;
        ReleaseReply reply;
        request.set_array_id(arr->array_id);

        Status status = stub_->Release(&context, request, &reply);
        delete arr;
    };

    int MmaClient::ReadPage(int array_id, int virtual_page_id, int offset) {
        ClientContext context;
        ReadPageRequest request;
        ReadPageReply reply;
        request.set_array_id(array_id);
        request.set_virtual_page_id(virtual_page_id);
        request.set_offset(offset);

        Status status = stub_->ReadPage(&context, request, &reply);

        return reply.value();
    }

    void MmaClient::WritePage(int array_id, int virtual_page_id, int offset, int value) {
        ClientContext context;
        WritePageRequest request;
        WritePageReply reply;
        request.set_array_id(array_id);
        request.set_virtual_page_id(virtual_page_id);
        request.set_offset(offset);
        request.set_value(value);

        Status status = stub_->WritePage(&context, request, &reply);
    }

} //namespace proj4
