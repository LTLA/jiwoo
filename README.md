# Automatic memory management of containers of array pointers

![Unit tests](https://github.com/LTLA/jiwoo/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/LTLA/jiwoo/actions/workflows/doxygenate.yaml/badge.svg)
[![Codecov](https://codecov.io/gh/LTLA/jiwoo/branch/master/graph/badge.svg?token=b6PCRpDFzn)](https://codecov.io/gh/LTLA/jiwoo)

## Background

Despite its generalist-sounding title, **jiwoo** is actually just designed for a very specific use case -
managing a thread-specific allocation to store partial results in a parallelized function that accepts a vector of array pointers from the caller.
It's a bit too complicated to actually demonstrate, so check out libraries like [**tatami_stats**](https://github.com/tatami-inc/tatami_stats)
or [**scran_aggregate**](https://github.com/libscran/scran_aggregate) for examples of actual usage.

## Quick start

The `Scope` class will automatically free any non-`NULL` array pointers inside its bound object when it is itself destroyed: 

```cpp
{
    std::vector<double*> ptrs;
    jiwoo::Scope scope(ptrs); // bind Scope to 'ptrs'

    ptrs.push_back(new double[123]);
    ptrs.push_back(new double[32]);
    ptrs.push_back(NULL); // ignored

    // Scope will find all allocated array pointers and delete[] them.
}
```

This works for any pointers that are stored in any nested combinations of `std::vector` and `std::optional`:

```cpp
{
    std::optional<std::vector<std::optional<std::vector<double*> > > ptrs;
    jiwoo::Scope scope(ptrs);

    ptrs.emplace(2);
    ptrs->back().emplace(5);
    ptrs->back()->front() = new double [59];

    // Again, Scope will find the pointer and delete[] it. 
}
```

Sometimes it is necessary to transfer pointers between containers bound to different `Scope` instances:

```cpp
{
    std::vector<std::optional<std::vector<double*> > > outer_ptrs(1);
    jiwoo::Scope outer_scope(outer_ptrs);

    {
        std::optional<std::vector<double*> > inner_ptrs;
        jiwoo::Scope inner_scope(inner_ptrs);

        inner_ptrs.emplace(2);
        inner_ptrs->front() = new double [59];

        // Newly allocated pointer will not be deleted here...
        jiwoo::transfer(inner_ptrs, outer_ptrs[0]);
    }

    // But instead be transfered into outer_ptrs, where it will
    // eventually be deleted here.
}
```

Check out the [reference documentation](https://ltla.github.io/jiwoo) for more details.


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
