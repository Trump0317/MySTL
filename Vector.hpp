/*
======== 总结 ========
my::vector 是一个动态顺序容器，功能和 std::vector 基本一致。
- 支持动态扩容、元素访问、迭代器、容量管理、批量赋值、插入、删除、swap、比较等常用操作。
- 支持异常安全，所有批量构造和销毁都采用逆序，防止资源泄漏。
- 支持自定义分配器，兼容标准库 allocator_traits。
- 后续可补充三向比较、emplace系列、SFINAE优化等高级特性，以及补充noexcept。
*/

#pragma once
#include <iterator>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <initializer_list>

namespace my
{
    using std::allocator;
    using std::allocator_traits;
    template <class T, class Allocator = allocator<T>>
    class vector
    {
    public:
        // 类型定义
        using value_type = T;                                                      // 元素类型
        using allocator_type = Allocator;                                          // 分配器类型
        using pointer = typename allocator_traits<Allocator>::pointer;             // 元素指针类型
        using const_pointer = typename allocator_traits<Allocator>::const_pointer; // 常量元素指针类型
        using reference = value_type &;                                            // 元素引用类型
        using const_reference = const value_type &;                                // 常量元素引用类型
        using size_type = std::size_t;                                             // 大小类型
        using difference_type = std::ptrdiff_t;                                    // 差值类型
        using iterator = pointer;                                                  // 迭代器类型
        using const_iterator = const_pointer;                                      // 常量迭代器类型
        using reverse_iterator = std::reverse_iterator<iterator>;                  // 反向迭代器类型
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;      // 常量反向迭代器类型

    protected:
        // 数据成员
        pointer m_first = nullptr;                                     // 指向首元素的指针
        pointer m_last = nullptr;                                      // 指向已用空间尾后位置的指针
        pointer m_end = nullptr;                                       // 指向分配空间尾后位置的指针
        [[no_unique_address]] allocator_type alloc = allocator_type(); // 分配器对象
        using allocator_traits_t = allocator_traits<allocator_type>;   // 分配器 traits

        // 内部释放空间函数
        void m_deallocate() noexcept
        {
            if (m_first)
            {
                allocator_traits_t::deallocate(alloc, m_first, m_end - m_first);
                m_first = m_last = m_end = nullptr;
            }
        }

    public:
        // 默认构造函数
        constexpr vector() noexcept(noexcept(Allocator())) = default;

        // 以指定分配器构造
        constexpr explicit vector(const Allocator &) noexcept
            : alloc(Allocator()) {}

        // 构造 n 个默认值元素
        constexpr explicit vector(size_type n, const Allocator & = Allocator())
        {
            m_first = allocator_traits_t::allocate(alloc, n);
            m_end = m_first + n;
            size_type i = 0;
            try
            {
                for (i = 0; i < n; ++i)
                {
                    allocator_traits_t::construct(alloc, m_first + i);
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                }
                m_deallocate();
                throw;
            }
            m_last = m_end;
        }

        // 构造 n 个 value 拷贝
        constexpr vector(size_type n, const T &value, const Allocator & = Allocator())
        {
            m_first = allocator_traits_t::allocate(alloc, n);
            m_end = m_first + n;
            size_type i = 0;
            try
            {
                for (; i < n; ++i)
                {
                    allocator_traits_t::construct(alloc, m_first + i, value);
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                }
                m_deallocate();
                throw;
            }
            m_last = m_end;
        }

        // 通过迭代器区间构造
        template <class InputIter>
        constexpr vector(InputIter first, InputIter last, const Allocator & = Allocator()) : alloc(Allocator())
        {
            size_type n = std::distance(first, last);
            m_first = allocator_traits_t::allocate(alloc, n);
            m_end = m_first + n;
            size_type i = 0;
            try
            {
                for (; i < n; ++i)
                {
                    allocator_traits_t::construct(alloc, m_first + i, *(first));
                    first++;
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                }
                m_deallocate();
                throw;
            }
            m_last = m_end;
        }

