#pragma once

#include <iterator>
#include <numeric>
#include <iostream>
#include <tuple>

#include "LzTools.hpp"


namespace lz { namespace detail {
#ifdef CXX_LT_17
    template<typename Same, typename First, typename... More>
    struct IsAllSame {
        static const bool value = std::is_same<Same, First>::value && IsAllSame<First, More...>::value;
    };

    template<typename Same, typename First>
    struct IsAllSame<Same, First> : std::is_same<Same, First> {
    };
#endif

    template<class Tuple, size_t I, class = void>
    struct PlusPlus {
        void operator()(Tuple& iterators, const Tuple& end) const {
            if (std::get<I>(iterators) != std::get<I>(end)) {
                ++std::get<I>(iterators);
            }
            else {
                PlusPlus<Tuple, I + 1>()(iterators, end);
            }
        }
    };

    template<class Tuple, size_t I>
    struct PlusPlus<Tuple, I, typename std::enable_if<I == std::tuple_size<std::decay_t<Tuple>>::value>::type> {
        void operator()(const Tuple& /*iterators*/, const Tuple& /*end*/) const {
        }
    };

    template<class Tuple, size_t I, class = void>
    struct NotEqual {
        bool operator()(const Tuple& iterators, const Tuple& end) const {
            if (std::get<I>(iterators) != std::get<I>(end)) {
                return std::get<I>(iterators) != std::get<I>(end);
            }
            else {
                return NotEqual<Tuple, I + 1>()(iterators, end);
            }
        }
    };

    template<class Tuple, size_t I>
    struct NotEqual<Tuple, I, typename std::enable_if<I == std::tuple_size<std::decay_t<Tuple>>::value - 1>::type> {
        bool operator()(const Tuple& iterators, const Tuple& end) const {
            return std::get<I>(iterators) != std::get<I>(end);
        }
    };

    template<class Tuple, size_t I, class = void>
    struct Deref {
        auto operator()(const Tuple& iterators, const Tuple& end) const -> decltype(*std::get<I>(iterators)) {
            if (std::get<I>(iterators) != std::get<I>(end)) {
                return *std::get<I>(iterators);
            }
            else {
                return Deref<Tuple, I + 1>()(iterators, end);
            }
        }
    };

    template<class Tuple, size_t I>
    struct Deref<Tuple, I, typename std::enable_if<I == std::tuple_size<std::decay_t<Tuple>>::value - 1>::type> {
        auto operator()(const Tuple& iterators, const Tuple&) const -> decltype(*std::get<I>(iterators)) {
            return *std::get<I>(iterators);
        }
    };

    template<class Tuple, size_t I>
    struct MinusMinus {
        void operator()(Tuple& iterators, Tuple& end) const {
            if (std::get<I>(iterators) == std::get<I>(end)) {
                --std::get<I>(iterators);
            }
            else {
                MinusMinus<Tuple, I - 1>()(iterators, end);
            }
        }
    };

    template<class Tuple>
    struct MinusMinus<Tuple, 0> {
        void operator()(Tuple& iterators, Tuple&) const {
            --std::get<0>(iterators);
        }
    };

    template<class Tuple, size_t I>
    struct MinIs {
        template<class DifferenceType>
        void operator()(Tuple& iterators, const Tuple& begin, const Tuple& end, const DifferenceType offset) const {
            auto current = std::get<I>(iterators);
            auto currentBegin = std::get<I>(begin);

            // Current is begin, move on to next iterator
            if (current == currentBegin) {
                MinIs<Tuple, I - 1>()(iterators, begin, end, offset);
            }
            else {
                auto distance = static_cast<DifferenceType>(std::distance(std::get<I>(begin), std::get<I>(end)));
                if (distance <= offset) {
                    std::get<I>(iterators) = std::get<I>(begin);
                    MinIs<Tuple, I - 1>()(iterators, begin, end, distance == 0 ? 1 : offset - distance);
                }
                else {
                    std::get<I>(iterators) = std::prev(std::get<I>(iterators), offset);
                }
            }
        }
    };

    template<class Tuple>
    struct MinIs<Tuple, 0> {
        template<class DifferenceType>
        void operator()(Tuple& iterators, const Tuple& begin, const Tuple& end, const DifferenceType offset) const  {
            auto& current = std::get<0>(iterators);
            auto currentBegin = std::get<0>(begin);
            auto distance = static_cast<DifferenceType>(std::distance(current, std::get<0>(end)));

            // first iterator is at position begin, and distance bigger than 0
            if (current == currentBegin && distance > 0) {
                throw std::out_of_range("cannot access elements before begin");
            }
            else {
                current = std::prev(current, offset);
            }
        }
    };

    template<class Tuple, size_t I, class = void>
    struct PlusIs {
        template<class DifferenceType>
        void operator()(Tuple& iterators, const Tuple& end, const DifferenceType offset) const {
            auto& currentIterator = std::get<I>(iterators);
            auto currentEnd = std::get<I>(end);

            auto distance = static_cast<DifferenceType>(std::distance(currentIterator, currentEnd));

            if (distance > offset) {
                currentIterator = std::next(currentIterator, offset);
            }
            else {
                currentIterator = currentEnd;
                PlusIs<Tuple, I + 1>()(iterators, end, offset - distance);
            }
        }
    };

