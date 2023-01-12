#include "../prototype_simd_index/stdx.h"

namespace std::experimental
{
  class simd_iterator_sentinel
  {};

  template <typename T, typename A>
    class simd_iterator
    {
      using V = stdx::simd<T, A>;
      const V* data = nullptr;
      int offset = 0;

    public:
      using value_type = T;
      using iterator_category = std::random_access_iterator_tag;
      using difference_type = int;

      constexpr simd_iterator() = default;

      constexpr
      simd_iterator(const V& d, int x)
      : data(&d), offset(x)
      {}

      constexpr
      simd_iterator(const simd_iterator &) = default;

      constexpr simd_iterator&
      operator=(const simd_iterator &) = default;

      constexpr value_type
      operator*() const
      { return (*data)[offset]; }

      constexpr simd_iterator&
      operator++()
      {
        ++offset;
        return *this;
      }

      constexpr simd_iterator
      operator++(int)
      {
        simd_iterator r = *this;
        ++offset;
        return r;
      }

      constexpr simd_iterator&
      operator--()
      {
        --offset;
        return *this;
      }

      constexpr simd_iterator
      operator--(int)
      {
        simd_iterator r = *this;
        --offset;
        return r;
      }

      constexpr difference_type
      operator-(simd_iterator rhs) const
      { return offset - rhs.offset; }

      constexpr friend difference_type
      operator-(simd_iterator it, simd_iterator_sentinel)
      { return it.offset - difference_type(V::size()); }

      constexpr friend difference_type
      operator-(simd_iterator_sentinel, simd_iterator it)
      { return difference_type(V::size()) - it.offset; }

      constexpr friend simd_iterator
      operator+(difference_type x, const simd_iterator& it)
      { return simd_iterator(*it.data, it.offset + x); }

      constexpr friend simd_iterator
      operator+(const simd_iterator& it, difference_type x)
      { return simd_iterator(*it.data, it.offset + x); }

      constexpr friend simd_iterator
      operator-(difference_type x, const simd_iterator& it)
      { return simd_iterator(*it.data, it.offset - x); }

      constexpr friend simd_iterator
      operator-(const simd_iterator& it, difference_type x)
      { return simd_iterator(*it.data, it.offset - x); }

      constexpr simd_iterator&
      operator+=(difference_type x)
      {
        offset += x;
        return *this;
      }

      constexpr simd_iterator&
      operator-=(difference_type x)
      {
        offset -= x;
        return *this;
      }

      constexpr value_type
      operator[](difference_type i) const
      { return (*data)[offset + i]; }

      constexpr friend auto operator<=>(simd_iterator a, simd_iterator b)
      { return a.offset <=> b.offset; }

      constexpr friend bool operator==(simd_iterator a, simd_iterator b) = default;

      constexpr friend bool operator==(simd_iterator a, simd_iterator_sentinel b)
      { return a.offset == difference_type(V::size()); }
    };

  template <typename T, typename A>
    class simd_mask_iterator
    {
      using V = stdx::simd_mask<T, A>;
      const V* data = nullptr;
      int offset = 0;

    public:
      using value_type = bool;
      using iterator_category = std::random_access_iterator_tag;
      using difference_type = int;

      constexpr simd_mask_iterator() = default;

      constexpr
      simd_mask_iterator(const V& d, int x)
      : data(&d), offset(x)
      {}

      constexpr
      simd_mask_iterator(const simd_mask_iterator &) = default;

      constexpr simd_mask_iterator&
      operator=(const simd_mask_iterator &) = default;

      constexpr value_type
      operator*() const
      { return (*data)[offset]; }

      constexpr simd_mask_iterator&
      operator++()
      {
        ++offset;
        return *this;
      }

      constexpr simd_mask_iterator
      operator++(int)
      {
        simd_mask_iterator r = *this;
        ++offset;
        return r;
      }

      constexpr simd_mask_iterator&
      operator--()
      {
        --offset;
        return *this;
      }

      constexpr simd_mask_iterator
      operator--(int)
      {
        simd_mask_iterator r = *this;
        --offset;
        return r;
      }

      constexpr difference_type
      operator-(simd_mask_iterator rhs) const
      { return offset - rhs.offset; }

      constexpr friend simd_mask_iterator
      operator+(difference_type x, const simd_mask_iterator& it)
      { return simd_mask_iterator(*it.data, it.offset + x); }

      constexpr friend simd_mask_iterator
      operator+(const simd_mask_iterator& it, difference_type x)
      { return simd_mask_iterator(*it.data, it.offset + x); }

      constexpr friend simd_mask_iterator
      operator-(difference_type x, const simd_mask_iterator& it)
      { return simd_mask_iterator(*it.data, it.offset - x); }

      constexpr friend simd_mask_iterator
      operator-(const simd_mask_iterator& it, difference_type x)
      { return simd_mask_iterator(*it.data, it.offset - x); }

      constexpr simd_mask_iterator&
      operator+=(difference_type x)
      {
        offset += x;
        return *this;
      }

      constexpr simd_mask_iterator&
      operator-=(difference_type x)
      {
        offset -= x;
        return *this;
      }

      constexpr value_type
      operator[](difference_type i) const
      { return (*data)[offset + i]; }

      constexpr friend auto operator<=>(simd_mask_iterator a, simd_mask_iterator b)
      { return a.offset <=> b.offset; }

      constexpr friend bool operator==(simd_mask_iterator a, simd_mask_iterator b) = default;

      constexpr friend bool operator==(simd_mask_iterator a, simd_iterator_sentinel b)
      { return a.offset == difference_type(V::size()); }
    };

template <typename T, typename A>
  constexpr simd_iterator<T, A>
  begin(const simd<T, A>& v)
  {
    static_assert(std::random_access_iterator<simd_iterator<T, A>>);
    static_assert(std::sentinel_for<simd_iterator_sentinel, simd_iterator<T, A>>);
    return simd_iterator<T, A>(v, 0);
  }

template <typename T, typename A>
  constexpr simd_iterator<T, A>
  begin(simd<T, A>& v)
  {
    static_assert(std::random_access_iterator<simd_iterator<T, A>>);
    static_assert(std::sentinel_for<simd_iterator_sentinel, simd_iterator<T, A>>);
    return simd_iterator<T, A>(v, 0);
  }

template <typename T, typename A>
  constexpr simd_iterator_sentinel
  end(const simd<T, A>& v)
  { return {}; }

template <typename T, typename A>
  constexpr simd_iterator_sentinel
  end(simd<T, A>& v)
  { return {}; }
}
