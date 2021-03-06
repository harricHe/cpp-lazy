#pragma once



#include "detail/BasicIteratorView.hpp"
#include "detail/GenerateIterator.hpp"


namespace lz {
    template<class GeneratorFunc>
    class Generate final : public detail::BasicIteratorView<detail::GenerateIterator<GeneratorFunc>> {
    public:
        using iterator = detail::GenerateIterator<GeneratorFunc>;
        using const_iterator = iterator;
        using value_type = typename std::iterator_traits<iterator>::value_type;

    private:
        size_t _amount{};
        detail::GenerateIteratorHelper<GeneratorFunc, value_type> _helper;

    public:
        /**
         * @brief Generator constructor.
         * @details Creates a generator object. Executes `func` `amount` of times, and returns the value of this
         * function.
         * @param func The function to execute `amount` of times.
         * @param amount The amount of times to execute. If `amount` is equal to `std::numeric_limits<size_t>::max()`
         * it is interpreted as a `while-true` loop.
         */
        Generate(const GeneratorFunc& func, const size_t amount):
            _amount(amount),
            _helper{func, amount == std::numeric_limits<size_t>::max()}
        {
        }

        /**
        * @brief Returns the beginning of the map iterator object.
        * @return A bidirectional iterator MapIterator.
        */
        iterator begin() const override {
            return iterator(0, &_helper);
        }

        /**
        * @brief Returns the ending of the map iterator object.
        * @return A bidirectional iterator MapIterator.
        */
        iterator end() const override {
            return iterator(_amount, &_helper);
        }
    };

    /**
     * @addtogroup ItFns
     * @{
     */

    /**
     * @brief Returns a view to a generate iterator.
     * @details This object can be used to generate `amount` of values, generated by the `generatorFunc` function.
     * Example:
     * ```cpp
     * int a = 0;
     * size_t amount = 4;
     * auto vector = lz::generate([&a]() { return a++; }, amount).toVector();
     * // vector yields: { 0, 1, 2, 3 }
     * @tparam GeneratorFunc Is automatically deduced.
     * @param generatorFunc The function to execute `amount` of times. The return value of the function is the type
     * that is generated.
     * @param amount The amount of times to execute `generatorFunc`.
     * @return A generator random access iterator view object.
     */
    template<class GeneratorFunc>
    Generate<GeneratorFunc> generate(const GeneratorFunc& generatorFunc, const size_t amount = std::numeric_limits<size_t>::max()) {
        return Generate<GeneratorFunc>(generatorFunc, amount);
    }


    // End of group
    /**
     * @}
     */
}