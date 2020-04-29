#include "flash.sx.hpp"
#include "eosio.token.hpp"

[[eosio::action]]
void flash::borrow( const name to, const name contract, const asset quantity, const optional<string> memo, const optional<name> notifier )
{
    // no authority required
    check( is_account( to ), to.to_string() + " account does not exists");
    const symbol_code symcode = quantity.symbol.code();

    // prevent sending transfer if `to` account does not contain any balance
    // prevents exploit from consuming RAM from contract
    check_open_balance( contract, to, symcode );

    // get initial balance of contract
    const asset balance = token::get_balance( contract, get_self(), symcode );
    check( balance.amount >= quantity.amount, "maximum borrow amount is " + balance.to_string() );

    // transfer funds to borrower
    token::transfer_action transfer( contract, { get_self(), "active"_n });
    transfer.send( get_self(), to, quantity, *memo );

    // notify recipient accounts after transfer has been sent
    if ( notifier->value ) {
        check( is_account( *notifier ), notifier->to_string() + " notifier account does not exists");
        require_recipient( *notifier );
    }

    // check if balance is above
    flash::checkbalance_action checkbalance( get_self(), { get_self(), "active"_n });
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

void flash::check_open_balance( const name contract, const name owner, const symbol_code symcode )
{
    token::accounts _accounts( contract, owner.value );
    const auto itr = _accounts.find( symcode.raw() );
    check( itr != _accounts.end(), owner.to_string() + " must have an open balance for " + symcode.to_string() );
}