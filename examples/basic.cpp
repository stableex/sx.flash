#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

#include <flash.sx.hpp>
#include <eosio.token.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract]] basic : public contract {

public:
	using contract::contract;

	[[eosio::action]]
	void myaction( const name contract, const asset quantity, const string memo )
	{
		flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
		borrow.send( get_self(), contract, quantity, memo );
	}

	[[eosio::on_notify("*::transfer")]]
	void on_transfer( const name from, const name to, asset quantity, const string memo )
	{
		// only handle incoming transfers from flash.sx
		if ( from != "flash.sx"_n ) return;

		// do actions before sending funds back

		// ==========================
		// PLACE YOUR CODE HERE
		//
		// EXAMPLE CODE - add & minus
		// ==========================
		if ( memo == "minus" ) quantity.amount -= quantity.amount / 2;
		if ( memo == "add" ) quantity.amount += quantity.amount / 2;

		// repay flashloan
		token::transfer_action transfer( get_first_receiver(), { get_self(), "active"_n });
		transfer.send( get_self(), from, quantity, memo );
	}

	// action wrappers
	using myaction_action = eosio::action_wrapper<"myaction"_n, &basic::myaction>;
};