        // 拷贝构造函数
        constexpr vector(const vector &other)
        {
            size_type n = other.size();
            m_first = allocator_traits_t::allocate(alloc, n);
            m_end = m_first + n;
            size_type i = 0;
            try
            {
                for (; i < n; ++i)
                {
                    allocator_traits_t::construct(alloc, m_first + i, other[i]);
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                }
                m_deallocate();
                throw;
            }
            m_last = m_end;
        }

        // 移动构造函数
        constexpr vector(vector &&other) noexcept
        {
            m_first = other.m_first;
            m_last = other.m_last;
            m_end = other.m_end;
            other.m_first = nullptr;
            other.m_last = nullptr;
            other.m_end = nullptr;
        }

        // 带分配器的拷贝构造
        constexpr vector(const vector &other, const type_identity_t<Allocator> &a)
            : alloc(a)
        {
            size_type n = other.size();
            m_first = allocator_traits_t::allocate(alloc, n);
            m_end = m_first + n;
            size_type i = 0;
            try
            {
                for (; i < n; ++i)
                    allocator_traits_t::construct(alloc, m_first + i, other[i]);
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                allocator_traits_t::deallocate(alloc, m_first, n);
                throw;
            }
            m_last = m_end;
        }

        // 带分配器的移动构造
        constexpr vector(vector &&other, const type_identity_t<Allocator> &a)
            : alloc(a)
        {
            size_type n = other.size();
            m_first = allocator_traits_t::allocate(alloc, n);
            m_end = m_first + n;
            size_type i = 0;
            try
            {
                for (; i < n; ++i)
                    allocator_traits_t::construct(alloc, m_first + i, std::move(other[i]));
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                allocator_traits_t::deallocate(alloc, m_first, n);
                throw;
            }
            m_last = m_end;
        }

        // 用初始化列表构造
        constexpr vector(initializer_list<T> il, const Allocator & = Allocator())
            : alloc(Allocator())
        {
            size_type n = il.size();
            m_first = allocator_traits_t::allocate(alloc, n);
            m_end = m_first + n;
            size_type i = 0;
            try
            {
                for (; i < n; ++i)
                {
                    allocator_traits_t::construct(alloc, m_first + i, *(il.begin() + i));
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                }
                m_deallocate();
                throw;
            }
            m_last = m_first + n;
        }

        // 析构函数，销毁所有元素并释放空间
        constexpr ~vector()
        {
            clear();
            m_deallocate();
        }

        // 拷贝赋值
        constexpr vector &operator=(const vector &x)
        {
            if (this != &x)
            {
                clear();
                m_deallocate();
                size_type n = x.size();
                m_first = allocator_traits_t::allocate(alloc, n);
                m_end = m_first + n;
                size_type i = 0;
                try
                {
                    for (; i < n; ++i)
                    {
                        allocator_traits_t::construct(alloc, m_first + i, x[i]);
                    }
                }
                catch (...)
                {
                    for (size_type k = i; k > 0; --k)
                    {
                        allocator_traits_t::destroy(alloc, m_first + k - 1);
                    }
                    m_deallocate();
                    throw;
                }
                m_last = m_end;
            }
            return *this;
        }

        // 移动赋值
        constexpr vector &operator=(vector &&x)
        {
            if (this != &x)
            {
                clear();
                m_deallocate();
                m_first = x.m_first;
                m_last = x.m_last;
                m_end = x.m_end;
                x.m_first = nullptr;
                x.m_last = nullptr;
                x.m_end = nullptr;
            }
            return *this;
        }

        // 用初始化列表赋值
        constexpr vector &operator=(std::initializer_list<T> il)
        {
            clear();
            m_deallocate();
            size_type n = il.size();
            m_first = allocator_traits_t::allocate(alloc, n);
            m_end = m_first + n;
            size_type i = 0;
            try
            {
                for (; i < n; ++i)
                {
                    allocator_traits_t::construct(alloc, m_first + i, *(il.begin() + i));
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                }
                m_deallocate();
                throw;
            }
            m_last = m_first + n;
            return *this;
        }

