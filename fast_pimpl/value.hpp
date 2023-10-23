#include "fpimpl.hpp"

namespace thirdparty {
    struct Super_havy;

    namespace super_havy {
        struct Value {
            using NativeSuperHavy = thirdparty::Super_havy;
            Value();
            Value(Value&& other);
            Value(const Value& other);
            Value& operator=(Value&& other);
            Value& operator=(const Value& other);
            ~Value();

            size_t Size() const;

            private:
                FPimpl<NativeSuperHavy, 40, 8> _data;
        };
    }
}
