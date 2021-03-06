# Copyright (c) 2011 Bryce Adelstein-Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(benchmarks
#    boost_tls_overhead
#    native_tls_overhead
#    hpx_tls_overhead
#    delay_baseline
    hpx_homogeneous_task_spawn
    hpx_heterogeneous_task_spawn
#    print_heterogeneous_payloads
    sizeof
    thread_cs_analysis
   )

if(MSVC)
  set(boost_library_dependencies)
else()
  set(boost_library_dependencies ${BOOST_PROGRAM_OPTIONS_LIBRARY})
endif()

#if(OPENMP_FOUND)
#  set(benchmarks
#      ${benchmarks}
#      openmp_homogeneous_task_spawn
#     )

#  set(openmp_homogeneous_task_spawn_FLAGS NOLIBS
#      DEPENDENCIES ${boost_library_dependencies})
#endif()

################################################################################
# find dependencies
################################################################################
find_package(HPX_Qthreads)
find_package(HPX_TBB)
find_package(HPX_SWARM)
find_package(HPX_OpenMP)


#if(UNIX)
  # GCC 4.7 chokes on Qthreads headers.
#  if(QTHREADS_FOUND AND (040700 GREATER ${GCC_VERSION}))
#    include_directories(${QTHREADS_INCLUDE_DIR})

#    set(benchmarks
#        ${benchmarks}
#        qthreads_homogeneous_task_spawn
#        qthreads_heterogeneous_task_spawn
#       )

#    set(qthreads_homogeneous_task_spawn_FLAGS NOLIBS
#        DEPENDENCIES ${boost_library_dependencies}
#                     ${QTHREADS_LIBRARY})

#    set(qthreads_heterogeneous_task_spawn_FLAGS NOLIBS
#        DEPENDENCIES ${boost_library_dependencies}
#                     ${QTHREADS_LIBRARY})
#  endif()
#endif()

#if(TBB_FOUND)
#  include_directories(${TBB_INCLUDE_DIR})

#  set(benchmarks
#      ${benchmarks}
#      tbb_homogeneous_task_spawn
#     )

#  set(tbb_homogeneous_task_spawn_FLAGS NOLIBS
#      DEPENDENCIES ${boost_library_dependencies}
#                   ${TBB_LIBRARY})
#endif()

#if(SWARM_FOUND)
#  include_directories(${SWARM_INCLUDE_DIR})

#  set(benchmarks
#      ${benchmarks}
#      swarm_homogeneous_task_spawn
#     )

#  set(swarm_homogeneous_task_spawn_FLAGS NOLIBS
#      DEPENDENCIES ${boost_library_dependencies}
#                   ${SWARM_LIBRARY})
#endif()

set(delay_baseline_FLAGS NOLIBS
    DEPENDENCIES ${boost_library_dependencies})

set(print_heterogeneous_payloads_FLAGS NOLIBS
    DEPENDENCIES ${boost_library_dependencies})

if(NOT MSVC)
  set(boost_library_dependencies ${boost_library_dependencies} ${BOOST_THREAD_LIBRARY})
endif()

set(boost_tls_overhead_FLAGS NOLIBS
    DEPENDENCIES ${boost_library_dependencies})

set(native_tls_overhead_FLAGS NOLIBS
    DEPENDENCIES ${boost_library_dependencies})

set(hpx_tls_overhead_FLAGS NOLIBS
    DEPENDENCIES ${boost_library_dependencies})

set(hpx_homogeneous_task_spawn_FLAGS DEPENDENCIES iostreams)

set(hpx_heterogeneous_task_spawn_FLAGS DEPENDENCIES iostreams)

#if(HPX_CXX11_LAMBDAS)
#  set(benchmarks ${benchmarks}
#      packaged_task_overhead
#      spinlock_overhead1
#      spinlock_overhead2
#     )

#  set(packaged_task_overhead_FLAGS DEPENDENCIES iostreams)

#  set(spinlock_overhead1_FLAGS DEPENDENCIES iostreams)

#  set(spinlock_overhead2_FLAGS DEPENDENCIES iostreams)
#endif()

set(sizeof_FLAGS DEPENDENCIES iostreams)
set(thread_cs_analysis_FLAGS DEPENDENCIES iostreams)

foreach(benchmark ${benchmarks})
  set(sources
      ${benchmark}.cpp)

  source_group("Source Files" FILES ${sources})

  # add example executable
  add_hpx_executable(${benchmark}
                     SOURCES ${sources}
                     ${${benchmark}_FLAGS}
                     FOLDER "Benchmarks/${benchmark}")

  # add a custom target for this example
  add_hpx_pseudo_target(benchmarks.performance.${benchmark})

  # make pseudo-targets depend on master pseudo-target
  add_hpx_pseudo_dependencies(benchmarks.performance
                              benchmarks.performance.${benchmark})

  # add dependencies to pseudo-target
  add_hpx_pseudo_dependencies(benchmarks.performance.${benchmark}
                              ${benchmark}_exe)
endforeach()

#if(OPENMP_FOUND)
#  set_target_properties(openmp_homogeneous_task_spawn_exe PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
#  set_target_properties(openmp_homogeneous_task_spawn_exe PROPERTIES LINK_FLAGS ${OpenMP_CXX_FLAGS})
#endif()

add_hpx_executable(util_function
                   SOURCES util/function.cpp
                   ${util_function_FLAGS}
                   FOLDER "Benchmarks/util_function")

add_hpx_pseudo_target(benchmarks.performance.util.function)

add_hpx_pseudo_dependencies(benchmarks.performance
                            benchmarks.performance.util.function)

add_hpx_pseudo_dependencies(benchmarks.performance.util.function
                            util.function_exe)