    template<class Tuple, size_t I>
    struct PlusIs<Tuple, I, typename std::enable_if<I == std::tuple_size<std::decay_t<Tuple>>::value>::type> {
        template<class DifferenceType>
        void operator()(Tuple& /*iterators*/, const Tuple& /*end*/, const DifferenceType /*offset*/) const {
        }
    };

    // Begin ConcatIterator
    template<class... Iterators>
    class ConcatenateIterator {
        using IterTuple = std::tuple<Iterators...>;
        IterTuple _iterators{};
        IterTuple _begin{};
        IterTuple _end{};

        using FirstTupleIterator = std::iterator_traits<std::decay_t<decltype(std::get<0>(std::declval<IterTuple>()))>>;

    public:
        using value_type = typename FirstTupleIterator::value_type;
        using difference_type = std::ptrdiff_t;
        using reference = typename FirstTupleIterator::reference;
        using pointer = typename FirstTupleIterator::pointer;
        using iterator_category = std::random_access_iterator_tag;

        static_assert(sizeof...(Iterators) >= 2, "amount of iterators/containers cannot be less than or equal to 1");

#ifdef CXX_LT_17
        static_assert(IsAllSame<typename std::iterator_traits<Iterators>::value_type...>::value,
                      "value types of iterators do not match");
        static_assert(IsAllSame<typename std::iterator_traits<Iterators>::pointer...>::value, "pointer types of iterators do not match");
        static_assert(IsAllSame<typename std::iterator_traits<Iterators>::reference...>::value, "reference types of iterators do not match");
#else
        static_assert(std::conjunction<std::is_same<value_type,
            typename std::iterator_traits<Iterators>::value_type>...>::value, "value types of iterators do not match");
        static_assert(std::conjunction<std::is_same<pointer,
            typename std::iterator_traits<Iterators>::pointer>...>::value, "pointer types of iterators do not match");
        static_assert(std::conjunction<std::is_same<reference ,
            typename std::iterator_traits<Iterators>::reference>...>::value, "reference types of iterators do not match");
#endif
    private:
        template<size_t... I>
        difference_type minus(std::index_sequence<I...>, const ConcatenateIterator& other) const {
            std::initializer_list<difference_type> totals = {
                static_cast<difference_type>(std::distance(std::get<I>(other._iterators), std::get<I>(_iterators)))...};
            return std::accumulate(totals.begin(), totals.end(), static_cast<difference_type>(0));
        }

    public:
        ConcatenateIterator(const IterTuple& iterators, const IterTuple& begin, const IterTuple& end) :  // NOLINT(modernize-pass-by-value)
            _iterators(iterators),
            _begin(begin),
            _end(end) {
        }

        reference operator*() const {
            return Deref<IterTuple, 0>()(_iterators, _end);
        }

        pointer operator->() const {
            return &*this;
        }

        ConcatenateIterator& operator++() {
            PlusPlus<IterTuple, 0>()(_iterators, _end);
            return *this;
        }

        ConcatenateIterator operator++(int) {
            ConcatenateIterator tmp(*this);
            ++*this;
            return tmp;
        }

        ConcatenateIterator& operator--() {
            MinusMinus<IterTuple, sizeof...(Iterators) - 1>()(_iterators, _end);
            return *this;
        }

        ConcatenateIterator operator--(int) {
            ConcatenateIterator tmp(*this);
            ++*this;
            return tmp;
        }

        ConcatenateIterator& operator+=(const difference_type offset) {
            PlusIs<IterTuple, 0>()(_iterators, _end, offset);
            return *this;
        }

        ConcatenateIterator& operator-=(const difference_type offset) {
            MinIs<IterTuple, sizeof...(Iterators) - 1>()(_iterators, _begin, _end, offset);
            return *this;
        }

        ConcatenateIterator operator+(const difference_type offset) const {
            ConcatenateIterator tmp(*this);
            tmp += offset;
            return tmp;
        }

        ConcatenateIterator operator-(const difference_type offset) const {
            ConcatenateIterator tmp(*this);
            tmp -= offset;
            return tmp;
        }

        difference_type operator-(const ConcatenateIterator& other) const {
            return minus(std::index_sequence_for<Iterators...>{}, other);
        }

        bool operator!=(const ConcatenateIterator& other) const {
            return NotEqual<IterTuple, 0>()(_iterators, other._iterators);
        }

        bool operator==(const ConcatenateIterator& other) const {
            return !(*this != other);
        }

        reference operator[](const difference_type offset) const {
            return *(*this + offset);
        }

        bool operator<(const ConcatenateIterator& other) const {
            return other - *this > 0;
        }

        bool operator>(const ConcatenateIterator& other) const {
            return other < *this;
        }

        bool operator<=(const ConcatenateIterator& other) const {
            return !(other < *this);
        }

        bool operator>=(const ConcatenateIterator& other) const {
            return !(*this < other);
        }
    };
}}