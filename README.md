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

**basic.cpp**

```c++
[[eosio::action]]
void init( const asset quantity )
{
	flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
	borrow.send( get_self(), "eosio.token"_n, quantity, "", set<name>() );
}

[[eosio::on_notify("*::transfer")]]
void on_transfer( const name from, const name to, asset quantity, const string memo )
{
	// only handle incoming transfers from contract
	if ( from != "flash.sx"_n ) return;

	// do actions before sending funds back
	// PLACE YOUR CODE HERE

	// repay flashloan
	token::transfer_action transfer( get_first_receiver(), { get_self(), "active"_n });
	transfer.send( get_self(), from, quantity, memo );
}
```

**notifiers.cpp**

```c++
[[eosio::action]]
void init( const name to, const asset quantity, const set<name> notifiers )
{
	flash::borrow_action borrow( "flash.sx"_n, { get_self(), "active"_n });
	borrow.send( to, "eosio.token"_n, quantity, "", notifiers );
}

[[eosio::on_notify("flash.sx::callback")]]
void callback( const name recipient, const name to, const name contract, asset quantity, const string memo )
{
	// do actions before sending funds back
	// PLACE YOUR CODE HERE

	// repay flashloan
	token::transfer_action transfer( contract, { get_self(), "active"_n });
	transfer.send( get_self(), "flash.sx"_n, quantity, memo );
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

## ACTION `callback`

Notifies recipient account via `callback` action after transfer has been sent from `borrow` action

- **authority**: `get_self()`

### params

- `{name} recipient` - callback recipient
- `{name} to` - receiver of flash loan
- `{name} contract` - token contract account
- `{asset} quantity` - flash loan request amount
- `{string} memo` - used for outgoing transfer

### Example

```c++
[[eosio::on_notify("flash.sx::callback")]]
void callback( const name recipient, const name to, const name contract, asset quantity, const string memo )
{
	token::transfer_action transfer( contract, { get_self(), "active"_n });
	transfer.send( get_self(), "flash.sx"_n, quantity, memo );
}
```

## Build

```bash
$ eosio-cpp flash.sx.cpp -o flash.sx.wasm -I external
```