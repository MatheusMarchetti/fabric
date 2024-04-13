#pragma once

#include "defines.hpp"

namespace fabric::ftl {
    class FB_API stopwatch {
       public:
        stopwatch();

        f64 mark();

       private:
        f64 start_time;
        f64 elapsed;
    };
}  // namespace fabric::ftl