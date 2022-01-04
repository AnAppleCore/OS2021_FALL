#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include <cstdlib>

#define PageSize 1024

namespace proj4 {

    class MmaClient;

    class ArrayList{
        private:
        friend class MmaClient;
        size_t size;
        MmaClient* mma_client;
        int array_id;
        ArrayList(size_t, MmaClient*, int);
        ~ArrayList();
        public:
        // you should not modify the public interfaces used in tests
        int Read (unsigned long);
        void Write (unsigned long, int);
    };

} // namespce: proj4
#endif