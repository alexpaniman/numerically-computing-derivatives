#include <cstddef>
#include <utility>

#include "cppgfplots.h"


template <size_t order> struct numerical_differentiator {
    static_assert(false, "There's no such level of differentiation defined!");
};

template <> struct numerical_differentiator<1> {
    double operator()(auto &&f, double x, double h) {
        return            (f(x +   h) - f(x -   h)) / (2*h);
    }
};

template <> struct numerical_differentiator<2> {
    double operator()(auto &&f, double x, double h) {
        return (4./ 3.) * (f(x +   h) - f(x -   h)) / (2*h) -
               (1./ 3.) * (f(x + 2*h) - f(x - 2*h)) / (4*h);
    }
};

template <> struct numerical_differentiator<3> {
    double operator()(auto &&f, double x, double h) {
        return (3./ 2.) * (f(x +   h) - f(x -   h)) / (2*h) -
               (3./ 5.) * (f(x + 2*h) - f(x - 2*h)) / (4*h) +
               (1./10.) * (f(x + 3*h) - f(x - 3*h)) / (6*h);
    }
};


template <typename T, T Begin,  class Func, T ...Is>
constexpr void static_for_impl(Func &&f, std::integer_sequence<T, Is...>) {
    (f(std::integral_constant<T, Begin + Is>{}), ...);
}

template <typename type, type begin, type end, class func_type>
constexpr void static_for(func_type &&f) {
    static_for_impl<type, begin>(std::forward<func_type>(f), std::make_integer_sequence<type, end - begin>{});
}


#define lambda(...) [](double x) { return __VA_ARGS__; }
#define analytical(func, diff, name) std::make_tuple(lambda(func), lambda(diff), std::string{name})

int main() {
    double sample_point = 42;

    auto functions = std::make_tuple(
        // -- function itself --            -- analytical derivative --              -- latex code for function --
        analytical(sin(x*x)        , 2.*x*cos(x*x)                           , "\\sin{x^2}"                          ),
        analytical(cos(sin(x))     , -sin(sin(x)) * cos(x)                   , "\\cos \\left ( \\sin x \\right )"    ),
        analytical(exp(sin(cos(x))), -sin(x) * cos(cos(x)) * exp(sin(cos(x))), "e^{\\cos \\left ( \\sin x \\right )}"),
        analytical(log(x + 3)      , 1. / (x + 3.)                           , "\\log \\left ( x + 3 \\right )"      ),
        analytical(sqrt(x + 3)     , 0.5*pow(x + 3, -0.5)                    , "\\sqrt{x + 3}"                       )
    );

    auto methods = std::make_tuple(
        [](auto &&f, double x, double h) { return (f(x + h) - f(x)) / h; },
        [](auto &&f, double x, double h) { return (f(x) - f(x - h)) / h; },
        numerical_differentiator<1>{},
        numerical_differentiator<2>{},
        numerical_differentiator<3>{}
    );

    [&]<size_t ...function_index>(std::index_sequence<function_index...>) {
        ([&]() {
            auto &&[current_function, analytical_differentiate, function_latex] = std::get<function_index>(functions);
            double analytical_derivative = analytical_differentiate(sample_point);

            plotting_plane plane("Numerical derivative approximation error for $" + function_latex + "$");

            [&]<size_t ...method_index>(std::index_sequence<method_index...>) {
                ([&]() {
                    std::vector<vec2> points;

                    for (int h = 1; h <= 21; ++ h) {
                        double step = 2. / (2 << h);

                        double diff = std::get<method_index>(methods)(current_function, sample_point, step);
                        double delta = fabs(diff - analytical_derivative);

                        points.emplace_back(step, delta);
                        std::cout << delta << " (diff = " << diff << ", value = " << analytical_derivative << ")\n";
                    }

                    std::cout << "\n";

                    plot current_plot { points };
                    current_plot.name = "Method " + std::to_string(method_index);

                    plane + current_plot;
                }(), ...);
            }(std::make_index_sequence<std::tuple_size_v<decltype(methods)>>());

            auto &[x_axis, y_axis] = plane.axes;
            x_axis.mode = y_axis.mode = LOG;

            y_axis.label = R"($ \left | f'(x_0) - f_\text{numerical}'(x_0) \right | $)";
            x_axis.label = R"($h$)";

            // Choose nicer colors to display on github:
            plane.bg = "0.11,0.11,0.13";
            plane.fg = "0.79,0.80,0.82";
            plane.generate_image("figures/graph-" + std::to_string(function_index) + ".png");
        }(), ...);
    }(std::make_index_sequence<std::tuple_size_v<decltype(functions)>>());

}

