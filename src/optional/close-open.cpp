[[eosio::action]]
void flash::close( const name account, const name contract, const symbol_code symcode )
{
    require_auth( account );

    // get user
    flash::balances_table _balances( get_self(), contract.value );
    const auto from = _balances.find( account.value );
    check( from != _balances.end(), "account has no balances");

    // fetch balances
    const asset balance = from->balances.at( symcode );
    check( balance.symbol.code() == symcode, "account has no balance");
    check( balance.amount == 0, "balance is not zero" );

    // remove balance
    bool is_empty = false;
    _balances.modify( from, same_payer, [&]( auto& row ) {
        row.balances.erase( symcode );
        if ( row.balances.size() == 0 ) is_empty = true;
    });
    if ( is_empty ) _balances.erase( from );
}

[[eosio::action]]
void flash::open( const name account, const name contract, const symbol sym, const name ram_payer )
{
    check( is_account( account ), "account does not exist" );

    // verify sym with token supply
	token::stats _stats( contract, sym.code().raw() );
	const asset supply = _stats.get( sym.code().raw(), "symbol does not exist" ).supply;
	check( supply.symbol == sym, "symbol precision mismatch" );
    check( supply.amount > 0, "supply must be positive");

	add_balance( account, contract, asset{0, sym}, ram_payer );
}
