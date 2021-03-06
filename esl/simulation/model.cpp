/// \file   model.cpp
///
/// \brief
///
/// \authors    Maarten P. Scholl
/// \date       2017-12-06
/// \copyright  Copyright 2017-2019 The Institute for New Economic Thinking,
///             Oxford Martin School, University of Oxford
///
///             Licensed under the Apache License, Version 2.0 (the "License");
///             you may not use this file except in compliance with the License.
///             You may obtain a copy of the License at
///
///                 http://www.apache.org/licenses/LICENSE-2.0
///
///             Unless required by applicable law or agreed to in writing,
///             software distributed under the License is distributed on an "AS
///             IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
///             express or implied. See the License for the specific language
///             governing permissions and limitations under the License.
///
///             You may obtain instructions to fulfill the attribution
///             requirements in CITATION.cff
///
#include <esl/simulation/model.hpp>

#include <chrono>
using std::chrono::high_resolution_clock;
#include <numeric>

#include <esl/agent.hpp>
#include <esl/computation/environment.hpp>
#include <esl/data/log.hpp>
#include <esl/quantity.hpp>


namespace esl::simulation {
    model::model( computation::environment &e
        , const parameter::parametrization &parameters)
        : environment_(e)
        , rounds_(0)
        , parameters(parameters)
        , start(parameters.get<time_point>("start"))
        , end(parameters.get<time_point>("end"))
        , time(parameters.get<time_point>("start"))
        , sample(parameters.get<std::uint64_t>("sample"))
        , agents(e)
        , verbosity(parameters.get<std::uint64_t>("verbosity"))
        , threads( std::max(1u, parameters.get<unsigned int>("threads")))
    {

    }

    void model::initialize()
    {

    }

    //std::map<esl::identity<esl::agent>, double> timings_;
    //std::map<esl::identity<esl::agent>, double> timings_cb_;
    //std::map<esl::identity<esl::agent>, double> timings_act_;

    time_point model::step(time_interval step)
    {
        assert(!step.empty());

        auto timer_start_ = high_resolution_clock::now();
        environment_.before_step();

        // read the sample index from the parameter collection

        std::mutex mutex_first_event_;
        time_point first_event_   = step.upper;
        unsigned int round_ = 0;
        do {

            if (verbosity > 0 && 0 == (rounds_ % verbosity)){
                LOG(notice) << "time " << step << " round " << round_  << std::endl;
            }
            first_event_   = step.upper;


            auto job_ = [&](std::shared_ptr<agent> a){
                // double agent_cb_end_;
                // timings_.emplace(i, 0.);
                // timings_cb_.emplace(i, 0.);
                // timings_act_.emplace(i, 0.);
                // auto agent_start_ = high_resolution_clock::now();
                // auto agent_act_ = high_resolution_clock::now();
                // The seed is deterministic in the following variables:
                std::seed_seq seed_ {
                    std::uint64_t(std::hash<identity<agent>>()(a->identifier)),
                    std::uint64_t(step.lower), std::uint64_t(round_),
                    sample};

                // try {

                {
                    std::unique_lock lock_(mutex_first_event_);
                    first_event_ = std::min(first_event_,
                                            a->process_messages(step, seed_));
                    // agent_cb_end_ = double((high_resolution_clock::now() - agent_start_).count()); agent_act_ = high_resolution_clock::now();
                    first_event_ = std::min(first_event_, a->act(step, seed_));
                }

                //} catch(const std::runtime_error &e) {
                //    LOG(errorlog) << e.what() << std::endl;
                //    throw e;
                //} catch(const std::exception &e) {
                //    LOG(errorlog) <<  e.what() << std::endl;
                //    throw e;
                //} catch(...) {
                //    throw;
                //}
                a->inbox.clear();
                // auto agent_end_ = high_resolution_clock::now() - agent_start_;


                // timings_cb_[a->identifier] += agent_cb_end_;
                // timings_act_[a->identifier] += double( (high_resolution_clock::now() - agent_act_).count());
                // timings_[a->identifier] += (double(agent_end_.count()) );
                // LOG(notice) << "a" << a->identifier << " tcb " << step << " " << std::setprecision(8) << timings_cb_[a->identifier]/ 1e+9/step.lower <<  " s" << std::endl; LOG(notice) << "a" << a->identifier << " tac " << step << " " << std::setprecision(8) << timings_act_[a->identifier]/ 1e+9/step.lower <<  " s" << std::endl;

            };

            // important: if using a single thread, run everything in main
            if(threads <= 1) {
                for(auto &[i, a] : agents.local_agents_) {
                    job_(a);
                }
            }else{
                std::vector<std::thread> threads_;
                auto iterator_ = agents.local_agents_.begin();
                for(const auto& tasks_: quantity(agents.local_agents_.size()) / threads){
                    std::vector<std::shared_ptr<agent>> task_split_;
                    for(auto i = quantity(0); i < tasks_; ++i){
                        task_split_.push_back(iterator_->second);
                        std::advance(iterator_, 1);
                    }

                    threads_.emplace_back([&](std::vector<std::shared_ptr<agent>> ts){
                            for(const auto& a: ts){
                                job_(a);
                            }
                        }, task_split_);
                }

                for(auto &t: threads_){
                    t.join();
                }

            }

            environment_.send_messages(*this);
            ++round_;
            ++rounds_;
        } while(step.lower >= first_event_);

        environment_.after_step(*this);
        auto total_ = high_resolution_clock::now() - timer_start_;
        //LOG(notice) << "step " << step << " took " << (double(total_.count()) / 1e+9) <<  " seconds" << std::endl;
        return first_event_;
    }

    void model::terminate()
    {

    }

}  // namespace esl::simulation
