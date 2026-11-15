#include "metro.h"

using namespace std;

const FarePolicy& farePolicy(int category)
{
    static const RegularFare regular;
    static const StudentFare student;
    static const SeniorFare senior;
    static const DisabledFare disabled;

    switch (category) {
    case STUDENT:
        return student;
    case SENIOR:
        return senior;
    case DISABLED:
        return disabled;
    default:
        return regular;
    }
}

int fareForDistance(long long meters)
{
    if (meters <= 2000)
        return 10;
    if (meters <= 5000)
        return 20;
    if (meters <= 12000)
        return 30;
    if (meters <= 21000)
        return 40;
    if (meters <= 32000)
        return 50;
    return 60;
}

bool isValidCategory(int category)
{
    return category >= REGULAR && category <= DISABLED;
}
