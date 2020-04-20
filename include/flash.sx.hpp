#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("flash.sx")]] flash : public contract {

public:
	using contract::contract;

	/**
	 * ## TABLE `balances`
	 *
	 * - scope: `contract`
	 *
	 * ### params
	 *
	 * - `{name} account` - account balance holder
	 * - `{map<name, asset>} balances` - accounts of balances
	 *
	 * ### example
	 *
	 * ```json
	 * {
	 *   "account": "myaccount",
	 *   "balances": ??
	 * }
	 * ```
	 */
	struct [[eosio::table("balances"), eosio::contract("flash.sx")]] balances_row {
		name                	 	account;
		map<symbol_code, asset>    	balances;

		uint64_t primary_key() const { return account.value; }
	};
	typedef eosio::multi_index< "balances"_n, balances_row> balances_table;

    /**
     * ## ACTION `borrow`
     *
     * Account requests to borrow quantity
     *
     * - **authority**: `any`
     *
     * ### params
     *
	 * - `{name} to` - receiver of flash loan
	 * - `{name} contract` - token contract account
	 * - `{asset} quantity` - flash loan request amount
	 * - `{string} memo` - used for outgoing transfer
	 * - `{set<name>} notifiers` - notify accounts via `callback` action after transfer has been sent
	 *
	 * ### Example 1
     *
	 * ```c++
	 * const asset quantity = asset{10000, symbol{"EOS", 4}};
	 * flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
	 * borrow.send( get_self(), "eosio.token"_n, quantity, "my memo", set<name>() );
	 * ```
	 *
	 * ### Example 2
	 *
	 * ```bash
	 * $ cleos push action flash.sx borrow '["myaccount", "eosio.token", "1.0000 EOS", "my memo", []]' -p myaccount
	 * ```
     */
	[[eosio::action]]
	void borrow( const name to, const name contract, const asset quantity, const string memo, const set<name> notifiers );

	/**
	 * ## ACTION `checkbalance`
     *
     * Throws error if account does not have equal or above desired asset quantity
     *
     * - **authority**: `any`
     *
     * ### params
	 *
	 * - `{name} account` - account to check if minimum balance is available
	 * - `{name} contract` - token contract account
	 * - `{asset} quantity` - minimum required balance amount
	 *
	 * ### returns
	 *
	 * - `{asset}` - current balance of account
	 *
	 * ### Example 1
	 *
	 * ```c++
	 * const name account = "myaccount"_n;
	 * const name contract = "eosio.token"_n;
	 * const asset quantity = asset{10000, symbol{"EOS", 4}};
	 *
	 * flash::checkbalance_action checkbalance( "flash.sx"_n, { account, "active"_n });
	 * checkbalance.send( account, contract, quantity );
	 * ```
	 *
	 * ### Example 2
	 *
	 * ```bash
	 * $ cleos push action flash.sx checkbalance '["myaccount", "eosio.token", "1.0000 EOS"] -p myaccount
	 * ```
	 */
	[[eosio::action]]
	asset checkbalance( const name account, const name contract, const asset quantity );

    /**
     * ## ACTION `callback`
     *
     * Notifies recipient account via `callback` action after transfer has been sent from `borrow` action
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
	 * - `{name} recipient` - callback recipient
	 * - `{name} to` - receiver of flash loan
	 * - `{name} contract` - token contract account
	 * - `{asset} quantity` - flash loan request amount
	 * - `{string} memo` - used for outgoing transfer
	 *
	 * ### Example
     *
	 * ```c++
	 * [[eosio::on_notify("flash.sx::callback")]]
	 * void callback( const name recipient, const name to, const name contract, asset quantity, const string memo )
	 * {
	 *     token::transfer_action transfer( contract, { get_self(), "active"_n });
	 * 	   transfer.send( get_self(), "flash.sx"_n, quantity, memo );
	 * }
	 * ```
     */
	[[eosio::action]]
	void callback( const name recipient, const name to, const name contract, const asset quantity, const string memo );

    /**
     * ## ACTION `withdraw`
     *
     * Allows account to withdraw available balance from contract.
     *
     * - **authority**: `account`
     *
     * ### params
     *
	 * - `{name} account` - account to withdraw from
	 * - `{name} contract` - token contract account
	 * - `{asset} quantity` - withdraw amount
	 *
	 * ### Example
     *
	 * ```bash
	 * cleos push action flash.sx withdraw '["myaccount", "eosio.token", "1.0000 EOS"]' -p myaccount
	 * ```
     */
	[[eosio::action]]
	void withdraw( const name account, const name contract, const asset quantity );

	[[eosio::on_notify("*::transfer")]]
	void on_transfer( const name from, const name to, asset quantity, const string memo );

	// action wrappers
	using borrow_action = eosio::action_wrapper<"borrow"_n, &flash::borrow>;
	using checkbalance_action = eosio::action_wrapper<"checkbalance"_n, &flash::checkbalance>;
	using callback_action = eosio::action_wrapper<"callback"_n, &flash::callback>;
	using withdraw_action = eosio::action_wrapper<"withdraw"_n, &flash::withdraw>;

private:
	void sub_balance( const name account, const name contract, const asset quantity );
	void add_balance( const name account, const name contract, const asset quantity, const name ram_payer );
};
