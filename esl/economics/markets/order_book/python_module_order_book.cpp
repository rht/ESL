/// \file   python_module_order_book.cpp
///
/// \brief
///
/// \authors    maarten
/// \date       2020-11-01
/// \copyright  Copyright 2017-2020 The Institute for New Economic Thinking,
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
#ifdef WITH_PYTHON
#include <esl/economics/markets/order_book/python_module_order_book.hpp>
#include <esl/economics/markets/order_book/matching_engine.hpp>
#include <esl/economics/markets/order_book/book.hpp>
using namespace esl::economics::markets::order_book;


#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(order_book)
{
    enum_<execution_report::state_t>("state_t")
        .value("invalid", execution_report::state_t::invalid)
        .value("cancel", execution_report::state_t::cancel)
        .value("match", execution_report::state_t::match)
        .value("placement", execution_report::state_t::placement)
        ;

    class_<execution_report>("execution_report")
        .def_readwrite("quantity", &execution_report::quantity)
        .def_readwrite("identifier", &execution_report::identifier)
        .def_readwrite("side", &execution_report::side)
        .def_readwrite("limit", &execution_report::limit)
        .def_readwrite("owner", &execution_report::owner)
        .def("__repr__", &execution_report::representation)
        .def("__str__", &execution_report::representation)

    ;

}

#endif
