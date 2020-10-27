#include <eosio.token/eosio.token.hpp>

#include "flash.sx.hpp"

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
    eosio::token::transfer_action transfer( contract, { get_self(), "active"_n });

    // get initial balance of contract & save
    check_open( contract, get_self(), symcode );
    const asset fee = sx::flash::calculate_fee( get_self(), quantity );
    const asset balance = eosio::token::get_balance( contract, get_self(), symcode );
    check( balance - fee >= quantity, get_self().to_string() + ": maximum borrow amount is " + (balance - fee).to_string() );
    save_balance( contract, balance + fee );

    // prevent sending transfer if `receiver` account does not contain any balance
    // prevents exploit from consuming RAM from contract
    check_open( contract, receiver, symcode );

    // 1. transfer funds to receiver
    transfer.send( get_self(), receiver, quantity, *memo );

    // 2. notify recipient accounts after transfer has been sent
    if ( notifier->value ) callback.send( get_self(), receiver, extended_asset{quantity, contract}, *memo, *notifier );

    // 3. check if balance is higher than previous
    checkbalance.send( contract, symcode );

    // 4. Logging
    flashlog.send( receiver, amount.quantity, fee, balance + fee );
}

[[eosio::action]]
void sx::flash::setsettings( const sx::flash::settings settings )
{
    require_auth( get_self() );

    sx::flash::settings_table _settings( get_self(), get_self().value );
    _settings.set( settings, get_self() );
}

void sx::flash::save_balance( const name contract, const asset balance )
{
    // table
    state_table _state( get_self(), get_self().value );
    check( !_state.exists(), get_self().to_string() + ": balance already exists, must now use `checkbalance`");

    // save contract balance
    // can only be created once (to prevent double entry attacks)
    _state.set( {contract, balance}, get_self() );
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

    // check balance of account, if below the desired amount, fail the transaction
    check( balance >= _state.get().balance, get_self().to_string() + ": borrowed quantity was not repaid before the end of inline action");

    // delete state once check is completed (to prevent double entry attacks)
    _state.remove();
}

[[eosio::action]]
void sx::flash::flashlog( const name receiver, const asset borrow, const asset fee, const asset reserve )
{
    require_auth( get_self() );

    if ( is_account("stats.sx"_n )) require_recipient( "stats.sx"_n );
}


[[eosio::action]]
void sx::flash::callback( const name code, const name receiver, const extended_asset amount, const string memo, const name notifier )
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