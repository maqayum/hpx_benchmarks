//  Copyright (c)      2012 Daniel Kogler
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*This benchmark measures how long it takes to spawn new threads directly*/
#include "statstd.hpp"
#include <hpx/include/threadmanager.hpp>
#include <hpx/util/lightweight_test.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>

//just an empty function to assign to the thread
void void_thread(){
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//this runs a series of tests for a packaged_action.apply()
void run_tests(uint64_t);

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//still required to run hpx
int hpx_main(variables_map& vm){
    uint64_t num = vm["number-spawned"].as<uint64_t>();
    csv = (vm.count("csv") ? true : false);
    run_tests(num);
    return hpx::finalize();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]){
    // Configure application-specific options.
    options_description
        desc_commandline("usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()
        ("number-spawned,N",
            boost::program_options::value<uint64_t>()
                ->default_value(50000),
            "number of created and joined")
        ("csv",
            "output results as csv "
            "(format:count,mean,accurate mean,variance,min,max)");

    // Initialize and run HPX
    outname = argv[0];
    return hpx::init(desc_commandline, argc, argv);
}

///////////////////////////////////////////////////////////////////////////////

//measure how long it takes to spawn threads with a simple argumentless function 
void run_tests(uint64_t num){
    uint64_t i = 0;
    double ot = timer_overhead(num);
    double mean1;
    string message = "Measuring time required to spawn threads:";
    vector<double> time;
    vector<hpx::thread> threads;
    threads.reserve(2*num);

    //first measure the average time it takes to spawn threads
    high_resolution_timer t;
    for(; i < num; ++i)
        threads.push_back(hpx::thread(&void_thread));
    mean1 = t.elapsed()/num;

    //now retrieve the statistical sampling of this time
    time.reserve(num);
    for(i = 0; i < num; i++){
        high_resolution_timer t1;
        threads.push_back(hpx::thread(&void_thread));
        time.push_back(t1.elapsed());
    }

    printout(time, ot, mean1, message);

    //ensure all created threads have joined or else we will not be able to safely
    //exit the program
    for(i = 0; i < num; ++i) threads[i].join();
    for(i = num; i < num+num; ++i) threads[i].join();
}

