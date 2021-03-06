#pragma once

#include <type_traits>
#include <algorithm>
#include <functional>
#include <vector>
#include <array>

#include "detail/BasicIteratorView.hpp"
#include "detail/FilterIterator.hpp"


namespace lz {
    template<class Iterator, class Function>
    class Filter final : public detail::BasicIteratorView<detail::FilterIterator<Iterator, Function>> {
    public:
        using iterator = detail::FilterIterator<Iterator, Function>;
        using const_iterator = iterator;

        using value_type = typename iterator::value_type;

    private:
        std::function<bool(value_type)> _predicate{};
        Iterator _begin{};
        Iterator _end{};

    public:
        static_assert(std::is_same<detail::FunctionReturnType<Function, value_type>, bool>::value,
            "function must return bool");
        /**
         * @brief The filter constructor.
         * @param begin Beginning of the iterator.
         * @param end End of the iterator.
         * @param function A function with parameter the value type of the iterable and must return a bool.
         */
        Filter(const Iterator begin, const Iterator end, const Function& function) :
            _predicate{function},
            _begin(begin),
            _end(end) {
        }

        /**
        * @brief Returns the beginning of the filter iterator object.
        * @return A forward iterator FilterIterator.
        */
        iterator begin() const override {
            return iterator(_begin, _end, &_predicate);
        }

        /**
        * @brief Returns the ending of the filter iterator object.
        * @return A forward iterator FilterIterator.
        */
        iterator end() const override {
            return iterator(_end, _end, &_predicate);;
        }
    };

    /**
     * @addtogroup ItFns
     * @{
     */

    /**
     * @brief Returns a forward filter iterator. If the `predicate` returns false, it is excluded.
     * @details I.e. `lz::filter({1, 2, 3, 4, 5}, [](int i){ return i % 2 == 0; });` will eventually remove all
     * elements that are not even.
     * @tparam Iterator Is automatically deduced.
     * @tparam Function Is automatically deduced, but must be a function, lambda or functor.
     * @param begin The beginning of the range.
     * @param end The ending of the range.
     * @param predicate A function that must return a bool, and needs a value type of the container as parameter.
     * @return A filter object from [begin, end) that can be converted to an arbitrary container or can be iterated
     * over.
     */
    template<class Iterator, class Function>
    Filter<Iterator, Function> filterrange(const Iterator begin, const Iterator end, const Function& predicate) {
        return Filter<Iterator, Function>(begin, end, predicate);
    }

    /**
     * @brief Returns a forward filter iterator. If the `predicate` returns false, the value it is excluded.
     * @details I.e. `lz::filter({1, 2, 3, 4, 5}, [](int i){ return i % 2 == 0; });` will eventually remove all
     * elements that are not even.
     * @tparam Iterable Is automatically deduced.
     * @tparam Function Is automatically deduced, but must be a function, lambda or functor.
     * @param iterable An iterable, e.g. a container / object with `begin()` and `end()` methods.
     * @param predicate A function that must return a bool, and needs a value type of the container as parameter.
     * @return A filter iterator that can be converted to an arbitrary container or can be iterated
     * over using `for (auto... lz::filter(...))`.
     */
    template<class Iterable, class Function>
    auto filter(Iterable&& iterable, const Function& predicate) -> Filter<decltype(std::begin(iterable)), Function> {
        return filterrange(std::begin(iterable), std::end(iterable), predicate);
    }

    // End of group
    /**
     * @}
     */
}