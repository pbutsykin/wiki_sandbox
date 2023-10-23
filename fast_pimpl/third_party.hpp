#include <vector>
#include <iostream>

namespace thirdparty {
    struct Super_havy {
    public:
        Super_havy() = default;

        explicit Super_havy(size_t num) {
            _data.resize(num);
        }

        Super_havy(Super_havy&& other) = default;
        Super_havy(const Super_havy& other) = default;
        Super_havy& operator=(Super_havy&& other) = default;
        Super_havy& operator=(const Super_havy& other) = default;

        size_t size() const {
            return _data.size();
        }

        void add(int v) {
            _data.push_back(v);
        }

    private:
        size_t min = 0;
        size_t max = 10;
        std::vector<int> _data{1,2,3,4,5};
    };
}