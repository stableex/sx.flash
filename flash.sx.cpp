#include <eosio.token/eosio.token.hpp>
#include "flash.sx.hpp"

[[eosio::action]]
void flash::borrow( const name to, const name contract, const asset quantity, const optional<string> memo, const optional<name> notifier )
{
    // no authority required
    check( is_account( to ), to.to_string() + " account does not exists");
    const symbol_code symcode = quantity.symbol.code();

    // static actions
    flash::callback_action callback( get_self(), { get_self(), "active"_n });
    flash::checkbalance_action checkbalance( get_self(), { get_self(), "active"_n });
    token::transfer_action transfer( contract, { get_self(), "active"_n });

    // prevent sending transfer if `to` account does not contain any balance
    // prevents exploit from consuming RAM from contract
    check_open( contract, to, symcode );
    check_open( contract, get_self(), symcode );

    // get initial balance of contract
    const asset balance = token::get_balance( contract, get_self(), symcode );
    check( balance.amount >= quantity.amount, "maximum borrow amount is " + balance.to_string() );

    // transfer funds to borrower
    transfer.send( get_self(), to, quantity, *memo );

    // notify recipient accounts after transfer has been sent
    if ( notifier->value ) {
        callback.send( to, contract, quantity, *memo, *notifier );
    }

    // save & check if balance is above
    map<symbol_code, name> symcodes;
    symcodes[ symcode ] = contract;
    savebalance( get_self(), symcodes );
    checkbalance.send( get_self(), symcodes );
}

[[eosio::action]]
void flash::savebalance( const name account, const map<symbol_code, name> symcodes )
{
    // table
    balances_table _balances( get_self(), get_self().value );
    auto itr = _balances.find( account.value );

    // unpack map object
    for ( const auto row : symcodes ) {
        const symbol_code symcode = row.first;
        const name contract = row.second;

        // get current balance
        check_open( contract, account, symcode );
        const asset balance = token::get_balance( contract, account, symcode );

        // create
        if ( itr == _balances.end() ) {
            _balances.emplace( get_self(), [&]( auto& row ) {
                row.account = account;
                row.balances[symcode] = extended_asset{ balance, contract };
            });
        // modify
        } else {
            _balances.modify( itr, get_self(), [&]( auto& row ) {
                row.balances[symcode] = extended_asset{ balance, contract };
            });
        }
    }
}

[[eosio::action]]
void flash::checkbalance( const name account, const map<symbol_code, name> symcodes )
{
    // fetch previously saved balance
    balances_table _balances( get_self(), get_self().value );
    const map<symbol_code, extended_asset> balances = _balances.get( account.value, "[account] not found, must first execute `savebalance`" ).balances;

    // unpack map object
    for ( const auto row : symcodes ) {
        const symbol_code symcode = row.first;
        const name contract = row.second;

        // get previously saved balance
        const extended_asset ext_balance = balances.at(symcode);
        const asset balance = ext_balance.quantity;

        // validate balance
        check( balance.is_valid(), "[symcode] not found, must first execute `savebalance`");
        check( ext_balance.contract == contract, "[contract] does not match account balance contract");

        // get current balance
        check_open( contract, account, symcode );
        const asset current_balance = token::get_balance( contract, account, symcode );

        // check balance of account, if below the desired amount, fail the transaction
        check( current_balance >= balance, account.to_string() + " must have a balance equal or above " + balance.to_string() );
    }
}

[[eosio::action]]
void flash::callback( const name to, const name contract, const asset quantity, const string memo, const name notifier )
{
    require_auth( get_self() );

    check( is_account( notifier ), notifier.to_string() + " notifier account does not exists");
    require_recipient( notifier );
}

void flash::check_open( const name contract, const name account, const symbol_code symcode )
{
    token::accounts _accounts( contract, account.value );
    auto itr = _accounts.find( symcode.raw() );
    check( itr != _accounts.end(), account.to_string() + " account must have " + symcode.to_string() + " `open` balance" );
}