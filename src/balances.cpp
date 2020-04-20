[[eosio::action]]
void flash::withdraw( const name account, const name contract, const asset quantity )
{
    require_auth( account );
	sub_balance( account, contract, quantity );
}

void flash::sub_balance( const name account, const name contract, const asset quantity )
{
    const symbol_code symcode = quantity.symbol.code();

    // get user
	flash::balances_table _balances( get_self(), contract.value );
    const auto from = _balances.find( account.value );
    check( from != _balances.end(), "account has no balances");

    // fetch balances
    const asset balance = from->balances.at( symcode );
    check( balance.symbol == quantity.symbol, "account has no balance");

    // substract balance
    bool is_empty = false;
    _balances.modify( from, same_payer, [&]( auto& row ) {
        row.balances[ symcode ] -= quantity;
        check( row.balances[ symcode ].amount >= 0, "overdrawn balance" );
        if ( row.balances[ symcode ].amount == 0 ) row.balances.erase( symcode );
        if ( row.balances.size() == 0 ) is_empty = true;
    });
    if ( is_empty ) _balances.erase( from );
}

void flash::add_balance( const name account, const name contract, const asset quantity, const name ram_payer )
{
    const symbol_code symcode = quantity.symbol.code();

	flash::balances_table _balances( get_self(), contract.value );
    const auto to = _balances.find( account.value );

    if ( to == _balances.end() ) {
        _balances.emplace( ram_payer, [&]( auto& row ){
            row.account = account;
            row.balances[ symcode ] = quantity;
        });
    } else {
        _balances.modify( to, same_payer, [&]( auto& row ) {
            if (row.balances[ symcode ].symbol.code() == symcode ) {
                row.balances[ symcode ] += quantity;
            } else {
                row.balances[ symcode ] = quantity;
            }
        });
    }
}