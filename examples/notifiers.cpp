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
	void myaction( const name to, const name contract, const asset quantity, const string memo, const set<name> notifiers )
	{
		flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
		borrow.send( to, contract, quantity, memo, notifiers );
	}

	[[eosio::on_notify("flash.sx::callback")]]
	void callback( const name recipient, const name to, const name contract, asset quantity, const string memo )
	{
		token::transfer_action transfer( contract, { get_self(), "active"_n });
		transfer.send( get_self(), "flash.sx"_n, quantity, memo );
	}

	// action wrappers
	using myaction_action = eosio::action_wrapper<"myaction"_n, &notifiers::myaction>;
};
