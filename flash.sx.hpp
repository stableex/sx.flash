#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

namespace sx {

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
     *     "fee": 2
     * }
     * ```
     */
    struct [[eosio::table("settings")]] settings {
        uint8_t     fee = 2;
    };
    typedef eosio::singleton< "settings"_n, settings > settings_table;

    /**
     * ## TABLE `state`
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
    struct [[eosio::table("state")]] state {
        name        contract;
        asset       balance;
    };
    typedef eosio::singleton< "state"_n, state > state_table;

    /**
     * ## ACTION `borrow`
     *
     * Account requests to borrow quantity
     *
     * - **authority**: `any`
     *
     * ### params
     *
     * - `{name} receiver` - receiver of flash loan
     * - `{extended_asset} amount` - flash loan request amount
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
    void borrow( const name receiver, const extended_asset amount, const optional<string> memo, const optional<name> notifier );

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
     * - `{asset} fee` - processing fee
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action flash.sx checkbalance '["eosio.token", "EOS", "0.0002 EOS"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void checkbalance( const name contract, const symbol_code symcode, const asset fee );

    /**
     * ## ACTION `callback`
     *
     * Notifies recipient account via `callback` action after transfer has been sent from `borrow` action
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} code` - flash loan contract
     * - `{name} receiver` - receiver of flash loan
     * - `{extended_asset} amount` - flash loan request amount
     * - `{asset} fee` - flash loan fee
     * - `{string} [memo=""]` - used for outgoing transfer
     * - `{name} [notifier=""]` - callback notifier account
     *
     * ### Example
     *
     * ```c++
     * [[eosio::on_notify("flash.sx::callback")]]
     * void callback( const name code, const name receiver, const extended_asset amount, const string memo, const name notifier )
     * {
     *     const asset fee = sx::flash::calculate_fee( code, amount.quantity );
     *
     *     eosio::token::transfer_action transfer( amount.contract, { receiver, "active"_n });
     * 	   transfer.send( receiver, code, amount.quantity + fee, memo );
     * }
     * ```
     */
    [[eosio::action]]
    void callback( const name code, const name receiver, const extended_asset amount, const asset fee, const string memo, const name notifier );

    [[eosio::action]]
    void setsettings( const optional<sx::flash::settings> settings );

    [[eosio::action]]
    void flashlog( const name code, const name receiver, const extended_asset amount, const asset fee );

    /**
     * Notify contract when any token transfer notifiers relay contract
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );

    /**
     * ## STATIC `calculate_fee`
     *
     * Calculate processing fee
     *
     * ### params
     *
     * - `{name} code` - flash loan code
     * - `{asset} quantity` - quantity input
     *
     * ### example
     *
     * ```c++
     * const asset quantity = asset{10000, symbol{"EOS", 4}}; // 1.0000 EOS
     * const asset fee = sx::flash::calculate_fee( "flash.sx"_n, quantity );
     * // => 0.0001 EOS
     * ```
     */
    static asset calculate_fee( const name code, const asset quantity )
    {
        sx::flash::settings_table _settings( code, code.value );
        check( _settings.exists(), "code is under maintenance");
        return _settings.get().fee * quantity / 10000;
    }

    // action wrappers
    using borrow_action = eosio::action_wrapper<"borrow"_n, &sx::flash::borrow>;
    using callback_action = eosio::action_wrapper<"callback"_n, &sx::flash::callback>;
    using checkbalance_action = eosio::action_wrapper<"checkbalance"_n, &sx::flash::checkbalance>;
    using flashlog_action = eosio::action_wrapper<"flashlog"_n, &sx::flash::flashlog>;

private:
    // flash
    void check_open( const name contract, const name account, const symbol_code symcode );
    void save_balance( const name contract, const asset balance );
};
}