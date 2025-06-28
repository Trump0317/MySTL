/*
    自实现array，已完成主要工作
    剩余部分：to_array函数，get函数，比较运算符，三向比较运算符，tuple接口
    以及一些其他的辅助函数，以及编译器优化部分
*/
#pragma once
#include <stdexcept>
#include <iterator>
#include <cstdlib>

template <typename T, size_t N>
struct Array
{
    using value_type = T;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // implement
    T element[N];

    // element access
    constexpr reference at(size_t index)
    {
        if (index >= size())
            throw std::out_of_range("Array::at out of range");
        return element[index];
    }
    constexpr const_reference at(size_t index) const
    {
        if (index >= size())
            throw std::out_of_range("Array::at out of range");
        return element[index];
    }
    constexpr reference operator[](size_t index) noexcept
    {
        return element[index];
    }
    constexpr const_reference operator[](size_t index) const noexcept
    {
        return element[index];
    }
    constexpr reference front() noexcept
    {
        return element[(size_type)0];
    }
    constexpr const_reference front() const noexcept
    {
        return element[(size_type)0];
    }
    constexpr reference back() noexcept
    {
        return element[N - 1];
    }
    constexpr const_reference back() const noexcept
    {
        return element[N - 1];
    }

    constexpr T *data() noexcept
    {
        return element;
    }
    constexpr const T *data() const noexcept
    {
        return element;
    }
    // iterator
    constexpr iterator begin() noexcept
    {
        return static_cast<iterator>(data());
    }
    constexpr const_iterator begin() const noexcept
    {
        return static_cast<const_iterator>(data());
    }
    constexpr iterator end() noexcept
    {
        return static_cast<iterator>(data() + N);
    }
    constexpr const_iterator end() const noexcept
    {
        return static_cast<const_iterator>(data() + N);
    }

    constexpr reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }
    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    constexpr reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }
    constexpr const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return static_cast<const_iterator>(data());
    }
    constexpr const_iterator cend() const noexcept
    {
        return static_cast<const_iterator>(data() + N);
    }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    // capacity
    constexpr bool empty() const noexcept
    {
        return size() == 0;
    }
    constexpr size_type size() const noexcept
    {
        return N;
    }
    constexpr size_type max_size() const noexcept
    {
        return N;
    }
    // operator
    constexpr void fill(const T &u)
    {
        for (size_type i = 0; i < N; i++)
            element[i] = u;
    }
};

// specialization for empty array
template <typename T>
struct Array<T, 0>
{
    using value_type = T;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // element access
    constexpr reference at(size_t) { throw std::out_of_range("Array<...,0>::at"); }
    constexpr const_reference at(size_t) const { throw std::out_of_range("Array<...,0>::at"); }
    constexpr reference operator[](size_t) { std::abort(); }
    constexpr const_reference operator[](size_t) const { std::abort(); }
    constexpr reference front() { std::abort(); }
    constexpr const_reference front() const { std::abort(); }
    constexpr reference back() { std::abort(); }
    constexpr const_reference back() const { std::abort(); }

    constexpr T *data() noexcept { return nullptr; }
    constexpr const T *data() const noexcept { return nullptr; }
    // iterator
    constexpr iterator begin() noexcept { return nullptr; }
    constexpr const_iterator begin() const noexcept { return nullptr; }
    constexpr iterator end() noexcept { return nullptr; }
    constexpr const_iterator end() const noexcept { return nullptr; }

    constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    constexpr const_iterator cbegin() const noexcept { return nullptr; }
    constexpr const_iterator cend() const noexcept { return nullptr; }
    constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
    // capacity
    constexpr bool empty() const noexcept { return true; }
    constexpr size_type size() const noexcept { return 0; }
    constexpr size_type max_size() const noexcept { return 0; }
    // operator
    constexpr void fill(const T &) noexcept {}
};

template <typename T, size_t N>
void swap(Array<T, N> &lhs, Array<T, N> &rhs)
{
    for (size_t i = 0; i < N; i++)
    {
        std::swap(lhs[i], rhs[i]);
    }
}

template <typename T, size_t N>
constexpr bool operator==(const Array<T, N> &lhs, const Array<T, N> &rhs)
{
    if (lhs.size() != rhs.size())
        return false;
    for (size_t i = 0; i < lhs.size(); ++i)
    {
        if (lhs[i] != rhs[i])
            return false;
    }
    return true;
}

template <typename T, size_t N>
constexpr bool operator!=(const Array<T, N> &lhs, const Array<T, N> &rhs)
{
    return !(lhs == rhs);
}