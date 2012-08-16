//  Copyright (c)      2012 Daniel Kogler
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "emptycomponent/emptycomponent.hpp"

using namespace hpx;

using components::emptycomponent;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int hpx_main(variables_map& vm){
    uint64_t i;
    uint64_t num = vm["number-created"].as<uint64_t>();
    csv = (vm.count("csv") ? true : false);

    double ot = timer_overhead(num);
    double mean1;
    vector<double> time;
    time.reserve(num);
    string message = "Measuring how long it takes to create minimal components:";

    naming::id_type gid = applier::get_applier().get_runtime_support_gid();
    emptycomponent* comps = new emptycomponent[num];
    
    high_resolution_timer t;
    for(i = 0; i < num; i++)
        comps[i].create(gid);
    mean1 = t.elapsed()/num;

    for(i = 0; i < num; i++)
        comps[i].free();

    free(comps);
    comps = new emptycomponent[num];
    for(i = 0; i < num; i++){
        high_resolution_timer t1;
        comps[i].create(gid);
        time.push_back(t1.elapsed());
    }
    printout(time, ot, mean1, message);

    return hpx::finalize();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]){
    // Configure application-specific options.
    options_description
        desc_commandline("usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()
        ("number-created,N",
            boost::program_options::value<uint64_t>()
                ->default_value(500000),
            "number of components created")
        ("csv",
            "output results as csv "
            "(format:count,mean,accurate mean,variance,min,max)");

    // Initialize and run HPX
    vector<string> cfg;
    outname = argv[0];
    cfg.push_back("hpx.components.emptycomponent.enabled = 1");
    return hpx::init(desc_commandline, argc, argv, cfg);
}

///////////////////////////////////////////////////////////////////////////////

