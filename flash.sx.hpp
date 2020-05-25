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
     * - `{name} account` - account name
     * - `{map<symbol_code, extended_asset>} balances` - balances
     *
     * ### example
     *
     * ```json
     * {
     *     "account": "myaccount",
     *     "balances": [
     *         { "key": "EOS", "value": {"contract": "eosio.token", "quantity": "1.0000 EOS" } }
     *     ]
     * }
     * ```
     */
    struct [[eosio::table("balances")]] balances_row {
        name                                account;
        map<symbol_code, extended_asset>    balances;

        uint64_t primary_key() const { return account.value; }
    };
    typedef eosio::multi_index< "balances"_n, balances_row > balances_table;

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
     * - `{string} [memo=""]` - (optional) transfer memo
     * - `{name} [notifier=""]` - (optional) notify accounts after transfer has been sent
     *
     * ### Example 1
     *
     * ```c++
     * const asset quantity = asset{10000, symbol{"EOS", 4}};
     * flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
     * borrow.send( get_self(), "eosio.token"_n, quantity, "my memo", "notifyme" );
     * ```
     *
     * ### Example 2
     *
     * ```bash
     * $ cleos push action flash.sx borrow '["myaccount", "eosio.token", "1.0000 EOS", "my memo", "notifyme"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void borrow( const name to, const name contract, const asset quantity, const optional<string> memo, const optional<name> notifier );

    /**
     * ## ACTION `checkbalance`
     *
     * Throws error if account does not have equal or above previously saved balance
     *
     * - **authority**: `any`
     *
     * ### params
     *
     * - `{name} account` - account to check if minimum balance is available
     * - `{name} contract` - token contract account
     * - `{symbol_code} symcode` - symbol code
     * - `{quantity} [addition=null]` - (optional) additional quantity to saved balance
     *
     * ### Example 1
     *
     * ```c++
     * const name account = "myaccount"_n;
     * const name contract = "eosio.token"_n;
     * const symbol_code symcode = symbol_code{"EOS"};
     * const asset addition = asset{0, symbol{"EOS", 4}};
     *
     * flash::checkbalance_action checkbalance( "flash.sx"_n, { account, "active"_n });
     * checkbalance.send( account, contract, symcode, addition );
     * ```
     *
     * ### Example 2
     *
     * ```bash
     * $ cleos push action flash.sx checkbalance '["myaccount", "eosio.token", "EOS", "0.0000 EOS"] -p myaccount
     * ```
     */
    [[eosio::action]]
    void checkbalance( const name account, const name contract, const symbol_code symcode, const optional<asset> addition );

    /**
     * ## ACTION `savebalance`
     *
     * Save existing balance of account
     *
     * - **authority**: `any`
     *
     * ### params
     *
     * - `{name} account` - account to save balance from
     * - `{name} contract` - token contract account
     * - `{symbol_code} symcode` - symbol code
     *
     * ### Example 1
     *
     * ```c++
     * const name account = "myaccount"_n;
     * const name contract = "eosio.token"_n;
     * const symbol_code symcode = symbol_code{"EOS"};
     *
     * flash::savebalance_action savebalance( "flash.sx"_n, { account, "active"_n });
     * savebalance.send( account, contract, symcode );
     * ```
     *
     * ### Example 2
     *
     * ```bash
     * $ cleos push action flash.sx checkbalance '["myaccount", "eosio.token", "EOS"] -p myaccount
     * ```
     */
    [[eosio::action]]
    void savebalance( const name account, const name contract, const symbol_code symcode );

    /**
     * ## ACTION `callback`
     *
     * Notifies recipient account via `callback` action after transfer has been sent from `borrow` action
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} to` - receiver of flash loan
     * - `{name} contract` - token contract account
     * - `{asset} quantity` - flash loan request amount
     * - `{string} memo` - used for outgoing transfer
     * - `{name} recipient` - callback recipient
     *
     * ### Example
     *
     * ```c++
     * [[eosio::on_notify("flash.sx::callback")]]
     * void callback( const name to, const name contract, asset quantity, const string memo, const name recipient )
     * {
     *     token::transfer_action transfer( contract, { get_self(), "active"_n });
     * 	   transfer.send( get_self(), "flash.sx"_n, quantity, memo );
     * }
     * ```
     */
    [[eosio::action]]
    void callback( const name to, const name contract, const asset quantity, const string memo, const name recipient );

    // action wrappers
    using borrow_action = eosio::action_wrapper<"borrow"_n, &flash::borrow>;
    using savebalance_action = eosio::action_wrapper<"savebalance"_n, &flash::savebalance>;
    using checkbalance_action = eosio::action_wrapper<"checkbalance"_n, &flash::checkbalance>;
    using callback_action = eosio::action_wrapper<"callback"_n, &flash::callback>;

private:
    void check_open( const name contract, const name account, const symbol_code symcode );
};
