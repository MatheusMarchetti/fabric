#pragma once

#include "defines.hpp"

namespace fabric::ftl {
    class FBAPI stopwatch {
       public:
        stopwatch();

        f64 mark();

       private:
        f64 start_time;
        f64 elapsed;
    };
}  // namespace fabric::ftl