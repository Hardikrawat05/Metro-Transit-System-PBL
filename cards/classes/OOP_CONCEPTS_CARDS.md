# Object-Oriented Programming in the Metro Smart Card Module

**Project:** Metro Transit System (PBL)
**Module:** Smart Cards (`cards/classes/`)
**Language:** C++

---

## 1. Overview

The smart card module handles everything a passenger does with a metro card: getting a card, recharging it, tapping in and out at gates, buying day or monthly passes, and replacing a lost card. It also covers the admin side (login, undo, activity log) and saving data to disk.

Files in `cards/classes/`:

| File | Responsibility |
|---|---|
| `smart_cards.cpp` | Card issue, recharge, tap in/out, passes, block and transfer, trip history |
| `clock.cpp` | Simulated system time (used for pass expiry and overstay penalty) |
| `admin_undo.cpp` | Admin login with lock-out, password change, undo, activity log |
| `storage.cpp` | Saving and loading all data, with corruption detection |

These files contain the method *implementations*. The class *declarations* are in `metro.h`: `Metro`, `Clock`, `Storage`, the `FarePolicy` family, `Array<T>`, `RouteHolder`.

The underlying data structures (hash table, stack, deque, linked list, graph) are written by hand. The OOP layer sits on top and provides a safe interface to them.

---

## 2. Which OOP concept is used for which feature

| OOP concept | Where | Feature it supports |
|---|---|---|
| **Encapsulation** | `Metro` private members (`cards_`, `adminHash_`, `failedLogins_`, `cardPtr()`) | Card balance and admin security can only change through validated methods |
| **Abstraction** | Public methods `createCard`, `recharge`, `tapIn`, `tapOut`, `buyPass`, `blockAndTransfer` | Menu code does not need to know how cards are stored |
| **Abstract class** | `FarePolicy` with pure virtual `name()` and `discountPercent()` | Common contract for passenger categories |
| **Inheritance** | `RegularFare`, `StudentFare`, `SeniorFare`, `DisabledFare` derive from `FarePolicy` | Concession categories (0%, 50%, 30%, 50% discount) |
| **Polymorphism** | `farePolicy(category).apply(baseFare)` | One code path calculates fare and pass price for every category |
| **Method overriding** | `waitlistPriority()` overridden in `StudentFare`, `SeniorFare`, `DisabledFare` | Seniors and disabled passengers get higher waitlist priority |
| **Template method pattern** | `FarePolicy::apply()` calls the virtual `discountPercent()` | Discount formula written once, subclasses supply only the percentage |
| **Virtual destructor** | `virtual ~FarePolicy()` | Correct cleanup through a base reference |
| **Static members** | `Clock` (all members static) | One shared simulated time for the whole system |
| **Class template** | `Array<T>` | Own dynamic array, used for trip history, undo history, activity log |
| **Function templates** | `collectValues<T>`, `sortRecords<T>`, `compareById<T>` | Writing cards and waitlist entries to file in sorted order |
| **Operator overloading** | `Array::operator[]`, `operator=`, `begin()/end()` | Natural syntax such as `cards[i]` and range-for loops |
| **Rule of Three** | `Array<T>`: destructor, copy constructor, copy assignment | Deep copies, no memory leaks or double frees |
| **RAII** | `RouteHolder`, `LineList`, `Metro::~Metro()` | Memory is released automatically, even on early return |
| **Friend class** | `friend class Storage;` in `Metro` | Save/load needs access to all internal state |
| **Exception handling** | `throw runtime_error` and `catch` in `storage.cpp`; `bad_alloc` in `Metro()` | Corrupt save files are reported instead of crashing |
| **Enums / enum class** | `CardStatus`, `PassType`, `Category`, `LoadStatus` | Readable states instead of magic numbers |
| **Const-correctness** | `findCard() const`, `hasValidPass() const`, `undoHistory() const` | Read-only queries cannot change data |
| **Result object** | `Result`, `TapOutResult`, `BookingResult` | Operations return status and message instead of printing |

---

## 3. Key design: fare calculation with polymorphism

```
            FarePolicy  (abstract)
            name(), discountPercent()
            waitlistPriority() = 3
            apply(base) -> base - base*discount/100
                    |
   +----------------+----------------+----------------+
   |                |                |                |
RegularFare     StudentFare      SeniorFare      DisabledFare
 0% off          50% off          30% off          50% off
                 priority 2       priority 0       priority 0
```

At tap-out, the fare is calculated like this:

```cpp
const FarePolicy& policy = farePolicy(c->category);
r.fare = policy.apply(baseFare);
```

