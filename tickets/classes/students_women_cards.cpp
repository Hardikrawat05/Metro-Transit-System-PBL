#include "metro.h"

#include <cctype>
#include <cstdlib>
#include <new>

using namespace std;

Result Metro::registerStudent(int id)
{
    if (id <= 0)
        return fail("Student ID must be a positive number.");
    int* active = (int*)(tableGet(students_, id));
    if (active != nullptr && *active == 1)
        return fail("Student ID " + to_string(id) + " is already active.");

    bool isNew = active == nullptr;
    int one = 1;
    tablePut(students_, id, &one);
    pushUndo(ACT_REGISTER_STUDENT, id, isNew ? 1 : 0, 0, 0,
             "Register student " + to_string(id));
    log((isNew ? "Registered student " : "Reactivated student ") + to_string(id));
    return ok(string("Student ID ") + to_string(id) +
              (isNew ? " registered." : " reactivated."));
}

Result Metro::deactivateStudent(int id)
{
    int* active = (int*)(tableGet(students_, id));
    if (active == nullptr || *active == 0)
        return fail("No active student with ID " + to_string(id) + ".");
    *active = 0;
    pushUndo(ACT_DEACTIVATE_STUDENT, id, 0, 0, 0, "Deactivate student " + to_string(id));
    log("Deactivated student " + to_string(id));
    return ok("Student ID " + to_string(id) + " deactivated.");
}

bool Metro::isValidStudent(int id) const
{
    const int* active = (const int*)(tableGet(students_, id));
    return active != nullptr && *active == 1;
}

Array<StudentRecord> Metro::students() const
{
    Array<StudentRecord> out;
    HmIter it;
    tableIterStart(students_, &it);
    int key = 0;
    void* value = nullptr;
    while (tableIterNext(&it, &key, &value))
        out.push_back(StudentRecord{key, *(int*)value});
    sortRecords(out, compareById<StudentRecord>);
    return out;
}

Result Metro::registerWomenCard(int id)
{
    if (id <= 0)
        return fail("Women's card number must be a positive number.");
    int* active = (int*)(tableGet(womenCards_, id));
    if (active != nullptr && *active == 1)
        return fail("Women's card " + to_string(id) + " is already active.");

    bool isNew = active == nullptr;
    int one = 1;
    tablePut(womenCards_, id, &one);
    pushUndo(ACT_REGISTER_WOMEN_CARD, id, isNew ? 1 : 0, 0, 0,
             "Register women's card " + to_string(id));
    log((isNew ? "Registered women's card " : "Reactivated women's card ") + to_string(id));
    return ok("Women's card " + to_string(id) + (isNew ? " registered." : " reactivated."));
}

Result Metro::deactivateWomenCard(int id)
{
    int* active = (int*)(tableGet(womenCards_, id));
    if (active == nullptr || *active == 0)
        return fail("No active women's card with number " + to_string(id) + ".");
    *active = 0;
    pushUndo(ACT_DEACTIVATE_WOMEN_CARD, id, 0, 0, 0,
             "Deactivate women's card " + to_string(id));
    log("Deactivated women's card " + to_string(id));
    return ok("Women's card " + to_string(id) + " deactivated.");
}

bool Metro::isValidWomenCard(int id) const
{
    const int* active = (const int*)(tableGet(womenCards_, id));
    return active != nullptr && *active == 1;
}

Array<StudentRecord> Metro::womenCards() const
{
    Array<StudentRecord> out;
    HmIter it;
    tableIterStart(womenCards_, &it);
    int key = 0;
    void* value = nullptr;
    while (tableIterNext(&it, &key, &value))
        out.push_back(StudentRecord{key, *(int*)value});
    sortRecords(out, compareById<StudentRecord>);
    return out;
}

int Metro::waitPriority(int category, int womenCardId)
{
    int byCategory = farePolicy(category).waitlistPriority();
    int byCard = womenCardId != 0 ? PRIORITY_WOMEN : PRIORITY_REGULAR;
    return byCategory < byCard ? byCategory : byCard;
}

string Metro::priorityName(int priority)
{
    switch (priority) {
    case PRIORITY_SENIOR_DISABLED:
        return "1 - Senior / disabled";
    case PRIORITY_WOMEN:
        return "2 - Women";
    case PRIORITY_STUDENT:
        return "3 - Student";
    default:
        return "4 - Regular";
    }
}
