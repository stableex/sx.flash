#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

#include <flash.sx.hpp>
#include <eosio.token.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract]] notifiers : public contract {

public:
	using contract::contract;

	[[eosio::action]]
	void init( const name to, const asset quantity, const set<name> notifiers )
	{
		flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
		borrow.send( to, "eosio.token"_n, quantity, "", notifiers );
	}

	[[eosio::on_notify("flash.sx::callback")]]
	void callback( const name recipient, const name to, const name contract, asset quantity, const string memo )
	{
		// do actions before sending funds back
		// PLACE YOUR CODE HERE

		// repay flashloan
		token::transfer_action transfer( contract, { get_self(), "active"_n });
		transfer.send( get_self(), "flash.sx"_n, quantity, memo );
	}
};
