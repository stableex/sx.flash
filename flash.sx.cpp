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
    savebalance( get_self(), contract, symcode );
    checkbalance.send( get_self(), contract, symcode, asset{0, quantity.symbol} );
}

[[eosio::action]]
void flash::savebalance( const name account, const name contract, const symbol_code symcode )
{
    // table
    balances_table _balances( get_self(), get_self().value );
    auto itr = _balances.find( account.value );

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

[[eosio::action]]
void flash::checkbalance( const name account, const name contract, const symbol_code symcode, const optional<asset> addition )
{
    // fetch previously saved balance
    balances_table _balances( get_self(), get_self().value );
    const map<symbol_code, extended_asset> balances = _balances.get( account.value, "[account] not found, must first execute `savebalance`" ).balances;

    const extended_asset ext_balance = balances.at(symcode);
    asset balance = ext_balance.quantity;

    // validate balance
    check( balance.is_valid(), "[symcode] not found, must first execute `savebalance`");
    check( ext_balance.contract == contract, "[contract] does not match account balance contract");

    // add addition to existing balance
    if ( addition ) {
        check( addition->symbol.code() == symcode, "[addition] does not match [symcode]");
        check( addition->symbol == balance.symbol, "[addition] does not match account balance symbol");
        balance.amount += addition->amount;
    }

    // get current balance
    check_open( contract, account, symcode );
    const asset current_balance = token::get_balance( contract, account, symcode );

    // check balance of account, if below the desired amount, fail the transaction
    check( current_balance >= balance, account.to_string() + " must have a balance equal or above " + balance.to_string() );
}

[[eosio::action]]
void flash::callback( const name to, const name contract, const asset quantity, const string memo, const name recipient )
{
    require_auth( get_self() );

    check( is_account( recipient ), recipient.to_string() + " recipient account does not exists");
    require_recipient( recipient );
}

void flash::check_open( const name contract, const name account, const symbol_code symcode )
{
    token::accounts _accounts( contract, account.value );
    auto itr = _accounts.find( symcode.raw() );
    check( itr != _accounts.end(), account.to_string() + " account must have `open` balance" );
}