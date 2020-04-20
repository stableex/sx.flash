[[eosio::on_notify("*::transfer")]]
void flash::on_transfer( const name from, const name to, asset quantity, const string memo )
{
	// only monitor incoming transfers
	if ( to != get_self() ) return;

	// exclude system contracts
	const set<name> system = {"eosio.ram"_n, "eosio.stake"_n, "eosio.rex"_n};
	if ( system.find( from ) != system.end() ) return;

	// incoming token inputs
	const name contract = get_first_receiver();
	const symbol_code symcode = quantity.symbol.code();

	// add balance to incoming account
	add_balance( from, contract, quantity, get_self() );
}