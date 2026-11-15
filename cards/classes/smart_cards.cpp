#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <new>

using namespace std;

Card* Metro::cardPtr(int id)
{
    return (Card*)(tableGet(cards_, id));
}

Result Metro::createCard(const string& name, int category, int studentId,
                         int initialBalance, int& newId)
{
    string clean = trim(name);
    if (clean.empty() || clean.size() >= NAME_LEN)
        return fail("Name must be 1 to " + to_string(NAME_LEN - 1) + " characters.");
    if (!isValidCategory(category))
        return fail("Invalid passenger category.");
    if (category == STUDENT && !isValidStudent(studentId))
        return fail("Student ID " + to_string(studentId) +
                    " is not registered or is inactive.");
    if (initialBalance < 0 || initialBalance > MAX_RECHARGE)
        return fail("Initial balance must be 0 to " + rs(MAX_RECHARGE) + ".");

    Card c{};
    c.id = nextCardId_++;
    copyText(c.name, sizeof c.name, clean);
    c.category = category;
    c.studentId = category == STUDENT ? studentId : 0;
    c.balance = initialBalance;
    c.status = C_ACTIVE;
    c.entryStation = -1;
    c.passType = PASS_NONE;
    c.createdAt = Clock::now();
    if (tablePut(cards_, c.id, &c) == nullptr)
        return fail("Out of memory.");

    newId = c.id;
    log("Issued card " + to_string(c.id) + " to " + clean);
    return ok("Card " + to_string(c.id) + " issued to " + clean + " with balance " +
              rs(initialBalance) + ".");
}

Result Metro::recharge(int cardId, int amount)
{
    Card* c = cardPtr(cardId);
    if (c == nullptr)
        return fail("Card not found.");
    if (c->status == C_BLOCKED)
        return fail("This card is blocked.");
    if (amount < 1 || amount > MAX_RECHARGE)
        return fail("Recharge amount must be 1 to " + rs(MAX_RECHARGE) + ".");
    if (c->balance + amount > MAX_BALANCE)
        return fail("Balance cannot exceed " + rs(MAX_BALANCE) + ".");
    c->balance += amount;
    log("Card " + to_string(cardId) + " recharged by " + rs(amount));
    return ok("Recharged " + rs(amount) + ". New balance: " + rs(c->balance) + ".");
}

bool Metro::hasValidPass(const Card& card) const
{
    return card.passType != PASS_NONE && card.passExpiry > Clock::now();
}

Result Metro::tapIn(int cardId, int station)
{
    Card* c = cardPtr(cardId);
    if (c == nullptr)
        return fail("Card not found.");
    if (c->status == C_BLOCKED)
        return fail("This card is blocked. Entry denied.");
    if (c->tappedIn)
        return fail("Card is already tapped in at " + stationName(c->entryStation) + ".");
    if (!graphHasStation(graph_, station))
        return fail("Invalid station.");
    if (!graphIsOpen(graph_, station))
        return fail(stationName(station) + " is closed. Entry denied.");
    if (!hasValidPass(*c) && c->balance < MIN_TAPIN_BALANCE)
        return fail("Minimum balance of " + rs(MIN_TAPIN_BALANCE) +
                    " needed. Please recharge.");

    c->tappedIn = 1;
    c->entryStation = station;
    c->entryTime = Clock::now();
    log("Card " + to_string(cardId) + " tapped in at " + stationName(station));
    return ok("Gate open. Welcome to " + stationName(station) + ".");
}

