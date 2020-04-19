# SX `Flashloans` ⚡️ - EOSIO Smart Contract

> Borrow any amount of liquidity instantly for $0 fees & $0 collateral required.

## Table of Content

- [`quickstart`](#quickstart)
- [`borrow`](#action-borrow)
- [`checkbalance`](#action-checkbalance)

## Quickstart

1. import `external/flash.sx.hpp` into your project
2. use static action `borrow` with parameters (`to`,`contract`,`quantity`, `memo`)
3. `on_notify` to intercept incoming token transfer from borrowing flashloan
  - Custom logic (trading, arbitrage, etc..)
  - Repay loan at the end of transaction

### Basic Implementation

```c++
#include <flash.sx.hpp>

[[eosio::action]]
void myaction( const name contract, const asset quantity, const string memo )
{
	flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
	borrow.send( get_self(), contract, quantity, memo );
}

[[eosio::on_notify("*::transfer")]]
void on_transfer( const name from, const name to, asset quantity, const string memo )
{
	// only handle incoming transfers from flash.sx
	if ( from != "flash.sx"_n ) return;

	// do actions before sending funds back

	// ==========================
	// PLACE YOUR CODE HERE
	//
	// EXAMPLE CODE - add & minus
	// ==========================
	if ( memo == "minus" ) quantity.amount -= quantity.amount / 2;
	if ( memo == "add" ) quantity.amount += quantity.amount / 2;

	// repay flashloan
	token::transfer_action transfer( get_first_receiver(), { get_self(), "active"_n });
	transfer.send( get_self(), from, quantity, memo );
}
```

## ACTION `borrow`

Account requests to borrow quantity

- **authority**: `any`

### params

- `{name} to` - receiver of flash loan
- `{name} contract` - token contract account
- `{asset} quantity` - flash loan request amount
- `{string} memo` - used for outgoing transfer
- `{set<name>} notifiers` - notify accounts via `callback` action after transfer has been sent

### Example 1

```c++
const asset quantity = asset{10000, symbol{"EOS", 4}};
flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
borrow.send( get_self(), "eosio.token"_n, quantity, "my memo", set<name>() );
```

### Example 2

```bash
$ cleos push action flash.sx borrow '["myaccount", "eosio.token", "1.0000 EOS", "my memo", []]' -p myaccount
```

## ACTION `checkbalance`

Throws error if account does not have equal or above desired asset quantity

- **authority**: `any`

### params

- `{name} account` - account to check if minimum balance is available
- `{name} contract` - token contract account
- `{asset} quantity` - minimum required balance amount

### returns

- `{asset}` - current balance of account

### Example 1

```c++
const name account = "myaccount"_n;
const name contract = "eosio.token"_n;
const asset quantity = asset{10000, symbol{"EOS", 4}};

flash::checkbalance_action checkbalance( "flash.sx"_n, { account, "active"_n });
checkbalance.send( account, contract, quantity );
```

### Example 2

```bash
$ cleos push action flash.sx checkbalance '["myaccount", "eosio.token", "1.0000 EOS"] -p myaccount
```

## Build

```bash
$ eosio-cpp flash.sx.cpp -o flash.sx.wasm -I external
```