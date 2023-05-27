#pragma once
#include "daisy.h"
#include "daisysp.h"

#include <array>

namespace daisysp {

    template<std::size_t Length, typename Generator, std::size_t... Indexes>
    constexpr auto lut_impl(Generator&& f, std::index_sequence<Indexes...>) {
        using content_type = decltype(f(std::size_t{0}));
        return std::array<content_type, Length> {{ f(Indexes)... }};
    }

    template<std::size_t Length, typename Generator>
    constexpr auto lut(Generator&& f){
        return lut_impl<Length>(std::forward<Generator>(f), std::make_index_sequence<Length>{});
    }

    template <size_t n>
    constexpr double cheb(double x) { return 2*x*cheb<n-1>(x) - cheb<n-2>(x); }
    template <>
    constexpr double cheb<0>(double x) { return 1;}
    template <>
    constexpr double cheb<1>(double x) { return x;}

    template <size_t Length, size_t k>
    constexpr float cheby(size_t n) {
        double x = 2.f*(static_cast<double>(n)/static_cast<double>(Length-1)) -1.f;
        return static_cast<double>(cheb<k>(x)); 
    }

    template<std::size_t Length, size_t k>
    constexpr auto chebyshev_lut = lut<Length>(cheby<Length, k>);

    constexpr int N = 1024;
    constexpr auto c1 = chebyshev_lut<N,1>;
    constexpr auto c2 = chebyshev_lut<N,2>;
    constexpr auto c3 = chebyshev_lut<N,3>;
    constexpr auto c4 = chebyshev_lut<N,4>;
    constexpr auto c5 = chebyshev_lut<N,5>;
    constexpr auto c6 = chebyshev_lut<N,6>;
    constexpr auto c7 = chebyshev_lut<N,7>;
    constexpr auto c8 = chebyshev_lut<N,8>;

    template <size_t N>
    class Shaper {
       
        public: 
            void init();
            float process(float in);
            void setWeight(int n, float w);
            const float * getWeights() const;
        private:
        
            std::array<float,8> weights;
            float m_inv_sum;
    };


    template <size_t N>
    void Shaper<N>::init() {
        weights = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};
        m_inv_sum = std::accumulate(weights.begin(), weights.end(), 0.f);
    }

    template <size_t N>
    const float * Shaper<N>::getWeights() const {
            return weights.data(); 
    }

    template <size_t N>
    float Shaper<N>::process(float in) {
       const uint16_t ind = static_cast<float>(N) *  (in + 1.f)/2.f;
       m_inv_sum = std::accumulate(weights.begin(), weights.end(), 0.f);
       float s =  weights[0]*c1[ind]
                + weights[1] * c2[ind]
                + weights[2] * c3[ind]
                + weights[3] * c4[ind]
                + weights[4] * c5[ind]
                + weights[5] * c6[ind]
                + weights[6] * c7[ind]
                + weights[7] * c8[ind];
        return s/m_inv_sum;
    }

    template <size_t N>
    void Shaper<N>::setWeight(int n, float w) {
        weights[n] = w;
    }

} /* namespace daisysp */