TapOutResult Metro::tapOut(int cardId, int station)
{
    TapOutResult r;
    Card* c = cardPtr(cardId);
    if (c == nullptr) {
        r.message = "Card not found.";
        return r;
    }
    if (c->status == C_BLOCKED) {
        r.message = "This card is blocked.";
        return r;
    }
    if (!c->tappedIn) {
        r.message = "This card has not tapped in.";
        return r;
    }
    if (!graphHasStation(graph_, station)) {
        r.message = "Invalid station.";
        return r;
    }
    if (!graphIsOpen(graph_, station)) {
        r.message = stationName(station) + " is closed. Please exit at another station.";
        return r;
    }

    int baseFare = MIN_FARE;
    if (station != c->entryStation) {
        RouteHolder rh;
        if (dijkstraRoute(graph_, c->entryStation, station, true, &rh.route)) {
            r.distanceM = rh.route.totalMeters;
            baseFare = fareForDistance(r.distanceM);
        }
    }

    const FarePolicy& policy = farePolicy(c->category);
    r.fare = policy.apply(baseFare);
    if (hasValidPass(*c)) {
        r.fare = 0;
        r.passUsed = true;
    }
    long long minutesInside = (Clock::now() - c->entryTime) / 60;
    if (minutesInside > MAX_TRIP_MIN)
        r.penalty = OVERSTAY_PENALTY;

    int total = r.fare + r.penalty;
    if (c->balance < total) {
        r.message = "Insufficient balance: need " + rs(total) + ", have " + rs(c->balance) +
                    ". Please recharge to exit.";
        return r;
    }

    c->balance -= total;
    c->tappedIn = 0;
    cardRevenue_ += total;

    Trip t{};
    t.cardId = cardId;
    t.source = c->entryStation;
    t.destination = station;
    t.fare = total;
    t.time = Clock::now();
    listAddFront(trips_, &t);
    c->entryStation = -1;

    r.ok = true;
    r.balanceLeft = c->balance;
    r.message = "Gate open. Charged " + rs(total) + ".";
    log("Card " + to_string(cardId) + " tapped out at " + stationName(station) +
        ", charged " + rs(total));
    return r;
}

int Metro::passPrice(int category, int passType) const
{
    int base = passType == PASS_DAY ? DAY_PASS_PRICE : MONTHLY_PASS_PRICE;
    return farePolicy(category).apply(base);
}

Result Metro::buyPass(int cardId, int passType)
{
    Card* c = cardPtr(cardId);
    if (c == nullptr)
        return fail("Card not found.");
    if (c->status == C_BLOCKED)
        return fail("This card is blocked.");
    if (passType != PASS_DAY && passType != PASS_MONTHLY)
        return fail("Invalid pass type.");
    if (hasValidPass(*c))
        return fail("Card already has a pass valid until " + Clock::format(c->passExpiry) + ".");

    int price = passPrice(c->category, passType);
    if (c->balance < price)
        return fail("Pass costs " + rs(price) + " but balance is " + rs(c->balance) + ".");

    long long duration = passType == PASS_DAY ? 24LL * 3600 : 30LL * 24 * 3600;
    c->balance -= price;
    c->passType = passType;
    c->passExpiry = Clock::now() + duration;
    passRevenue_ += price;
    string kind = passType == PASS_DAY ? "Day pass" : "Monthly pass";
    log(kind + " bought on card " + to_string(cardId));
    return ok(kind + " activated for " + rs(price) + ". Valid until " +
              Clock::format(c->passExpiry) + ".");
}

Result Metro::blockAndTransfer(int cardId, int& newCardId)
{
    Card* old = cardPtr(cardId);
    if (old == nullptr)
        return fail("Card not found.");
    if (old->status == C_BLOCKED)
        return fail("Card is already blocked.");

    Card fresh = *old;
    fresh.id = nextCardId_++;
    fresh.tappedIn = 0;
    fresh.entryStation = -1;
    fresh.createdAt = Clock::now();
    if (tablePut(cards_, fresh.id, &fresh) == nullptr)
        return fail("Out of memory.");

    old = cardPtr(cardId);
    int moved = old->balance;
    old->status = C_BLOCKED;
    old->balance = 0;
    old->passType = PASS_NONE;
    old->passExpiry = 0;
    old->tappedIn = 0;

    newCardId = fresh.id;
    log("Card " + to_string(cardId) + " blocked, moved to card " +
        to_string(fresh.id));
    return ok("Card " + to_string(cardId) + " blocked. New card " +
              to_string(fresh.id) + " issued with balance " + rs(moved) + ".");
}

const Card* Metro::findCard(int id) const
{
    return (const Card*)(tableGet(cards_, id));
}

Array<Trip> Metro::cardHistory(int cardId, int maxTrips) const
{
    Array<Trip> out;
    for (ListNode* n = trips_->head; n != nullptr && (int)out.size() < maxTrips; n = n->next) {
        const Trip* t = (const Trip*)n->data;
        if (t->cardId == cardId)
            out.push_back(*t);
    }
    return out;
}
