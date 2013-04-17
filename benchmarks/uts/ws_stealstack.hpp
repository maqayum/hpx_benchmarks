
//  Copyright (c) 2013 Thomas Heller
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BENCHMARKS_UTS_WS_STEALSTACK_HPP
#define BENCHMARKS_UTS_WS_STEALSTACK_HPP

#include <benchmarks/uts/params.hpp>
#include <hpx/include/components.hpp>

namespace components
{
    struct ws_stealstack
      : hpx::components::simple_component_base<ws_stealstack>
    {
        enum states
        {
            WORK    = 0,
            SEARCH  = 1,
            IDLE    = 2,
            OVH     = 3,
            NSTATES = 4
        };

        struct stats
        {
            stats()
              : n_nodes(0)
              , n_leaves(0)
              , n_release(0)
              , n_acquire(0)
              , n_steal(0)
              , n_fail(0)
              , max_stack_depth(0)
              , max_tree_depth(0)
            {
                time[WORK] = 0.0;
                time[SEARCH] = 0.0;
                time[IDLE] = 0.0;
                time[OVH] = 0.0;
            }

            template <typename Archive>
            void serialize(Archive & ar, unsigned)
            {
                ar & n_nodes;
                ar & n_leaves;
                ar & n_acquire;
                ar & n_release;
                ar & n_steal;
                ar & n_fail;

                ar & max_stack_depth;
                ar & max_tree_depth;
                
                ar & time;
            }
            
            std::size_t n_nodes;
            std::size_t n_leaves;
            std::size_t n_acquire;
            std::size_t n_release;
            std::size_t n_steal;
            std::size_t n_fail;

            int max_stack_depth;
            int max_tree_depth;
            
            double time[NSTATES];
        };

        ws_stealstack()
          : global_work(0)
          , local_work(0)
          , walltime(0)
          , work_time(0)
          , search_time(0)
          , idle_time(0)
          , idle_sessions(0)
          , time_last(0)
          , curState(0)
          , start_time(0)
          , chunks_recvd(0)
          , chunks_sent(0)
          , ctrl_recvd(0)
          , ctrl_sent(0)
          , pollint_adaptive(false)
        {
        }

        void init_symbolic_names()
        {
            names.reserve(size);
            for(std::size_t i = 0; i < size; ++i)
            {
                std::string name("ws_stealstack_");
                name += boost::lexical_cast<std::string>(i);
                names.push_back(name);
            }
            
            hpx::agas::register_name(names[rank], this->get_gid());
        }

        void init(params p, std::size_t r, std::size_t s)
        {
            rank = r;
            size = s;
            param = p;

            hpx::future<void> f = hpx::async(&ws_stealstack::init_symbolic_names, this);

            last_steal = rank;

            if(p.polling_interval == 0)
            {
                pollint_adaptive = true;
                p.polling_interval = 1;
            }

            if(rank == 0)
            {
                node n;
                n.init_root(param);
                put_work(n);
            }

            hpx::wait(f);
        }

        HPX_DEFINE_COMPONENT_ACTION(ws_stealstack, init);

        void resolve_names()
        {
            ids.resize(size);
            for(std::size_t i = 0; i < names.size(); ++i)
            {
                hpx::agas::resolve_name(names[i], ids[i]);
            }
        }
        
        HPX_DEFINE_COMPONENT_ACTION(ws_stealstack, resolve_names);

        void put_work(node const & n)
        {
            /* If the stack is empty, push an empty stealstack_node. */
            if(local_queue.empty())
            {
                stealstack_node node(param.chunk_size);
                local_queue.push_front(node);
            }


            /* If the current stealstack_node is full, push a new one. */
            if(local_queue.front().work.size() == param.chunk_size)
            {
                stealstack_node node(param.chunk_size);
                local_queue.push_front(node);
            }
            else if (local_queue.front().work.size() > param.chunk_size)
            {
                throw std::logic_error("ss_put_work(): Block has overflowed!");
            }
            
            stealstack_node & node = local_queue.front();

            node.work.push_back(n);
            local_work++;
            stat.max_stack_depth = std::max(local_work, stat.max_stack_depth);
        }

        void gen_children(node & parent)
        {
            int parent_height = parent.height;

            stat.max_tree_depth = std::max(stat.max_tree_depth, parent_height);

            int num_children = parent.get_num_children(param);
            int child_type = parent.child_type(param);

            parent.num_children = num_children;

            if(num_children > 0)
            {

                for(int i = 0; i < num_children; ++i)
                {
                    node child;
                    child.type = child_type;
                    child.height = parent_height + 1;
                    for(int j = 0; j < param.compute_granularity; ++j)
                    {
                        rng_spawn(parent.state.state, child.state.state, i);
                    }

                    put_work(child);
                }
            }
            else
            {
                ++stat.n_leaves;
            }
        }

        bool ensure_local_work()
        {
            if(local_work < 0)
            {
                throw std::logic_error("ensure_local_work(): local_work count is less than 0!");
            }

            if(local_work == 0)
            {
                return false;
            }

            return true;
        }

        bool get_work(node & n)
        {
            if(!ensure_local_work())
            {
                return false;
            }

            stealstack_node & node = local_queue.front();

            if(node.work.size() > 0)
            {
                n = node.work.back();
                node.work.pop_back();
            }
            else
            {
                hpx::cout << "get_work(): called with node.work.size() = 0, "
                    << "local_work=" << local_work
                    << " or " << (local_work % param.chunk_size)
                    << " (mod " << param.chunk_size << ")\n" << hpx::flush;
                throw std::logic_error("get_work(): Underflow!");
            }

            if(node.work.size() == 0)
            {
                local_queue.pop_front();
            }

            ++stat.n_nodes;
            --local_work;

            return true;
        }

        void tree_search()
        {
            node parent;
            while(get_work(parent))
            {
                gen_children(parent);
            }
        }
        
        HPX_DEFINE_COMPONENT_ACTION(ws_stealstack, tree_search);

        stats get_stats()
        {
            return stat;
        }

        HPX_DEFINE_COMPONENT_ACTION(ws_stealstack, get_stats);

        private:
            std::vector<std::string> names;
            std::vector<hpx::id_type> ids;
            int global_work;
            int local_work;

            stats stat;

            double walltime;
            double work_time;
            double search_time;
            double idle_time;
            int idle_sessions;
            double time_last;
            int entries[NSTATES];
            int curState;

            double start_time;

            std::deque<stealstack_node> local_queue;
            int last_steal;
            int chunks_recvd;
            int chunks_sent;
            int ctrl_recvd;
            int ctrl_sent;

            params param;
            bool pollint_adaptive;
            std::size_t rank;
            std::size_t size;
    };
}

#endif