# Automatic memory management of array pointers

![Unit tests](https://github.com/LTLA/jiwoo/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/LTLA/jiwoo/actions/workflows/doxygenate.yaml/badge.svg)
[![Codecov](https://codecov.io/gh/LTLA/jiwoo/branch/master/graph/badge.svg?token=b6PCRpDFzn)](https://codecov.io/gh/LTLA/jiwoo)

## Background

Despite its generalist-sounding title, **jiwoo** is actually just designed for a very specific use case -
managing a thread-specific allocation to store partial results in a parallelized function that accepts a vector of array pointers from the caller.
This is more involved than one might expect when we try to squeeze out some more performance by avoiding unnecessary allocations.

## Quick start

The `EquilengthArrays` class is an array of equilength arrays of the specified value type.
When destoyred, all of the arrays are automatically freed.

```cpp
#include "jiwoo/jiwoo.hpp"

// 13 arrays of 20 doubles.
jiwoo::EquilengthArrays<double> arr(13, 20);
double* first = arr[0];
double* last = arr[19];
double* const* ptr = arr.get();
ptr[0][1] = 14;
```

Check out the [reference documentation](https://ltla.github.io/jiwoo) for more details.

## Typical use case

**jiwoo** is primarily intended for use in a function that:

- Accepts a collection of pointers in its function signature, used to store the output.
- Is parallelized in a manner that each thread needs its own copy of the output buffers.
  For example, each thread computes a part of each output statistic, which is combined into the final output value in the subsequent serial section.

A toy example of such a function is shown below.
Some more concrete examples can be found in [`tatami_stats::group_rss()`](https://github.com/tatami-inc/tatami_stats/tree/master/include/tatami_stats/group_rss.hpp)
and [`scran_aggregate::aggregate_across_cells()`](https://github.com/libscran/scran_aggregate/tree/master/include/scran_aggregate/aggregate_across_cells.hpp).

```cpp
void compute_statistic(
    const int num_threads,
    const std::size_t output_length,
    std::vector<double*>& output_buffers // pointers to arrays of 'output_length'
) {
    const std::size_t num_output = output_buffers.size();
    std::optional<std::vector<std::optional<jiwoo::EquilengthArrays<double> > > > partial_output;
    if (num_threads > 1) {
        partial_output.emplace(num_threads - 1);
    }

    // Check out https://github.com/LTLA/subpar for parallelization details.
    subpar::parallelize_simple(
        num_threads,
        [&](const int thread) -> void {
            std::optional<jiwoo::EquilengthArrays<double> > tmp_output;

            double* const* outptrs;
            if (thread == 0) {
                outptrs = output_buffers.data();
            } else {
                tmp_output.emplace(num_output, output_length);
                outptrs = tmp_output->get();
            }

            // Here we compute some kind of statistic within each thread,
            // storing the result in the arrays referenced by entries of 'outptrs'. 

            if (thread > 0) {
                partial_output[thread - 1] = std::move(tmp_output);
            }
        }
    );

    // Combine the partial results from all threads > 0.
    for (int u = 1; u < num_threads; ++u) {
        const auto& partial_used = *((*partial_output)[u]);
        for (std::size_t o = 0; o < num_output; ++o) {
            auto cur_output = output_buffers[o];
            auto cur_partial = partial_used[o];
            for (std::size_t k = 0; k < output_length; ++k) {
                // Add combining logic here, for example:
                cur_output[k] += cur_partial[k];
            }
        }
    }
}
```

The use of **jiwoo** here is motivated by several considerations:

1. The toy function writes directly to `output_buffers` in the first thread.
   This optimization avoids an unnecessary allocation of temporary output buffers, particularly during serial execution.
2. Normally, if we wanted to allocate temporary output buffers, we would create a `std::vector<std::vector<double> >` that automatically manages the memory. 
   However, if we want to easily switch between `output_buffers` and our temporary buffers in different threads,
   our temporary output buffers must also be represented as a contiguous array of `double*` pointers.
   It would be slightly inefficient to allocate both a `std::vector<std::vector<double> >` and also a `std::vector<double*>` with pointers to each vector's data.
3. So, we create a `jiwoo::EquilengthArrays` and extract the pointer to the internal array of pointers with the `get()` method.
   This is directly interchangeable with the equivalent pointer from `output_buffers`. 

## Building projects 

### CMake with `FetchContent`

If you're using CMake, you just need to add something like this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  jiwoo
  GIT_REPOSITORY https://github.com/LTLA/jiwoo
  GIT_TAG master # or any version of interest 
)

FetchContent_MakeAvailable(jiwoo)
```

Then you can link to **jiwoo** to make the headers available during compilation:

```cmake
# For executables:
target_link_libraries(myexe jiwoo)

# For libaries
target_link_libraries(mylib INTERFACE jiwoo)
```

### CMake with `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DJIWOO_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(ltla_jiwoo CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE ltla::jiwoo)
```

### Manual

If you're not using CMake, the simple approach is to just copy the files in the `include/` subdirectory - 
either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.

## Origin of the name

This library is named after the main character of a K-drama that I imagined in the shower.
The title of the K-drama is 잘 익은 자두의 향기, "Scent of a ripe plum".

> **Synopsis:**
> Ji-Woo never believed in love.
> After her parents divorced in her teens, she devoted herself to study and work, steadily climbing the corporate ladder until she became a vice-president of a major company.
> Now approaching her forties, she watches on from the sidelines as her friends and family experiences the highs and loves of their romantic lives.
> However, when a handsome young graduate joins her team, youthful passions begin to stir in her heart.
> Has spingtime finally come for Ji-Woo?

The show has three main arcs.

- The first arc deals with Ji-Woo overcoming the mental scars of her parents' divorce and learning to love again.
  The twist here is that the divorce was merely a ruse;
  Ji-Woo's mother came from old money and her father disapproved of the marriage to Ji-Woo's father.
  When her mother began to succumb to a terminal illness, they pretended to divorce to ensure that Ji-Woo would be supported by her wealthy grandparents.
- The second arc deals with Ji-Woo's ongoing relationship with the graduate.
  Here we introduce some new characters, namely a young female intern who is also attracted to the graduate.
  Now Ji-Woo has to manage a complex web of relationships within her team, managing both her lover and her competition.
- The third and final arc is something to do with marriage, I haven't thought too much about it yet.

Anyway, I don't have the time to flesh this out into a screenplay and also I need to learn Korean.
A reference to this idea in this library is the best we're going to get for now.
