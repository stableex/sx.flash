#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

static constexpr int64_t asset_mask{(1LL << 62) - 1};
static constexpr int64_t asset_max{ asset_mask }; //  4611686018427387903
static constexpr name TOKEN_CONTRACT = "token.sx"_n;

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
     * ## TABLE `vault`
     *
     * - `{asset} balance` - vault deposit balance
     * - `{asset} supply` - vault active supply
     * - `{time_point_sec} last_updated` - last updated timestamp
     *
     * ### example
     *
     * ```json
     * {
     *   "balance": {"quantity": "100.0000 EOS", "contract": "eosio.token"},
     *   "supply": {"quantity": "1000000.0000 SXEOS", "contract": "token.sx"},
     *   "last_updated": "2020-11-23T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("vault")]] vault_row {
        extended_asset          balance;
        extended_asset          supply;
        time_point_sec          last_updated;

        uint64_t primary_key() const { return balance.quantity.symbol.code().raw(); }
        uint64_t by_supply() const { return supply.quantity.symbol.code().raw(); }
    };
    typedef eosio::multi_index< "vault"_n, vault_row,
        indexed_by<"bysupply"_n, const_mem_fun<vault_row, uint64_t, &vault_row::by_supply>>
    > vault_table;

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

    /**
     * ## ACTION `setvault`
     *
     * Set initial vault balance & supply
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{extended_symbol} deposit` - deposit symbol
     * - `{symbol_code} supply` - liquidity supply symbol
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action vault.sx setvault '[["4,EOS", "eosio.token"], "SXEOS"]' -p vault.sx
     * ```
     */
    [[eosio::action]]
    void setvault( const extended_symbol deposit, const symbol_code supply );

    [[eosio::action]]
    void setsettings( const sx::flash::settings settings );

   /**
     * Notify contract when any token transfer notifiers relay contract
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const std::string memo );

    /**
     * ## STATIC `calculate_fee`
     *
     * Calculate processing fee
     *
     * ### params
     *
     * - `{name} contract` - flash loan contract
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
    static asset calculate_fee( const name contract, const asset quantity )
    {
        sx::flash::settings_table _settings( contract, contract.value );
        check( _settings.exists(), "contract is under maintenance");
        return _settings.get().fee * quantity / 10000;
    }

    // action wrappers
    using setvault_action = eosio::action_wrapper<"setvault"_n, &sx::flash::setvault>;
    using borrow_action = eosio::action_wrapper<"borrow"_n, &sx::flash::borrow>;
    using callback_action = eosio::action_wrapper<"callback"_n, &sx::flash::callback>;
    using checkbalance_action = eosio::action_wrapper<"checkbalance"_n, &sx::flash::checkbalance>;

private:
    // flash
    void check_open( const name contract, const name account, const symbol_code symcode );
    void save_balance( const name contract, const asset balance );

    // eosio.token helper
    void transfer( const name from, const name to, const extended_asset value, const string memo );
    void create( const extended_symbol value );
    void retire( const extended_asset value, const string memo );
    void issue( const extended_asset value, const string memo );

    // vault
    extended_asset calculate_issue( const symbol_code id, const asset payment );
    extended_asset calculate_retire( const symbol_code id, const asset payment );
    void set_balance( const extended_asset value );
};
}