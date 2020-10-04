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

    // get initial balance of contract & save
    const asset balance = token::get_balance( contract, get_self(), symcode );
    check( balance.amount >= quantity.amount, "maximum borrow amount is " + balance.to_string() );
    save_balance( contract, balance );

    // 1. transfer funds to borrower
    transfer.send( get_self(), to, quantity, *memo );

    // 2. notify recipient accounts after transfer has been sent
    if ( notifier->value ) callback.send( get_self(), to, contract, quantity, *memo, *notifier );

    // 3. check if balance is higher than previous
    checkbalance.send( contract, symcode );
}

void flash::save_balance( const name contract, const asset balance )
{
    // table
    balances_table _balances( get_self(), get_self().value );
    auto itr = _balances.find( contract.value );
    check( itr == _balances.end(), "balance already exists, must now use `checkbalance`");

    // save contract balance
    // can only be created once (to prevent double entry attacks)
    _balances.emplace( get_self(), [&]( auto& row ) {
        row.contract = contract;
        row.balance = balance;
    });
}

[[eosio::action]]
void flash::checkbalance( const name contract, const symbol_code symcode )
{
    require_auth( get_self() );

    // fetch previously saved balance
    balances_table _balances( get_self(), get_self().value );
    const auto itr = _balances.find( contract.value );
    check( itr != _balances.end(), "must first execute `borrow`");

    // get current balance
    const asset balance = token::get_balance( contract, get_self(), symcode );

    // check balance of account, if below the desired amount, fail the transaction
    check( balance >= itr->balance, get_self().to_string() + " must have a balance equal or above " + balance.to_string() );

    // delete balances once check is completed (to prevent double entry attacks)
    _balances.erase( itr );
}

[[eosio::action]]
void flash::callback( const name from, const name to, const name contract, const asset quantity, const string memo, const name notifier )
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