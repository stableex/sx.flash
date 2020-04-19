#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

#include <eosio.token.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("flash.sx")]] flash : public contract {

public:
	using contract::contract;

	[[eosio::action]]
	void borrow( const name to, const name contract, const asset quantity, const string memo, const set<name> notifiers )
	{
		// no authority required
		check( is_account( to ), to.to_string() + " account does not exists");

		// prevent sending transfer if `to` account does not contain any balance
		const symbol_code symcode = quantity.symbol.code();
		check( token::get_balance( contract, to, symcode ).amount, to.to_string() + " must have an open balance for " + symcode.to_string() );

		// get initial balance of contract
		const asset balance = token::get_balance( contract, get_self(), quantity.symbol.code() );
		check( balance.amount >= quantity.amount, "maximum borrow amount is " + balance.to_string() );

		// transfer funds to borrower
		token::transfer_action transfer( contract, { get_self(), "active"_n });
		transfer.send( get_self(), to, quantity, memo );

		// notify recipient accounts via `callback` action after transfer has been sent
		for ( const name recipient : notifiers ) {
			callback_action callback( get_self(), { get_self(), "active"_n });
			callback.send( recipient, to, contract, quantity, memo );
		}

		// check if balance is above
		checkbalance_action checkbalance( get_self(), { get_self(), "active"_n });
		checkbalance.send( get_self(), contract, balance );
	}

	[[eosio::action]]
	asset checkbalance( const name account, const name contract, const asset quantity )
	{
		// check balance of account, if below the desired amount, fail the transaction
		const asset balance = token::get_balance( contract, account, quantity.symbol.code() );
		check( balance.amount >= quantity.amount, account.to_string() + " must have a balance equal or above " + quantity.to_string() );
		return balance;
	}

	[[eosio::action]]
	void callback( const name recipient, const name to, const name contract, const asset quantity, const string memo )
	{
		check( is_account( recipient ), recipient.to_string() + " recipient account does not exists");
		require_recipient( recipient );
	}

	// action wrappers
	using borrow_action = eosio::action_wrapper<"borrow"_n, &flash::borrow>;
	using checkbalance_action = eosio::action_wrapper<"checkbalance"_n, &flash::checkbalance>;
	using callback_action = eosio::action_wrapper<"callback"_n, &flash::callback>;
};
