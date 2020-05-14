#include <iostream>
#include <bits/stdc++.h>
#include <chrono>
#include <immintrin.h>

static inline int64_t random64_value(int64_t low, int64_t high = LLONG_MAX)
{
    std::random_device rd;
    std::mt19937_64 eng(rd());
    std::uniform_int_distribution<int64_t> distr(low, high);
    return distr(eng);
}

template<size_t N>
struct bnode {
    union alignas(64) {
        int64_t keys[N];
        __m256i  keys256[N >> 2];
    };

    void init(const int range)
    {
        assert(!((uint64_t)keys256 % 64));

        for (int i = 0; i < N; i++)
            keys[i] = random64_value(i * range, i * range + range);
    }

    using cb_type = int (bnode::*)(const int64_t);

    [[gnu::optimize("O0")]]
    uint64_t test(const uint64_t key, const cb_type search)
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        (this->*search)(key);
        auto t2 = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    }

    //[[gnu::optimize("unroll-loops")]]
    int search_avx(const int64_t key)
    {
        const __m256i m256_key = _mm256_set1_epi64x(key);
        int bit_cnt = 0;

        for (int i = 0; i < (N >> 2); i++) {
            __m256i result = _mm256_cmpgt_epi64(m256_key, keys256[i]);
            int64_t r0 = _mm256_movemask_epi8(result);

            bit_cnt += __builtin_popcount(r0);
        }

        return bit_cnt >> 3;
    }

    int search_binary(const int64_t key)
    {
        int left = 0, right = N;

        while (left < right) {
            const int mid = (right + left) >> 1;

            if (key < keys[mid])
                right = mid;
            else
                left = mid + 1;
        }

         while (left > 0 && key <= keys[left - 1])
            left--;

        return left;
    }

    int search_linner(const int64_t key)
    {
        int i = 0;
        for (i; i < N && key > keys[i]; i++);

        return i;
    }

    int search_binary_stl(const int64_t key)
    {
        //return std::binary_search(&keys[0], &keys[len], key);
        return std::lower_bound(&keys[0], &keys[N], key) - &keys[0];
    }
};

int main()
{
    const int node_num = 64;
    const int node_keys = 64;
    const int range = 1<<20;
    const int m_num = 5;

    using bnode_t = struct bnode<node_keys>;
    using item_t = std::pair<int64_t, bnode_t>;
    std::vector<item_t> nlist;

    for (int i = 0; i < node_num; i++) {
        struct bnode<node_keys> node;
        node.init(range);

        uint64_t key = random64_value(0, range*node_keys);

        nlist.push_back(std::make_pair(key, node));
    }

    auto measure_exec_time = [&](const auto &cb_search) {
        uint64_t duration = 0;
        for (auto &item : nlist) {
            auto key = item.first;
            auto node = item.second;

            duration += node.test(key, cb_search);
        }
        return duration / node_num;
    };

    uint64_t mt_linner = 0, mt_binary = 0, mt_avx2 = 0, mt_binary_stl = 0;
    for(int i = 0; i < m_num; i++) {
        mt_linner += measure_exec_time(&bnode_t::search_linner);
        mt_avx2 += measure_exec_time(&bnode_t::search_avx);
        mt_binary += measure_exec_time(&bnode_t::search_binary);
        mt_binary_stl += measure_exec_time(&bnode_t::search_binary_stl);
    }

    std::cout << "liner:\t\t" << mt_linner / m_num << " ns/search (" << m_num <<"M)\n";
    std::cout << "binary:\t\t" << mt_binary / m_num << " ns/search (" << m_num <<"M)\n";
    std::cout << "avx2:\t\t" << mt_avx2 / m_num << " ns/search (" << m_num <<"M)\n";
    std::cout << "binary_stl:\t" << mt_binary_stl / m_num << " ns/search (" << m_num <<"M)\n";

    return 0;
}
