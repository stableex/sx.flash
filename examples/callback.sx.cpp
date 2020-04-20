#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

#include <flash.sx.hpp>
#include <eosio.token.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("callback.sx")]] example : public contract {

public:
	using contract::contract;

	[[eosio::action]]
	void init( const name to, const asset quantity )
	{
		flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
		borrow.send( to, "eosio.token"_n, quantity, "", set<name>{ get_self() } );
	}

	[[eosio::on_notify("flash.sx::callback")]]
	void callback( const name recipient, const name to, const name contract, asset quantity, const string memo )
	{
		// do actions before sending funds back
		// PLACE YOUR CODE HERE

		// repay flashloan
		token::transfer_action transfer( contract, { to, "active"_n });
		transfer.send( to, "flash.sx"_n, quantity, memo );
	}
};
