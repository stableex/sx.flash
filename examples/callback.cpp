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
	void init( const name receiver, const asset quantity )
	{
		sx::flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
		borrow.send( receiver, extended_asset{quantity, "eosio.token"_n}, "callback memo", get_self() );
	}

	[[eosio::on_notify("flash.sx::callback")]]
	void on_callback( const name code, const name receiver, const extended_asset amount, const string memo, const name recipient )
	{
		// do actions before sending funds back
		// PLACE YOUR CODE HERE

		// repay flashloan
		const asset fee = sx::flash::calculate_fee( code, amount.quantity );
		token::transfer_action transfer( amount.contract, { receiver, "active"_n });
		transfer.send( receiver, code, amount.quantity + fee, memo );
	}
};
