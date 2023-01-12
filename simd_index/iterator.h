/* SPDX-License-Identifier: GPL-3.0-or-later WITH GCC-exception-3.1 */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef PROTOTYPE_SIMD_INDEX_ITERATOR_H_
#define PROTOTYPE_SIMD_INDEX_ITERATOR_H_

/* Iterator for simd_index to implement a range (optional)
 * =======================================================
 *
 * A random access iterator similar to iota_view<size_t>::iterator.
 */
struct simd_index_iterator
{
  using value_type = std::size_t;
  using difference_type = std::make_signed_t<value_type>;
  using iterator_concept = std::random_access_iterator_tag;
  using reference = const value_type&;
  using iterator_category = std::random_access_iterator_tag;

  std::size_t m_value;

  constexpr simd_index_iterator&
  operator++() noexcept
  { ++m_value; return *this; }

  constexpr simd_index_iterator
  operator++(int) noexcept
  { simd_index_iterator r = *this; ++m_value; return r; }

  constexpr simd_index_iterator&
  operator--() noexcept
  { --m_value; return *this; }

  constexpr simd_index_iterator
  operator--(int) noexcept
  { simd_index_iterator r = *this; --m_value; return r; }

  constexpr value_type
  operator*() const noexcept
  { return m_value; }

  constexpr std::size_t
  operator[](difference_type i) const noexcept
  { return m_value + i; }

  constexpr friend auto operator<=>(simd_index_iterator, simd_index_iterator) noexcept = default;

  constexpr friend simd_index_iterator
  operator+(simd_index_iterator it, difference_type d)
  { return {it.m_value + d}; }

  constexpr friend simd_index_iterator
  operator+(difference_type d, simd_index_iterator it)
  { return {it.m_value + d}; }

  constexpr friend simd_index_iterator
  operator-(simd_index_iterator it, difference_type d)
  { return {it.m_value - d}; }

  constexpr friend difference_type
  operator-(simd_index_iterator l, simd_index_iterator r)
  { return l.m_value - r.m_value; }
};

#endif  // PROTOTYPE_SIMD_INDEX_ITERATOR_H_
