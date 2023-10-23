#pragma once

#include <type_traits>

template <class T, size_t Size, size_t Alignment>
class FPimpl final {
public:
    template <class... Args>
    explicit FPimpl(Args&&... args) noexcept(std::is_nothrow_constructible<T>::value) {
        new(Ptr()) T(std::forward<Args>(args)...);
    }

    FPimpl(FPimpl&& other) noexcept(std::is_nothrow_constructible<T>::value) {
        new(Ptr()) T(std::move(*other.Ptr()));
    }

    FPimpl(const FPimpl& other) noexcept(std::is_nothrow_constructible<T>::value) {
        new(Ptr()) T(*other.Ptr());
    }

    FPimpl& operator=(FPimpl&& other) noexcept {
        if (this != &other)
            *Ptr() = std::move(*other);

        return *this;
    }

    FPimpl& operator=(const FPimpl& other) noexcept {
        if (this != &other)
            *Ptr() = *other.Ptr();

        return *this;
    }

    ~FPimpl() {
        validate<sizeof(T), alignof(T)>();
        Ptr()->~T();
    }

    T* operator->() noexcept {
        return Ptr();
    }

    const T* operator->() const noexcept {
        return Ptr();
    }

    T& operator*() noexcept {
        return *Ptr();
    }

    T* Ptr() noexcept {
        return reinterpret_cast<T*>(&_data);
    }

    const T* Ptr() const noexcept {
        return reinterpret_cast<const T*>(&_data);
    }

private:
    template<size_t ActualSize, size_t ActualAlignment>
    static void validate() noexcept {
        static_assert(Size == ActualSize, "Size and sizeof(T) mismatch");
        static_assert(Alignment == ActualAlignment, "Alignment and alignof(T) mismatch");
    }

    typename std::aligned_storage<Size, Alignment>::type _data;
};
