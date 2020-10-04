#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

#include <eosio.token/eosio.token.hpp>

#include "../flash.sx.hpp"

using namespace eosio;
using namespace std;

class [[eosio::contract("callback")]] example : public contract {

public:
	using contract::contract;

	[[eosio::action]]
	void init( const name to, const asset quantity )
	{
		flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
		borrow.send( to, "eosio.token"_n, quantity, "callback memo", get_self() );
	}

	[[eosio::on_notify("flash.sx::callback")]]
	void on_callback( const name from, const name to, const name contract, asset quantity, const string memo, const name recipient )
	{
		// do actions before sending funds back
		// PLACE YOUR CODE HERE

		// repay flashloan
		token::transfer_action transfer( contract, { to, "active"_n });
		transfer.send( to, "flash.sx"_n, quantity, memo );
	}
};
