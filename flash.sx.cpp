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
    vector<extended_symbol> symcodes;
    symcodes.push_back(extended_symbol{ symbol{symcode, 0}, contract });
    save_balance( get_self(), symcodes );
    checkbalance.send( get_self(), symcodes );
}

[[eosio::action]]
void flash::savebalance( const name account, const vector<extended_symbol> symcodes )
{
    require_auth( account );
    save_balance( account, symcodes );
}

void flash::save_balance( const name account, const vector<extended_symbol> symcodes )
{
    check( is_account( account ), "account does not exists");
    check( symcodes.size(), "symcodes is empty");

    // table
    balances_table _balances( get_self(), get_self().value );
    auto itr = _balances.find( account.value );
    check( itr == _balances.end(), "balance already exists, must now use `checkbalance`");

    // unpack map object
    map<uint64_t, extended_asset> balances;
    for ( const auto row : symcodes ) {
        const symbol_code symcode = row.get_symbol().code();
        const name contract = row.get_contract();
        const uint64_t id = symcode.raw() + contract.value;
        check( balances.find( id ) == balances.end(), "balance id already exists");

        // get current balance
        check_open( contract, account, symcode );
        const asset balance = token::get_balance( contract, account, symcode );
        balances[id] = extended_asset{ balance, contract };
    }

    // save user balances
    // can only be created once (to prevent double entry attacks)
    _balances.emplace( get_self(), [&]( auto& row ) {
        row.account = account;
        row.balances = balances;
    });
}

[[eosio::action]]
void flash::checkbalance( const name account, const vector<extended_symbol> symcodes )
{
    check( is_account( account ), "account does not exists");
    check( symcodes.size(), "symcodes is empty");

    // fetch previously saved balance
    balances_table _balances( get_self(), get_self().value );
    const auto itr = _balances.find( account.value );
    check( itr != _balances.end(), "[account] not found, must first execute `savebalance`");
    const map<uint64_t, extended_asset> balances = itr->balances;

    // unpack map object
    for ( const auto row : symcodes ) {
        const symbol_code symcode = row.get_symbol().code();
        const name contract = row.get_contract();
        const uint64_t id = symcode.raw() + contract.value;

        // get previously saved balance
        const extended_asset ext_balance = balances.at(id);
        const asset balance = ext_balance.quantity;

        // validate balance
        check( balance.is_valid(), "[symcode] + [contract] not found, must first execute `savebalance`");
        check( ext_balance.contract == contract, "[contract] does not match account balance contract");

        // get current balance
        check_open( contract, account, symcode );
        const asset current_balance = token::get_balance( contract, account, symcode );

        // check balance of account, if below the desired amount, fail the transaction
        check( current_balance >= balance, account.to_string() + " must have a balance equal or above " + balance.to_string() );
    }
    // delete balances once check is completed (to prevent double entry attacks)
    _balances.erase( itr );
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