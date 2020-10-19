
// import the order book
#include <esl/economics/markets/order_book/book.hpp>

// import currencies, because we will be trading the stock against USD
#include <esl/economics/currencies.hpp>

// we will fix the namespace mess once the API stabilises
using namespace esl;
using namespace esl::economics;
using namespace esl::economics::markets::order_book;
using esl::economics::markets::order_book::limit_order_message;
using namespace esl::economics::markets::order_book::statically_allocated;

// trades in this market are in multiples of the lot size
// e.g. a lot_size of 100 means that a buy order for a quantity of 3 buys 300 shares
// A larger lot size enables small tick sizes
constexpr size_t lot_size = 100;

// helper function to create limit orders submitted to an exchange.
// note that ESL uses fixed-precision
limit_order_message create(double p, size_t q = 1000, limit_order_message::side_t side = limit_order_message::side_t::sell)
{
    // substitute your own stock ticker type here
    esl::economics::markets::ticker ticker_dummy_;

    // substitute your agent id
    identity<agent> owner_dummy_;

    return limit_order_message
        ( ticker_dummy_
            , owner_dummy_
            , side
            , markets::quote(price(p, currencies::USD)
                                , lot_size * currencies::USD.denominator)
            , q
            ,limit_order_message::lifetime_t::good_until_cancelled
        );
}

//
int main(int argc, char** argv) {
    // create the range of valid prices for the order book (inclusive)
    auto  min_ = markets::quote(price( 0.01, currencies::USD), lot_size * currencies::USD.denominator);
    auto  max_ = markets::quote(price(10.00, currencies::USD), lot_size * currencies::USD.denominator);

    // define the maximum number of open orders
    // as the statically_allocated::book maintains a fixed-size memory pool
    constexpr size_t max_orders_ = 1'00'000;

    // creates the order book
    auto book_ = std::make_unique<book<max_orders_>>(min_, max_);

    book_->insert(create(4.75, 500, limit_order_message::side_t::buy));
    book_->insert(create(4.75, 750, limit_order_message::side_t::sell));

    // use book_.reports to view the output generated by the order book, which contains
    //  1. the placement of the buy order
    //  2. matching 500 of the buy and sell order (confirmed for each agent)
    //  3. placement of the remainder 250 of the sell order
}