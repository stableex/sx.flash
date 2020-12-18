#include <eosio.token/eosio.token.hpp>
#include <sx.vaults/vaults.sx.hpp>

#include "flash.sx.hpp"

/**
 * Notify contract when any token transfer notifiers relay contract
 */
[[eosio::on_notify("*::transfer")]]
void sx::flash::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    sx::flash::settings_table _settings( get_self(), get_self().value );
    sx::flash::state_table _state( get_self(), get_self().value );

    check( _settings.exists(), "code is under maintenance");

    // prevent incoming funds from vault.sx when flash loan is active
    if ( _state.exists() ) check( from != "vaults.sx"_n, get_self().to_string() + ": during active flash loan, incoming transfers from `vaults.sx` are not allowed");
}

[[eosio::action]]
void sx::flash::borrow( const name receiver, const extended_asset amount, const optional<string> memo, const optional<name> notifier )
{
    // no authority required
    check( is_account( receiver ), get_self().to_string() + ": receiver " + receiver.to_string() + " account does not exists");
    const name contract = amount.contract;
    const asset quantity = amount.quantity;
    const symbol_code symcode = quantity.symbol.code();

    // static actions
    sx::flash::callback_action callback( get_self(), { get_self(), "active"_n });
    sx::flash::checkbalance_action checkbalance( get_self(), { get_self(), "active"_n });
    sx::flash::flashlog_action flashlog( get_self(), { get_self(), "active"_n });
    sx::vaults::update_action update( "vaults.sx"_n, { get_self(), "active"_n });
    eosio::token::transfer_action transfer( contract, { get_self(), "active"_n });

    // get initial balance of contract & save
    check_open( contract, get_self(), symcode );
    const asset fee = sx::flash::calculate_fee( get_self(), quantity );
    const asset balance = eosio::token::get_balance( contract, get_self(), symcode );
    check( balance - fee >= quantity, get_self().to_string() + ": maximum borrow amount is " + (balance - fee).to_string() );

    // prevent sending transfer if `receiver` account does not contain any balance
    // prevents exploit from consuming RAM from contract
    check_open( contract, receiver, symcode );

    // 1. save existing balance of borrowed asset
    save_balance( contract, balance + fee );

    // 2. transfer funds to receiver
    transfer.send( get_self(), receiver, quantity, *memo );

    // 3. notify recipient accounts after transfer has been sent
    if ( notifier->value ) callback.send( get_self(), receiver, extended_asset{quantity, contract}, fee, *memo, *notifier );

    // 4. check if balance is higher than previous
    checkbalance.send( contract, symcode );

    // 5. update vault balance
    update.send( symcode );

    // 6. logging
    flashlog.send( get_self(), receiver, amount, fee );
}

[[eosio::action]]
void sx::flash::flashlog( const name code, const name receiver, const extended_asset amount, const asset fee )
{
    require_auth( get_self() );

    if ( is_account("stats.sx"_n) ) require_recipient( "stats.sx"_n );
    if ( is_account("vaults.sx"_n) ) require_recipient( "vaults.sx"_n );
}

[[eosio::action]]
void sx::flash::setsettings( const optional<sx::flash::settings> settings )
{
    require_auth( get_self() );

    sx::flash::settings_table _settings( get_self(), get_self().value );
    if ( settings ) _settings.set( *settings, get_self() );
    else _settings.remove();
}

void sx::flash::save_balance( const name contract, const asset balance )
{
    // can only be created once (to prevent double entry attacks)
    state_table _state( get_self(), get_self().value );
    check( !_state.exists(), get_self().to_string() + ": balance already exists, must now use `checkbalance`");

    // save contract balance
    _state.set( { contract, balance }, get_self() );
}

[[eosio::action]]
void sx::flash::checkbalance( const name contract, const symbol_code symcode )
{
    require_auth( get_self() );

    // fetch previously saved balance
    state_table _state( get_self(), get_self().value );
    check( _state.exists(), get_self().to_string() + ": must first execute `borrow`");

    // get current balance
    const asset balance = eosio::token::get_balance( contract, get_self(), symcode );

    // check balance of account, if below the desired amount,  fail the transaction
    check( balance >= _state.get().balance, get_self().to_string() + ": borrowed quantity was not repaid before the end of inline action");

    // delete state once check is completed (to prevent double entry attacks)
    _state.remove();
}

[[eosio::action]]
void sx::flash::callback( const name code, const name receiver, const extended_asset amount, const asset fee, const string memo, const name notifier )
{
    require_auth( get_self() );

    check( is_account( notifier ), get_self().to_string() + ": " + notifier.to_string() + " notifier account does not exists");
    require_recipient( notifier );
}

void sx::flash::check_open( const name contract, const name account, const symbol_code symcode )
{
    eosio::token::accounts _accounts( contract, account.value );
    auto itr = _accounts.find( symcode.raw() );
    check( itr != _accounts.end(), get_self().to_string() + ": " + account.to_string() + " account must have " + symcode.to_string() + " `open` balance" );
}