        // assign区间赋值
        template <class InputIter>
        constexpr void assign(InputIter first, InputIter last)
        {
            clear();
            size_type n = std::distance(first, last);
            reserve(n);
            size_type i = 0;
            try
            {
                for (; i < n; i++)
                {
                    allocator_traits_t::construct(alloc, m_first + i, *first);
                    ++first;
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; k--)
                {
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                    ++first;
                }
                throw;
            }
        }

        // assign n 个 u
        constexpr void assign(size_type n, const T &u)
        {
            clear();
            reserve(n);
            size_type i = 0;
            try
            {
                for (; i < n; i++)
                {
                    allocator_traits_t::construct(alloc, m_first + i, u);
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; k--)
                {
                    allocator_traits_t::destroy(alloc, m_first + k - 1);
                }
                throw;
            }
        }

        // assign 初始化列表
        constexpr void assign(std::initializer_list<value_type> il)
        {
            assign(il.begin(), il.end());
        }

        // 获取分配器
        constexpr allocator_type get_allocator() const noexcept
        {
            return alloc;
        }

        // 迭代器相关
        constexpr iterator begin() noexcept { return m_first; }                                              // 返回首元素迭代器
        constexpr const_iterator begin() const noexcept { return m_first; }                                  // 返回首元素常量迭代器
        constexpr iterator end() noexcept { return m_last; }                                                 // 返回尾后迭代器
        constexpr const_iterator end() const noexcept { return m_last; }                                     // 返回尾后常量迭代器
        constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(m_last); }                    // 反向首迭代器
        constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(m_last); }  // 反向首常量迭代器
        constexpr reverse_iterator rend() noexcept { return reverse_iterator(m_first); }                     // 反向尾迭代器
        constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(m_first); }   // 反向尾常量迭代器
        constexpr const_iterator cbegin() const noexcept { return m_first; }                                 // 常量首元素迭代器
        constexpr const_iterator cend() const noexcept { return m_last; }                                    // 常量尾后迭代器
        constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(m_last); } // 常量反向首迭代器
        constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(m_first); }  // 常量反向尾迭代器

        // 容量相关
        constexpr bool empty() const noexcept { return m_first == m_last; }                               // 判断是否为空
        constexpr size_type size() const noexcept { return static_cast<size_type>(m_last - m_first); }    // 返回元素个数
        constexpr size_type max_size() const noexcept { return allocator_traits_t::max_size(alloc); }     // 返回最大可分配元素数
        constexpr size_type capacity() const noexcept { return static_cast<size_type>(m_end - m_first); } // 返回当前容量

        // 调整大小
        constexpr void resize(size_type n)
        {
            if (n > max_size())
                throw std::length_error("resize: n > max_size");
            if (n == size())
                return;
            size_type sz = size();
            if (n < sz)
            {
                for (size_type i = sz; i != n; i++)
                {
                    allocator_traits_t::destroy(alloc, m_first + i);
                }
                m_last = m_first + n;
            }
            else if (n > sz)
            {
                reserve(n);
                size_type i = 0;
                try
                {
                    for (i = sz; i < n; i++)
                    {
                        allocator_traits_t::construct(alloc, m_first + i, c);
                    }
                }
                catch (...)
                {
                    for (size_type k = i; k > sz; k--)
                    {
                        allocator_traits_t::destroy(alloc, m_first + k - 1);
                    }
                    throw;
                }
                m_last = m_first + n;
            }
        }

        // 调整大小并填充值
        constexpr void resize(size_type n, const T &c)
        {
            if (n > max_size())
                throw std::length_error("resize: n > max_size");
            if (n == size())
                return;
            size_type sz = size();
            if (n < sz)
            {
                for (size_type i = sz; i < n; i++)
                {
                    allocator_traits_t::destroy(alloc, m_first + i);
                }
                m_last = m_first + n;
            }
            else if (n > sz)
            {
                reserve(n);
                size_type i = 0;
                try
                {
                    for (i = sz; i < n; i++)
                    {
                        allocator_traits_t::construct(alloc, m_first + i, c);
                    }
                }
                catch (...)
                {
                    for (size_type k = i; k > sz; k--)
                    {
                        allocator_traits_t::destroy(alloc, m_first + k - 1);
                    }
                    throw;
                }
                m_last = m_first + n;
            }
        }

        // 预留空间
        constexpr void reserve(size_type n)
        {
            if (n > max_size())
                throw std::length_error("reserve: n > max_size");
            if (n <= capacity())
                return;
            n = std::max(n, capacity() * 2);
            if (n == 0)
            {
                m_first = m_last = m_end = nullptr;
                return;
            }
            auto old_first = m_first;
            auto old_last = m_last;
            auto old_end = m_end;
            pointer now_first = allocator_traits_t::allocate(alloc, n);
            size_type old_size = old_last - old_first;
            size_type i = 0;
            try
            {
                for (; i != old_size; ++i)
                {
                    allocator_traits_t::construct(alloc, now_first + i, std::move_if_noexcept(old_first[i]));
                }
            }
            catch (...)
            {
                for (size_type j = i; j > 0; --j)
                {
                    allocator_traits_t::destroy(alloc, now_first + j - 1);
                }
                allocator_traits_t::deallocate(alloc, now_first, n);
                throw;
            }
            for (size_type i = old_size; i > 0; i--)
            {
                allocator_traits_t::destroy(alloc, old_first + i - 1);
            }
            if (old_end - old_first != 0)
            {
                allocator_traits_t::deallocate(alloc, old_first, old_end - old_first);
            }
            m_first = now_first;
            m_end = m_first + n;
            m_last = m_first + old_size;
        }

        // 收缩空间到实际大小
        constexpr void shrink_to_fit()
        {
            auto old_first = m_first;
            auto old_last = m_last;
            auto old_end = m_end;
            size_type n = old_last - old_first;
            if (n == 0)
            {
                m_first = m_end = m_last = nullptr;
                return;
            }
            pointer new_first = allocator_traits_t::allocate(alloc, n);
            size_type i = 0;
            try
            {
                for (; i < n; ++i)
                {
                    allocator_traits_t::construct(alloc, new_first + i, std::move_if_noexcept(*(old_first + i)));
                }
            }
            catch (...)
            {
                // 构造失败时，销毁已构造对象并释放新空间
                for (size_type j = i; j > 0; --j)
                {
                    allocator_traits_t::destroy(alloc, new_first + j - 1);
                }
                allocator_traits_t::deallocate(alloc, new_first, n);
                throw;
            }
            // 构造成功后，销毁旧对象并释放旧空间
            for (size_type i = n; i > 0; --i)
            {
                allocator_traits_t::destroy(alloc, old_first + i - 1);
            }
            if (old_end - old_first != 0)
            {
                allocator_traits_t::deallocate(alloc, old_first, old_end - old_first);
            }
            m_first = new_first;
            m_last = m_end = new_first + n;
        }

        // 元素访问
        constexpr reference operator[](size_type n) { return *(m_first + n); }             // 下标访问
        constexpr const_reference operator[](size_type n) const { return *(m_first + n); } // 下标常量访问
        constexpr const_reference at(size_type n) const
        {
            if (n >= size())
                throw std::out_of_range("Vector: at out of range");
            return *(m_first + n);
        }
        constexpr reference at(size_type n)
        {
            if (n >= size())
                throw std::out_of_range("Vector: at out of range");
            return *(m_first + n);
        }
        constexpr reference front() { return *m_first; }                 // 首元素
        constexpr const_reference front() const { return *m_first; }     // 首元素常量
        constexpr reference back() { return *(m_last - 1); }             // 尾元素
        constexpr const_reference back() const { return *(m_last - 1); } // 尾元素常量

        // 数据指针
        constexpr T *data() noexcept { return m_first; }
        constexpr const T *data() const noexcept { return m_first; }

        // 尾部原地构造元素
        template <class... Args>
        constexpr reference emplace_back(Args &&...args)
        {
            if (size() + 1 >= capacity())
                reserve(size() + 1);
            try
            {
                allocator_traits_t::construct(alloc, m_last, std::forward<Args>(args)...);
            }
            catch (...)
            {
                allocator_traits_t::destroy(alloc, m_last);
                throw;
            }
            m_last++;
            return *(m_last - 1);
        }

        // 尾部插入元素（拷贝）
        constexpr void push_back(const T &x)
        {
            if (size() + 1 >= capacity())
                reserve(size() + 1);
            try
            {
                allocator_traits_t::construct(alloc, m_last, x);
            }
            catch (...)
            {
                allocator_traits_t::destroy(alloc, m_last, x);
                throw;
            }
            m_last++;
        }

        // 尾部插入元素（移动）
        constexpr void push_back(T &&x)
        {
            if (size() + 1 >= capacity())
                reserve(size() + 1);
            try
            {
                allocator_traits_t::construct(alloc, m_last, std::move(x));
            }
            catch (...)
            {
                allocator_traits_t::destroy(alloc, m_last);
                throw;
            }
            m_last++;
        }

        // 删除尾部元素
        constexpr void pop_back()
        {
            m_last--;
            allocator_traits::destroy(alloc, m_last);
        }

        // template <container - compatible - range<T> R>
        // constexpr void append_range(R &&rg);

        template <class... Args>
        constexpr iterator emplace(const_iterator position, Args &&...args)
        {
            size_type j = position - m_first;
            if (size() + 1 >= capacity())
                reserve(size() + 1);
            size_type old_size = size();
            // 后半部分向后移动一位
            size_type i = 0;
            try
            {
                for (i = 0; i < old_size - j; ++i)
                {
                    allocator_traits_t::construct(alloc, m_last - i, std::move(m_first[old_size - 1 - i]));
                }
            }
            catch (...)
            {
                // 构造失败，销毁已构造的后半部分
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last + 1 - k);
                }
                throw;
            }
            // 插入新元素
            try
            {
                allocator_traits_t::construct(alloc, m_first + j, std::forward<Args>(args)...);
            }
            catch (...)
            {
                // 插入失败，销毁后半部分
                for (size_type k = old_size - j; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last + 1 - k);
                }
                throw;
            }
            m_last++;
            return m_first + j;
        }
        constexpr iterator insert(const_iterator position, const T &x)
        {
            size_type j = position - m_first;
            if (size() + 1 >= capacity())
                reserve(size() + 1);
            size_type old_size = size();
            // 后半部分向后移动一位
            size_type i = 0;
            try
            {
                for (i = 0; i < old_size - j; ++i)
                {
                    allocator_traits_t::construct(alloc, m_last - i, m_first[old_size - 1 - i]);
                }
            }
            catch (...)
            {
                // 构造失败，销毁已构造的后半部分
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last + 1 - k);
                }
                throw;
            }
            // 插入新元素
            try
            {
                allocator_traits_t::construct(alloc, m_first + j, x);
            }
            catch (...)
            {
                // 插入失败，销毁后半部分
                for (size_type k = old_size - j; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last + 1 - k);
                }
                throw;
            }
            m_last++;
            return m_first + j;
        }
        constexpr iterator insert(const_iterator position, T &&x)
        {
            size_type j = position - m_first;
            if (size() + 1 >= capacity())
                reserve(size() + 1);
            size_type old_size = size();
            size_type i = 0;
            try
            {
                for (i = 0; i < old_size - j; ++i)
                {
                    allocator_traits_t::construct(alloc, m_last - i, std::move(m_first[old_size - 1 - i]));
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last - k + 1);
                }
                throw;
            }
            try
            {
                allocator_traits_t::construct(alloc, m_first + j, std::move(x));
            }
            catch (...)
            {
                for (size_type k = old_size - j; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last - k + 1);
                }
                throw;
            }
            m_last++;
            return m_first + j;
        }
        constexpr iterator insert(const_iterator position, size_type n, const T &x)
        {
            if (n == 0)
                return m_first + (position - m_first);
            size_type j = position - m_first;
            if (size() + n >= capacity())
                reserve(size() + n);
            size_type old_size = size();
            size_type i = 0;
            try
            {
                for (i = 0; i < old_size - j; ++i)
                {
                    allocator_traits_t::construct(alloc, m_last + n - 1 - i, std::move(m_first[old_size - 1 - i]));
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last + n - k);
                }
                throw;
            }
            i = 0;
            try
            {
                for (size_t i = 0; i < n; i++)
                {
                    allocator_traits_t::construct(alloc, m_first + j + i, x);
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_first + j + k - 1);
                }
                for (size_type k = old_size - j; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last + n - k);
                }
                throw;
            }
            m_last += n;
            return m_first + j;
        }
        template <class InputIter>
        constexpr iterator insert(const_iterator position, InputIter first, InputIter last)
        {
            size_type n = std::distance(first, last);
            if (n == 0)
                return m_first + (position - m_first);
            size_type j = position - m_first;
            if (size() + n >= capacity())
                reserve(size() + n);
            size_type old_size = size();
            size_type i = 0;
            try
            {
                for (i = 0; i < old_size - j; ++i)
                {
                    allocator_traits_t::construct(alloc, m_last + n - 1 - i, std::move(m_first[old_size - 1 - i]));
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last + n - k);
                }
                throw;
            }
            try
            {
                for (i = 0; i < n; ++i, ++first)
                {
                    allocator_traits_t::construct(alloc, m_first + j + i, *first);
                }
            }
            catch (...)
            {
                for (size_type k = i; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_first + j + k - 1);
                }
                for (size_type k = old_size - j; k > 0; --k)
                {
                    allocator_traits_t::destroy(alloc, m_last + n - k);
                }
                throw;
            }
            m_last += n;
            return m_first + j;
        }
        constexpr iterator insert(const_iterator position, initializer_list<T> il)
        {
            return insert(position, il.begin(), il.end());
        }

        // 删除指定位置的元素，返回下一个元素的迭代器
        constexpr iterator erase(const_iterator position)
        {
            if (position < m_first || position >= m_last)
                throw std::out_of_range("Vector: erase out of range");
            size_type j = position - m_first;
            size_type old_size = size();
            if (j >= old_size)
                return m_first + j; // nothing to erase
            for (size_type i = j; i < old_size - 1; ++i)
            {
                allocator_traits_t::construct(alloc, m_first + i, std::move(m_first[i + 1]));
            }
            allocator_traits_t::destroy(alloc, m_last - 1);
            --m_last;
            return m_first + j;
        }

        // 删除区间[first, last)的元素，返回区间后第一个元素的迭代器
        constexpr iterator erase(const_iterator first, const_iterator last)
        {
            if (first < m_first || last > m_last || first >= last)
                throw std::out_of_range("Vector: erase out of range");
            size_type j = first - m_first;
            size_type k = last - m_first;
            size_type old_size = size();
            if (j >= old_size || k <= j)
                return m_first + j; // nothing to erase
            for (size_type i = j; i < old_size - (k - j); ++i)
            {
                allocator_traits_t::construct(alloc, m_first + i, std::move(m_first[i + (k - j)]));
            }
            for (size_type i = old_size - (k - j); i < old_size; ++i)
            {
                allocator_traits_t::destroy(alloc, m_first + i);
            }
            m_last -= (k - j);
            return m_first + j;
        }

        // 交换两个vector的内容
        constexpr void swap(vector &other)
        // noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value || std::allocator_traits<Allocator>::is_always_equal::value)
        {
            using std::swap;
            swap(m_first, other.m_first);
            swap(m_last, other.m_last);
            swap(m_end, other.m_end);
            swap(alloc, other.alloc);
        }

        // 清空vector，销毁所有元素但不释放空间
        constexpr void clear() noexcept
        {
            if (m_first)
            {
                for (pointer it = m_last; it != m_first;)
                {
                    --it;
                    allocator_traits_t::destroy(alloc, it);
                }
                m_last = m_first;
            }
        }
    };

    // 全局swap重载
    template <class T, class Alloc>
    void swap(vector<T, Alloc> &lhs, vector<T, Alloc> &rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

}