[[eosio::action]]
void flash::borrow( const name to, const name contract, const asset quantity, const string memo, const set<name> notifiers )
{
    // no authority required
    check( is_account( to ), to.to_string() + " account does not exists");

    // static actions
    token::transfer_action transfer( contract, { get_self(), "active"_n });
    flash::callback_action callback( get_self(), { get_self(), "active"_n });
    flash::checkbalance_action checkbalance( get_self(), { get_self(), "active"_n });

    // prevent sending transfer if `to` account does not contain any balance (prevents consuming RAM from contract)
    const symbol_code symcode = quantity.symbol.code();
    token::accounts accounts( contract, to.value );
    const auto accounts_itr = accounts.find( symcode.raw() );
    check( accounts_itr != accounts.end(), to.to_string() + " must have an open balance for " + symcode.to_string() );

    // get initial balance of contract
    const asset balance = token::get_balance( contract, get_self(), symcode );
    check( balance.amount >= quantity.amount, "maximum borrow amount is " + balance.to_string() );

    // transfer funds to borrower
    transfer.send( get_self(), to, quantity, memo );

    // notify recipient accounts via `callback` action after transfer has been sent
    for ( const name recipient : notifiers ) {
        callback.send( recipient, to, contract, quantity, memo );
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
void flash::callback( const name recipient, const name to, const name contract, const asset quantity, const string memo )
{
    require_auth( get_self() );

    check( is_account( recipient ), recipient.to_string() + " recipient account does not exists");
    require_recipient( recipient );
}
