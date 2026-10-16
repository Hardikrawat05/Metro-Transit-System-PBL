#ifndef STUDENTREGISTRY_H
#define STUDENTREGISTRY_H

#include "common.h"

class StudentRegistry {
private:
    int studentIDs[MAX_STUDENTS];
    bool active[MAX_STUDENTS];
    int count;

public:
    StudentRegistry();
    bool registerStudent(int id);
    bool isValidStudent(int id);
    bool deactivateStudent(int id);
    void displayStudents();
};

