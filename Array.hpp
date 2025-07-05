/*
======== 总结 ========
my::array 是一个固定大小的顺序容器，功能和 std::array 基本一致。
- 支持元素访问、迭代器、容量查询、批量赋值、swap、比较等常用操作。
- 针对 N=0 做了特化，行为与标准库一致。
- 后续可补充 get、to_array、三向比较、tuple接口等高级特性，以及补充noexcept。
*/

#pragma once
#include <stdexcept>
#include <iterator>
#include <cstdlib>

namespace my
{
    // 固定大小数组容器，类似于std::array
    template <typename T, size_t N>
    struct array
    {
        // 类型定义
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

        // 数据成员，存储元素
        T element[N];

        // 元素访问
        // 安全访问，越界抛异常
        constexpr reference at(size_t index)
        {
            if (index >= size())
                throw std::out_of_range("array::at out of range");
            return element[index];
        }
        constexpr const_reference at(size_t index) const
        {
            if (index >= size())
                throw std::out_of_range("array::at out of range");
            return element[index];
        }
        // 下标访问，不做越界检查
        constexpr reference operator[](size_t index) noexcept
        {
            return element[index];
        }
        constexpr const_reference operator[](size_t index) const noexcept
        {
            return element[index];
        }
        // 访问第一个元素
        constexpr reference front() noexcept
        {
            return element[(size_type)0];
        }
        constexpr const_reference front() const noexcept
        {
            return element[(size_type)0];
        }
        // 访问最后一个元素
        constexpr reference back() noexcept
        {
            return element[N - 1];
        }
        constexpr const_reference back() const noexcept
        {
            return element[N - 1];
        }

        // 数据指针访问
        constexpr T *data() noexcept
        {
            return element;
        }
        constexpr const T *data() const noexcept
        {
            return element;
        }

        // 迭代器相关
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

        // 容量相关
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

        // 批量赋值，将所有元素赋值为u
        constexpr void fill(const T &u)
        {
            for (size_type i = 0; i < N; i++)
                element[i] = u;
        }
    };

    // 针对N=0的特化，空数组
    template <typename T>
    struct array<T, 0>
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

        // 元素访问，全部抛出异常或终止
        constexpr reference at(size_t) { throw std::out_of_range("array<...,0>::at"); }
        constexpr const_reference at(size_t) const { throw std::out_of_range("array<...,0>::at"); }
        constexpr reference operator[](size_t) { std::abort(); }
        constexpr const_reference operator[](size_t) const { std::abort(); }
        constexpr reference front() { std::abort(); }
        constexpr const_reference front() const { std::abort(); }
        constexpr reference back() { std::abort(); }
        constexpr const_reference back() const { std::abort(); }

        // 数据指针与迭代器均为nullptr
        constexpr T *data() noexcept { return nullptr; }
        constexpr const T *data() const noexcept { return nullptr; }
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

        // 容量相关
        constexpr bool empty() const noexcept { return true; }
        constexpr size_type size() const noexcept { return 0; }
        constexpr size_type max_size() const noexcept { return 0; }

        // 空实现
        constexpr void fill(const T &) noexcept {}
    };

    // 交换两个array的内容
    template <typename T, size_t N>
    void swap(array<T, N> &lhs, array<T, N> &rhs)
    {
        for (size_t i = 0; i < N; i++)
        {
            std::swap(lhs[i], rhs[i]);
        }
    }

    // 相等比较
    template <typename T, size_t N>
    constexpr bool operator==(const array<T, N> &lhs, const array<T, N> &rhs)
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

    // 不等比较
    template <typename T, size_t N>
    constexpr bool operator!=(const array<T, N> &lhs, const array<T, N> &rhs)
    {
        return !(lhs == rhs);
    }
}

