#include "flash.sx.hpp"
#include "eosio.token.hpp"

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
    token::accounts _accounts( contract, to.value );
    _accounts.get( symcode.raw(), "to account must have open balance" );

    // get initial balance of contract
    const asset balance = token::get_balance( contract, get_self(), symcode );
    check( balance.amount >= quantity.amount, "maximum borrow amount is " + balance.to_string() );

    // transfer funds to borrower
    transfer.send( get_self(), to, quantity, *memo );

    // notify recipient accounts after transfer has been sent
    if ( notifier->value ) {
        callback.send( to, contract, quantity, *memo, *notifier );
    }

    // check if balance is above
    checkbalance.send( get_self(), contract, balance );
}

[[eosio::action]]
asset flash::checkbalance( const name account, const name contract, const asset quantity )
{
    // check balance of account, if below the desired amount, fail the transaction
    const asset balance = token::get_balance( contract, account, quantity.symbol.code() );
    check( balance.amount >= quantity.amount, account.to_string() + " must have a balance equal or above " + quantity.to_string() );
    return balance;
}

[[eosio::action]]
void flash::callback( const name to, const name contract, const asset quantity, const string memo, const name recipient )
{
    require_auth( get_self() );

    check( is_account( recipient ), recipient.to_string() + " recipient account does not exists");
    require_recipient( recipient );
}
