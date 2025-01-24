#pragma once


/**
 * \n
 * It is a requirement that the first argument passed to the
 * function is an index based on the maximum thread count.
 * \n\n
 * use 'std::ref' for references.
 * @tparam threadCount how many threads to create
 * @tparam Function
 * @tparam Args
 * @param func the function to call
 * @param args the arguments to pass to the function
 * @return
 */
template<int threadCount, typename Function, typename... Args>
int run_parallel(Function func, Args... args);
