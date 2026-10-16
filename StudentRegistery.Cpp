#include "common.h"
#include "StudentRegistry.h"

StudentRegistry::StudentRegistry() {
    count = 0;
}

bool StudentRegistry::registerStudent(int id) {
    if (id <= 0 || count >= 100)
        return false;

    for (int i = 0; i < count; i++) {
        if (studentIDs[i] == id) {
            if (active[i])
                return false;
            active[i] = true;
            return true;
        }
    }

    studentIDs[count] = id;
    active[count] = true;
    count++;
    return true;
}

bool StudentRegistry::isValidStudent(int id) {
    for (int i = 0; i < count; i++)
        if (studentIDs[i] == id && active[i])
            return true;

    return false;
}

bool StudentRegistry::deactivateStudent(int id) {
    for (int i = 0; i < count; i++) {
        if (studentIDs[i] == id && active[i]) {
            active[i] = false;
            return true;
        }
    }
    return false;
}

void StudentRegistry::displayStudents() {
    for (int i = 0; i < count; i++)
        cout << studentIDs[i] << " - "
             << (active[i] ? "Active" : "Inactive") << endl;
}