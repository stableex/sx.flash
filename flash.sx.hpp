#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

namespace sx {

using eosio::asset;
using eosio::name;
using eosio::check;
using eosio::symbol_code;

using std::string;
using std::optional;

class [[eosio::contract("flash.sx")]] flash : public eosio::contract {

public:
    using contract::contract;

    /**
     * ## TABLE `settings`
     *
     * - `{uint8_t} fee` - processing fee (bips 1/100 of 1%)
     *
     * ### example
     *
     * ```json
     * {
     *     "fee": 0
     * }
     * ```
     */
    struct [[eosio::table("settings")]] settings {
        uint8_t     fee = 0;
    };
    typedef eosio::singleton< "settings"_n, settings > settings_table;

    /**
     * ## TABLE `balances`
     *
     * - `{name} contract` - contract name
     * - `{asset} balance` - balance amount
     *
     * ### example
     *
     * ```json
     * {
     *     "contract": "eosio.token",
     *     "balance": "1.0000 EOS"
     * }
     * ```
     */
    struct [[eosio::table("balances")]] balances_row {
        name        contract;
        asset       balance;

        uint64_t primary_key() const { return contract.value; }
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
     * Throws error if contract does not have equal or above previously saved balance
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} contract` - contract name
     * - `{symbol_code} symcode` - symbol code
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action flash.sx checkbalance '["eosio.token", "EOS"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void checkbalance( const name contract, const symbol_code symcode );

    /**
     * ## ACTION `callback`
     *
     * Notifies recipient account via `callback` action after transfer has been sent from `borrow` action
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} from` - sender of flash loan
     * - `{name} to` - receiver of flash loan
     * - `{name} contract` - token contract account
     * - `{asset} quantity` - flash loan request amount
     * - `{string} memo` - used for outgoing transfer
     * - `{name} notifier` - callback notifier account
     *
     * ### Example
     *
     * ```c++
     * [[eosio::on_notify("flash.sx::callback")]]
     * void callback( const name from, const name to, const name contract, asset quantity, const string memo, const name notifier )
     * {
     *     token::transfer_action transfer( contract, { get_self(), "active"_n });
     * 	   transfer.send( get_self(), "flash.sx"_n, quantity, memo );
     * }
     * ```
     */
    [[eosio::action]]
    void callback( const name from, const name to, const name contract, const asset quantity, const string memo, const name notifier );

    // action wrappers
    using borrow_action = eosio::action_wrapper<"borrow"_n, &flash::borrow>;
    using callback_action = eosio::action_wrapper<"callback"_n, &flash::callback>;
    using checkbalance_action = eosio::action_wrapper<"checkbalance"_n, &flash::checkbalance>;

private:
    void check_open( const name contract, const name account, const symbol_code symcode );
    void save_balance( const name contract, const asset balance );
};
}