There is no `if (category == STUDENT) ... else if ...` chain. To add a new category (for example "Defence personnel"), we add one subclass and one `case` in `farePolicy()`. The same call is reused for pass prices:

```cpp
int Metro::passPrice(int category, int passType) const {
    int base = passType == PASS_DAY ? DAY_PASS_PRICE : MONTHLY_PASS_PRICE;
    return farePolicy(category).apply(base);
}
```

This follows the Open/Closed principle: open for extension, closed for modification.

---

## 4. File-wise explanation

### 4.1 `smart_cards.cpp` (card lifecycle)

- **Encapsulation:** `cardPtr()` returns a modifiable card and is **private**. The public `findCard()` returns `const Card*`. So a balance can only change through `recharge`, `tapOut` or `buyPass`, and each one enforces its rules (maximum recharge, maximum balance, blocked card, minimum balance to enter).
- **Polymorphism:** fare and pass price discounts (Section 3).
- **RAII:** `RouteHolder rh;` wraps the shortest-path result in `tapOut`. Its destructor frees the memory on every return path.
- **Result object:** `tapOut` returns a `TapOutResult` with fare, penalty, whether a pass was used, distance and remaining balance. The menu decides how to show it.
- **Business rules implemented:** minimum balance to tap in, overstay penalty after the maximum trip time, free travel with a valid pass, and blocking a lost card while transferring its balance to a new card.

### 4.2 `clock.cpp` (simulated time)

- `Clock` is a utility class with only **static** members and a private static `offsetSeconds_`.
- `now()` returns real time plus the offset, so we can fast-forward time with `advanceMinutes()` to demonstrate pass expiry, overstay penalties and admin lock-out without waiting.

### 4.3 `admin_undo.cpp` (admin security and undo)

- **Encapsulation of security data:** `adminHash_`, `failedLogins_` and `lockedUntil_` are private. Only a salted hash of the password is stored, never the password. After 3 wrong attempts, login is locked for 60 seconds.
- **Command pattern:** each admin action is stored as an `AdminAction` record. `undoLast()` pops the latest action and `applyUndo()` performs the reverse. If an undo is not possible, the action is pushed back so history is not lost.
- **Safe copies:** `undoHistory()` and `recentActivity()` are `const` and return `Array<T>` copies, so callers cannot damage the internal stack or log.
- **Integrity check:** `stationInUse()` makes sure a station cannot be removed while tickets, cards or trips still refer to it.

### 4.4 `storage.cpp` (saving and loading)

- **Friend class:** `Storage` accesses the private data of `Metro` to save every counter and table.
- **Function templates:** `collectValues<Card>()` and `sortRecords(..., compareById<Card>)` produce a deterministic file.
- **Exception handling:** helper functions throw `runtime_error` for bad numbers or missing fields. `load()` catches them and returns `LoadStatus::Corrupt` with an error message.
- **Safe save:** data is written to a `.tmp` file first and then renamed, so a crash during saving does not destroy the old file.

---

## 5. Supporting classes in `metro.h`

**`Array<T>`** is our own `std::vector`-style container. Capacity starts at 8 and doubles when full. It has a destructor (`delete[]`), a deep-copy constructor and copy assignment (Rule of Three), `operator[]`, and `begin()/end()` for range-based for loops.

**`Metro`** is a Facade. It owns the graph, hash tables, heaps, lists, deque, trees and stack, and exposes one clean public interface. Its constructor allocates everything (and frees it and throws `bad_alloc` on failure). Its destructor calls `freeAll()`.

---

## 6. Limitations and possible improvements

| Limitation | Possible improvement |
|---|---|
| Hand-written data structures store data as `void*`, which needs casts | Make them class templates for type safety |
| `Metro` has many responsibilities | Split into card, ticket and admin services |
| `Clock` is a global static | Pass a clock object into `Metro` so tests are independent |
| `Metro` owns raw pointers but can still be copied | Mark copy constructor and assignment as `= delete` |
| `Array<T>` has no move semantics or bounds checking | Add move constructor and `at()` with checks |
| `hashPassword` is a simple salted hash | Use a standard algorithm (bcrypt/Argon2) in a real deployment |

---

## 7. Summary

| Feature | Main concepts |
|---|---|
| Passenger discounts | Inheritance, abstract class, polymorphism |
| Waitlist priority | Method overriding |
| Card balance safety | Encapsulation, const-correctness |
| Admin login lock-out | Encapsulation, static `Clock` |
| Undo | Command pattern |
| Passes and expiry | Polymorphism, static `Clock` |
| Route cost at tap-out | RAII |
| Save and load | Friend class, templates, exceptions, enum class |
| Trip and activity history | Class template, Rule of Three |
